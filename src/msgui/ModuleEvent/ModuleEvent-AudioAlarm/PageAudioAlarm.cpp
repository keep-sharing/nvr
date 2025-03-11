#include "PageAudioAlarm.h"
#include "ui_PageAudioAlarm.h"
#include "ActionAudioAlarm.h"
#include "EffectiveTimeAudioAlarm.h"
#include "EventLoop.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"

PageAudioAlarm::PageAudioAlarm(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PageAudioAlarm)
{
    ui->setupUi(this);
    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupClicked(int)));

    ui->sliderAlarmThreshold->setRange(0, 100);

    ui->lineEditAudioSampleValue->setEnabled(false);
    m_effectiveTime = new EffectiveTimeAudioAlarm(this);
    m_action = new ActionAudioAlarm(this);
    onLanguageChanged();
}

PageAudioAlarm::~PageAudioAlarm()
{
    delete ui;
}

void PageAudioAlarm::initializeData()
{
    ui->channelGroup->setCurrentIndex(0);
}

void PageAudioAlarm::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_AUDIO_ALARM:
        ON_RESPONSE_FLAG_GET_IPC_AUDIO_ALARM(message);
        break;
    case RESPONSE_FLAG_SET_IPC_AUDIO_ALARM:
        ON_RESPONSE_FLAG_SET_IPC_AUDIO_ALARM(message);
        break;
    case RESPONSE_FLAG_GET_IPC_AUDIO_ALARM_SAMPLE:
        ON_RESPONSE_FLAG_GET_IPC_AUDIO_ALARM_SAMPLE(message);
        break;
    default:
        break;
    }
}

void PageAudioAlarm::ON_RESPONSE_FLAG_GET_IPC_AUDIO_ALARM(MessageReceive *message)
{
    IPC_AUDIO_ALARM_S *ipcAudioAlarm = static_cast<IpcAudioAlarm *>(message->data);
    if (!ipcAudioAlarm) {
        qWarning() << "PageAudioAlarm::ON_RESPONSE_FLAG_GET_IPC_AUDIO_ALARM, data is null.";
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        m_eventLoop.quit();
        return;
    }
    m_haveJsonData = true;
    setSettingEnable(true);
    ui->checkBoxEnable->setChecked(ipcAudioAlarm->alarmEnable);
    ui->sliderAlarmThreshold->setValue(ipcAudioAlarm->alarmThreshold);
    on_checkBoxEnable_clicked(ui->checkBoxEnable->isChecked());
    m_eventLoop.quit();
}

void PageAudioAlarm::ON_RESPONSE_FLAG_SET_IPC_AUDIO_ALARM(MessageReceive *message)
{
    Q_UNUSED(message)
    //closeWait();
    m_eventLoop.quit();
}

void PageAudioAlarm::ON_RESPONSE_FLAG_GET_IPC_AUDIO_ALARM_SAMPLE(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "PageAudioAlarm::ON_RESPONSE_FLAG_GET_IPC_AUDIO_ALARM_SAMPLE, data is null.";
        m_eventLoop.quit();
        return;
    }
    int result = (*((int *)message->data));
    if (result < 0) {
        ui->lineEditAudioSampleValue->setText("-");
    } else {
        ui->lineEditAudioSampleValue->setText(QString("%1").arg(result));
    }

    m_eventLoop.quit();
}

void PageAudioAlarm::saveSetting(quint64 copyFlags)
{
    return;
    //showWait();
    if (m_effectiveTime) {
        m_effectiveTime->saveEffectiveTime(m_currentChannel, copyFlags);
    }
    if (m_action) {
        m_action->saveAction(m_currentChannel, copyFlags);
    }
    struct IpcAudioAlarm ipcAudioAlarm;
    ipcAudioAlarm.chnId = m_currentChannel;
    ipcAudioAlarm.alarmEnable = ui->checkBoxEnable->isChecked();
    ipcAudioAlarm.alarmThreshold = static_cast<int>(ui->sliderAlarmThreshold->value());

    memset(ipcAudioAlarm.copyChn, 0, sizeof(ipcAudioAlarm.copyChn));
    for (int i = 0; i < MAX_CAMERA; ++i) {
        if (copyFlags >> i & 0x01) {
            ipcAudioAlarm.copyChn[i] = '1';
        } else {
            ipcAudioAlarm.copyChn[i] = '0';
        }
    }
    sendMessage(REQUEST_FLAG_SET_IPC_AUDIO_ALARM, &ipcAudioAlarm, sizeof(IpcAudioAlarm));
    //m_eventLoop.exec();
    sendMessageOnly(REQUEST_FLAG_UPDATE_IPC_AUDIO_ALARM_ACTION, &copyFlags, sizeof(copyFlags));
}

