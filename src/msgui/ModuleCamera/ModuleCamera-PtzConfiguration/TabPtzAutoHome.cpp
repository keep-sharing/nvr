#include "TabPtzAutoHome.h"
#include "ui_TabPtzAutoHome.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"

TabPtzAutoHome::TabPtzAutoHome(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::TabPtzAutoHome)
{
    ui->setupUi(this);
    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelGroupClicked(int)));

    ui->comboBoxAutoHomeMode->clear();
    ui->comboBoxAutoHomeMode->addItem(GET_TEXT("PTZDIALOG/21015", "Preset"), 0);

    QRegExp regx("[0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, ui->lineEditLatencyTime);
    ui->lineEditLatencyTime->setValidator(validator);
    ui->lineEditLatencyTime->setRange(5, 720);

    ui->comboBoxAutoHomeMode->setEnabled(false);

    onLanguageChanged();
}

TabPtzAutoHome::~TabPtzAutoHome()
{
    delete ui;
}

void TabPtzAutoHome::initializeData()
{
    ui->channelGroup->setCurrentIndex(currentChannel());
}

void TabPtzAutoHome::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_PTZ_AUTO_HOME:
        ON_RESPONSE_FLAG_GET_IPC_PTZ_AUTO_HOME(message);
        break;
    case RESPONSE_FLAG_SET_IPC_PTZ_AUTO_HOME:
        ON_RESPONSE_FLAG_SET_IPC_PTZ_AUTO_HOME(message);
        break;
    default:
        break;
    }
}

void TabPtzAutoHome::setSettingEnable(bool enable)
{
    ui->pushButtonApply->setEnabled(enable);
    ui->pushButtonCall->setEnabled(enable);

    ui->checkBoxEnable->setChecked(false);
    ui->checkBoxEnable->setEnabled(enable);

    ui->comboBoxAutoHomeModeNumber->setCurrentIndex(0);
    ui->comboBoxAutoHomeModeNumber->setEnabled(enable);

    ui->lineEditLatencyTime->setText("5");
    ui->lineEditLatencyTime->setEnabled(enable);
}

void TabPtzAutoHome::ON_RESPONSE_FLAG_GET_IPC_PTZ_AUTO_HOME(MessageReceive *message)
{
    m_eventLoop.exit();

    IpcPtzAutoHome *autoHome = static_cast<IpcPtzAutoHome *>(message->data);
    if (!autoHome) {
      setSettingEnable(false);
      ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }
    memcpy(&m_ptzAutoHome, autoHome, sizeof(IpcPtzAutoHome));
    if (!m_ptzAutoHome.ptzSupport) {
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }

    ui->widgetMessage->hide();
    setSettingEnable(true);

    ui->checkBoxEnable->setChecked(m_ptzAutoHome.autoHomeEnable);
    ui->lineEditLatencyTime->setText(QString("%1").arg(m_ptzAutoHome.latencyTime));
    ui->comboBoxAutoHomeMode->setCurrentIndexFromData(m_ptzAutoHome.autoHomeMode);

    ui->comboBoxAutoHomeModeNumber->clear();
    ui->comboBoxAutoHomeModeNumber->addItem(GET_TEXT("PTZCONFIG/166015","Current Location"), 0);
    int limit = m_ptzAutoHome.wiperSupport ? 52 : 51;
    for (int i = 0; i < PRESET_MAX; i++) {
        if (!m_ptzAutoHome.preset[i].enable || (i >= 32 && i <= limit)) {
            continue;
        }
        ui->comboBoxAutoHomeModeNumber->addItem(m_ptzAutoHome.preset[i].name, i + 1);
    }
    ui->comboBoxAutoHomeModeNumber->setCurrentIndexFromData(m_ptzAutoHome.autoHomeModeNumber);
    on_checkBoxEnable_clicked();
}

void TabPtzAutoHome::ON_RESPONSE_FLAG_SET_IPC_PTZ_AUTO_HOME(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit();
}

void TabPtzAutoHome::onLanguageChanged()
{
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->labelAutoHomeEnable->setText(GET_TEXT("PTZCONFIG/166000", "Auto Home"));
    ui->labelLatencyTime->setText(GET_TEXT("PTZCONFIG/166016", "Latency Time"));
    ui->labelAutoHomeModeNumber->setText(GET_TEXT("PTZCONFIG/166017", "Auto Home Mode Number"));
    ui->labelAutoHomeMode->setText(GET_TEXT("PTZCONFIG/166018", "Auto Home Mode"));
    ui->pushButtonCall->setText(GET_TEXT("SMARTEVENT/146008", "call"));

    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabPtzAutoHome::onChannelGroupClicked(int channel)
{
    m_channel = channel;
    setCurrentChannel(channel);
    ui->commonVideo->playVideo(m_channel);
    if (!LiveView::instance()->isChannelConnected(channel)) {
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPC_PTZ_AUTO_HOME, &channel, sizeof(int));
    //m_eventLoop.exec();
    //closeWait();
}

void TabPtzAutoHome::on_pushButtonApply_clicked()
{
    if (!ui->lineEditLatencyTime->checkValid()) {
        return;
    }
    m_ptzAutoHome.latencyTime = ui->lineEditLatencyTime->text().toInt();
    m_ptzAutoHome.autoHomeMode = ui->comboBoxAutoHomeMode->currentIntData();
    m_ptzAutoHome.autoHomeModeNumber = ui->comboBoxAutoHomeModeNumber->currentIntData();

    //showWait();
    sendMessage(REQUEST_FLAG_SET_IPC_PTZ_AUTO_HOME, &m_ptzAutoHome, sizeof(IpcPtzAutoHome));
    //m_eventLoop.exec();
    //closeWait();
}

void TabPtzAutoHome::on_pushButtonBack_clicked()
{
    back();
}

void TabPtzAutoHome::on_pushButtonCall_clicked()
{
    IpcPtzControl ptzControl;
    memset(&ptzControl, 0, sizeof(IpcPtzControl));
    ptzControl.chnId = m_channel;
    ptzControl.controlType = IPC_PTZ_CONTORL_CALL_PRESET;
    ptzControl.controlAction = ui->comboBoxAutoHomeModeNumber->currentIntData();
    sendMessageOnly(REQUEST_FLAG_SET_IPC_PTZ_CONTROL_JSON, &ptzControl, sizeof(IpcPtzControl));
}

void TabPtzAutoHome::on_checkBoxEnable_clicked()
{
    m_ptzAutoHome.autoHomeEnable = ui->checkBoxEnable->isChecked();
    ui->pushButtonCall->setEnabled(m_ptzAutoHome.autoHomeEnable);
    ui->comboBoxAutoHomeModeNumber->setEnabled(m_ptzAutoHome.autoHomeEnable);
    ui->lineEditLatencyTime->setEnabled(m_ptzAutoHome.autoHomeEnable);
}
