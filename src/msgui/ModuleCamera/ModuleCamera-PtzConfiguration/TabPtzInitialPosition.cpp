#include "TabPtzInitialPosition.h"
#include "ui_TabPtzInitialPosition.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"

TabPtzInitialPosition::TabPtzInitialPosition(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::TabPtzInitialPosition)
{
    ui->setupUi(this);
    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelGroupClicked(int)));
    onLanguageChanged();
}

TabPtzInitialPosition::~TabPtzInitialPosition()
{
    delete ui;
}

void TabPtzInitialPosition::initializeData()
{
    ui->channelGroup->setCurrentIndex(currentChannel());
}

void TabPtzInitialPosition::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabPtzInitialPosition::setSettingEnable(bool enable)
{
    if (!enable) {
        ui->ptzControlPanel->clearPreset();
    }
    ui->ptzControlPanel->setEnabled(enable);
    ui->pushButtonSet->setEnabled(enable);
    ui->pushButtonCall->setEnabled(enable);
    ui->pushButtonClear->setEnabled(enable);
}

void TabPtzInitialPosition::setInitialPosition(int action)
{
    IpcPtzInitialPosition ptzInitialPosition;
    memset(&ptzInitialPosition, 0, sizeof(IpcPtzInitialPosition));
    ptzInitialPosition.chnId = m_channel;
    ptzInitialPosition.positionCmd = action;
    sendMessage(REQUEST_FLAG_SET_IPC_PTZ_INITIAL_POSITION, &ptzInitialPosition, sizeof(IpcPtzInitialPosition));
}

void TabPtzInitialPosition::onLanguageChanged()
{
    ui->labelInitialPosition->setText(GET_TEXT("PTZCONFIG/166002", "Initial Position"));
    ui->pushButtonSet->setText(GET_TEXT("PTZDIALOG/21021", "Set"));
    ui->pushButtonClear->setText(GET_TEXT("PTZCONFIG/36039", "Clear"));
    ui->pushButtonCall->setText(GET_TEXT("SMARTEVENT/146008", "Call"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->ptzControlPanel->onLanguageChanged();
}

void TabPtzInitialPosition::onChannelGroupClicked(int channel)
{
    setCurrentChannel(channel);
    m_channel = channel;
    ui->commonVideo->playVideo(m_channel);
    ui->ptzControlPanel->setCurrentChannel(channel);
    if (!LiveView::instance()->isChannelConnected(channel)) {
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }
    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(channel, &system_info);
    //showWait();
    if (!system_info.ptzSupport) {
        //closeWait();
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }
    ui->widgetMessage->hide();
    setSettingEnable(true);
    gPtzDataManager->beginGetData(channel);
    gPtzDataManager->waitForGetCameraModelInfo();
    gPtzDataManager->waitForGetPtzOvfInfo();
    resp_ptz_ovf_info *ptz_ovf_info = gPtzDataManager->ptzOvfInfo();
    ui->ptzControlPanel->showPresetData(ptz_ovf_info->preset, gPtzDataManager->maxPresetCount(channel));

    gPtzDataManager->waitForGetPtzSpeed();
    ui->ptzControlPanel->editSpeedValue(gPtzDataManager->ptzSpeed());

    gPtzDataManager->waitForGetAutoScan();
    ui->ptzControlPanel->setAutoScanChecked(gPtzDataManager->autoScan());
    //closeWait();
}

void TabPtzInitialPosition::on_pushButtonBack_clicked()
{
    back();
}

void TabPtzInitialPosition::on_pushButtonSet_clicked()
{
    setInitialPosition(0);
}

void TabPtzInitialPosition::on_pushButtonClear_clicked()
{
    setInitialPosition(1);
}

void TabPtzInitialPosition::on_pushButtonCall_clicked()
{
    setInitialPosition(2);
}