void PageAudioAlarm::onLanguageChanged()
{
    ui->labelAudioAlarm->setText(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"));
    ui->labelAlarmThreshold->setText(GET_TEXT("AUDIO_ALARM/159001", "Alarm Threshold"));
    ui->labelAudioSampleValue->setText(GET_TEXT("AUDIO_ALARM/159002", "Audio Sample Value"));
    ui->labelEffectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->labelAction->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));

    ui->pushButtonEffectiveTime->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonAction->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PageAudioAlarm::setSettingEnable(bool enable)
{
    ui->checkBoxEnable->setEnabled(enable);
    ui->pushButtonCopy->setEnabled(enable);
    ui->pushButtonApply->setEnabled(enable);
    on_checkBoxEnable_clicked(enable);
}

void PageAudioAlarm::clearSetting()
{
    ui->checkBoxEnable->setChecked(false);
    ui->lineEditAudioSampleValue->setText("0");
    ui->sliderAlarmThreshold->setValue(25);
    m_haveJsonData = false;
}

void PageAudioAlarm::onButtonGroupClicked(int index)
{
    if (!isVisible()) {
        return;
    }
    if (m_action) {
        m_action->clearCache();
    }
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }
    clearSetting();
    setSettingEnable(false);
    ui->widgetMessage->hideMessage();
    m_currentChannel = index;
    ui->video->playVideo(index);

    if (!LiveView::instance()->isChannelConnected(m_currentChannel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }
    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(m_currentChannel, &system_info);
    if (!system_info.systemAudioSupportType || system_info.systemAudioSupportType == IPC_AUDIO_SUPPORT_UNKNOW) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPC_AUDIO_ALARM, &m_currentChannel, sizeof(int));
    //m_eventLoop.exec();
    if (m_haveJsonData) {
        sendMessage(REQUEST_FLAG_GET_IPC_AUDIO_ALARM_SAMPLE, &m_currentChannel, sizeof(int));
        //m_eventLoop.exec();
    }
    //closeWait();
}

void PageAudioAlarm::on_checkBoxEnable_clicked(bool checked)
{
    ui->sliderAlarmThreshold->setEnabled(checked);
    ui->toolButtonReset->setEnabled(checked);
    ui->pushButtonEffectiveTime->setEnabled(checked);
    ui->pushButtonAction->setEnabled(checked);
}

void PageAudioAlarm::on_pushButtonEffectiveTime_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimeAudioAlarm(this);
    }
    m_effectiveTime->showEffectiveTime(m_currentChannel);
}

void PageAudioAlarm::on_pushButtonAction_clicked()
{
    if (!m_action) {
        m_action = new ActionAudioAlarm(this);
    }
    m_action->showAction(m_currentChannel);
}

void PageAudioAlarm::on_pushButtonCopy_clicked()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        quint64 copyFlags = copy.checkedFlags(true);
        saveSetting(copyFlags);
    }
}

void PageAudioAlarm::on_pushButtonApply_clicked()
{
    quint64 copyFlags = 0;
    ms_set_bit(&copyFlags, m_currentChannel, 1);
    saveSetting(copyFlags);
}

void PageAudioAlarm::on_pushButtonBack_clicked()
{
    emit sig_back();
}

void PageAudioAlarm::on_toolButtonReset_clicked()
{
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPC_AUDIO_ALARM_SAMPLE, &m_currentChannel, sizeof(int));
    //m_eventLoop.exec();
    //closeWait();
}

void PageAudioAlarm::on_toolButtonReset_pressed()
{
    ui->toolButtonReset->setIcon(QIcon(":/common/common/reset_hover.png"));
}

void PageAudioAlarm::on_toolButtonReset_released()
{
    ui->toolButtonReset->setIcon(QIcon(":/common/common/reset.png"));
}
