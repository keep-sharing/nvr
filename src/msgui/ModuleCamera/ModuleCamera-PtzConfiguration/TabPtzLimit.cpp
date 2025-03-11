#include "TabPtzLimit.h"
#include "ui_TabPtzLimit.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"

TabPtzLimit::TabPtzLimit(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::TabPtzLimit)
{
    ui->setupUi(this);
    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelGroupClicked(int)));

    ui->comboBoxLimitMode->clear();
    ui->comboBoxLimitMode->addItem(GET_TEXT("PTZCONFIG/166029", "Manual Limit"), 0);
    ui->comboBoxLimitMode->addItem(GET_TEXT("PTZCONFIG/166030", "Scan Lmit"), 1);

    ui->lineEditModeStatus->setEnabled(false);

    connect(ui->widgetLimitPanel, SIGNAL(setLimitControl(PTZ_LIMITS_TYPE, int)), this, SLOT(setLimitControl(PTZ_LIMITS_TYPE, int)));
    onLanguageChanged();
}

TabPtzLimit::~TabPtzLimit()
{
    delete ui;
}

void TabPtzLimit::initializeData()
{
    ui->channelGroup->setCurrentIndex(currentChannel());
}

void TabPtzLimit::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_PTZ_LIMIT:
        ON_RESPONSE_FLAG_GET_IPC_PTZ_LIMIT(message);
        break;
    default:
        break;
    }
}

void TabPtzLimit::setSettingEnable(bool enable)
{
    if (!enable) {
        ui->ptzControlPanel->clearPreset();
    }
    ui->ptzControlPanel->setEnabled(enable);

    ui->comboBoxLimitMode->setCurrentIndex(0);
    ui->comboBoxLimitMode->setEnabled(enable);

    ui->checkBoxEnable->setChecked(false);
    ui->checkBoxEnable->setEnabled(enable);
    ui->lineEditModeStatus->clear();
    ui->pushButtonApply->setEnabled(enable);
    ui->widgetLimitPanel->setEnabled(enable);
    ui->widgetLimitPanel->initializeData(0, 0, 1);
    ui->widgetLimitPanel->setMinimumHeight(186);
    ui->widgetLimitPanel->setMaximumHeight(186);
}

void TabPtzLimit::changeMask(int &order, int bit, int value)
{
    if (value) {
        order |= (1 << bit);
    } else {
        order &= ~(1 << bit);
    }
}

void TabPtzLimit::updateMyInfo(PTZ_LIMITS_TYPE type, int value)
{
    switch (type) {
    case PTZ_LIMITS_MANUAL_CLEAR_ALL:
        m_limitInfo.manualLimited = 0;
        break;
    case PTZ_LIMITS_MANUAL_SET_LEFT:
        changeMask(m_limitInfo.manualLimited, 0, value);
        break;
    case PTZ_LIMITS_MANUAL_SET_RIGHT:
        changeMask(m_limitInfo.manualLimited, 1, value);
        break;
    case PTZ_LIMITS_MANUAL_SET_UP:
        changeMask(m_limitInfo.manualLimited, 2, value);
        break;
    case PTZ_LIMITS_MANUAL_SET_DOWN:
        changeMask(m_limitInfo.manualLimited, 3, value);
        break;
    case PTZ_LIMITS_SCAN_CLEAR_ALL:
        m_limitInfo.scanLimited = 0;
        break;
    case PTZ_LIMITS_SCAN_SET_LEFT:
        changeMask(m_limitInfo.scanLimited, 0, value);
        break;
    case PTZ_LIMITS_SCAN_SET_RIGHT:
        changeMask(m_limitInfo.scanLimited, 1, value);
        break;
    case PTZ_LIMITS_SCAN_SET_UP:
        changeMask(m_limitInfo.scanLimited, 2, value);
        break;
    case PTZ_LIMITS_SCAN_SET_DOWN:
        changeMask(m_limitInfo.scanLimited, 3, value);
        break;
    default:
        break;
    }
}

void TabPtzLimit::setLimitControl(PTZ_LIMITS_TYPE type, int value)
{
    updateMyInfo(type, value);
    IpcPtzLimitControl limitControl;
    memset(&limitControl, 0, sizeof(IpcPtzLimitControl));
    limitControl.chnId = m_channel;
    limitControl.limitType = type;
    limitControl.setValue = value;
    sendMessageOnly(REQUEST_FLAG_SET_IPC_PTZ_LIMIT, &limitControl, sizeof(IpcPtzLimitControl));
}

void TabPtzLimit::ON_RESPONSE_FLAG_GET_IPC_PTZ_LIMIT(MessageReceive *message)
{
    m_eventLoop.exit();

    IpcPtzLimitINFO *limitInfo = static_cast<IpcPtzLimitINFO *>(message->data);
    if (!limitInfo) {
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }
    memcpy(&m_limitInfo, limitInfo, sizeof(IpcPtzLimitINFO));
    if (!m_limitInfo.ptzSupport) {
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }
    //showWait();

    ui->widgetMessage->hide();
    setSettingEnable(true);
    gPtzDataManager->beginGetData(m_channel);
    gPtzDataManager->waitForGetCameraModelInfo();
    gPtzDataManager->waitForGetPtzOvfInfo();
    resp_ptz_ovf_info *ptz_ovf_info = gPtzDataManager->ptzOvfInfo();
    ui->ptzControlPanel->showPresetData(ptz_ovf_info->preset, gPtzDataManager->maxPresetCount(m_channel));

    gPtzDataManager->waitForGetPtzSpeed();
    ui->ptzControlPanel->editSpeedValue(gPtzDataManager->ptzSpeed());

    gPtzDataManager->waitForGetAutoScan();
    ui->ptzControlPanel->setAutoScanChecked(gPtzDataManager->autoScan());
    //closeWait();
    ui->comboBoxLimitMode->setCurrentIndexFromData(0);

    if (!m_limitInfo.speedOrMiniPtzDemoSupport) {
        ui->widgetLimitPanel->setMinimumHeight(126);
        ui->widgetLimitPanel->setMaximumHeight(126);
    }
}

void TabPtzLimit::onLanguageChanged()
{
    ui->labelLimitMode->setText(GET_TEXT("PTZCONFIG/166031", "Limit Mode"));
    ui->labelEnable->setText(GET_TEXT("PTZCONFIG/166001", "PTZ Lmits"));
    ui->labelModeStatus->setText(GET_TEXT("PTZCONFIG/166032", "Mode Status"));
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->ptzControlPanel->onLanguageChanged();
}

void TabPtzLimit::onChannelGroupClicked(int channel)
{
    m_channel = channel;
    ui->commonVideo->playVideo(m_channel);
    setCurrentChannel(channel);
    if (!LiveView::instance()->isChannelConnected(channel)) {
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPC_PTZ_LIMIT, &channel, sizeof(int));
    //m_eventLoop.exec();
    //closeWait();
}

void TabPtzLimit::on_pushButtonApply_clicked()
{
    PTZ_LIMITS_TYPE type = ui->comboBoxLimitMode->currentIntData() ? PTZ_LIMITS_SCAN_LIMIT_ENABLE : PTZ_LIMITS_MANUAL_LIMIT_ENABLE;
    setLimitControl(type, ui->checkBoxEnable->isChecked());
}

void TabPtzLimit::on_pushButtonBack_clicked()
{
    back();
}

void TabPtzLimit::on_comboBoxLimitMode_indexSet(int index)
{
    Q_UNUSED(index)
    switch (ui->comboBoxLimitMode->currentIntData()) {
    case 0:
        ui->checkBoxEnable->setChecked(m_limitInfo.manualLimitEnable);
        ui->lineEditModeStatus->setText(m_limitInfo.manualLimitStatus ? GET_TEXT("PTZCONFIG/166035", "Limited") : GET_TEXT("PTZCONFIG/166036", "Not Limited"));
        ui->widgetLimitPanel->initializeData(m_limitInfo.manualLimited, ui->comboBoxLimitMode->currentIntData(), m_limitInfo.speedOrMiniPtzDemoSupport);
        break;
    case 1:
        ui->checkBoxEnable->setChecked(m_limitInfo.scanLimitEnable);
        ui->lineEditModeStatus->setText(m_limitInfo.scanLimitStatus ? GET_TEXT("PTZCONFIG/166035", "Limited") : GET_TEXT("PTZCONFIG/166036", "Not Limited"));
        ui->widgetLimitPanel->initializeData(m_limitInfo.scanLimited, ui->comboBoxLimitMode->currentIntData(), m_limitInfo.speedOrMiniPtzDemoSupport);
        break;
    default:
        break;
    }
    ui->widgetLimitPanel->setEnabled(ui->checkBoxEnable->isChecked());
}

void TabPtzLimit::on_checkBoxEnable_clicked()
{
    if (ui->comboBoxLimitMode->currentIntData() == 0) {
        m_limitInfo.manualLimitEnable = ui->checkBoxEnable->isChecked();
    } else {
        m_limitInfo.scanLimitEnable = ui->checkBoxEnable->isChecked();
    }
    ui->widgetLimitPanel->setEnabled(ui->checkBoxEnable->isChecked());
}
