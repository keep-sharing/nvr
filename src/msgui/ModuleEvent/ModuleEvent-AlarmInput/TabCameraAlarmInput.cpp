#include "TabCameraAlarmInput.h"
#include "ui_TabCameraAlarmInput.h"
#include "ActionCameraAlarmInput.h"
#include "EffectiveTimeCameraAlarmInput.h"
#include "EventLoop.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsMessage.h"
#include "MsWaitting.h"
#include "MyButtonGroup.h"
#include "MyDebug.h"
#include "channelcopydialog.h"

TabCameraAlarmInput::TabCameraAlarmInput(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabCameraAlarmInput)
{
    ui->setupUi(this);

    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelGroupClicked(int)));

    m_alarmButtonGroup = new MyButtonGroup(this);
    m_alarmButtonGroup->addButton(ui->pushButton_alarm1, ActionCameraAlarmInput1);
    m_alarmButtonGroup->addButton(ui->pushButton_alarm2, ActionCameraAlarmInput2);
    m_alarmButtonGroup->addButton(ui->pushButton_alarm3, ActionCameraAlarmInput3);
    m_alarmButtonGroup->addButton(ui->pushButton_alarm4, ActionCameraAlarmInput4);
    connect(m_alarmButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onAlarmInputGroupClicked(int)));

    ui->comboBox_type->clear();
    ui->comboBox_type->addItem(GET_TEXT("ALARMIN/52005", "NO"), 0);
    ui->comboBox_type->addItem(GET_TEXT("ALARMIN/52006", "NC"), 1);

    m_effective = new EffectiveTimeCameraAlarmInput(this);
    m_action = new ActionCameraAlarmInput(this);

    setSettingEnable(false);
    onLanguageChanged();
}

TabCameraAlarmInput::~TabCameraAlarmInput()
{
    delete ui;
}

void TabCameraAlarmInput::initializeData()
{
    ui->channelGroup->setCurrentIndex(0);
}

void TabCameraAlarmInput::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_ALARM_INPUT:
        ON_RESPONSE_FLAG_GET_IPC_ALARM_INPUT(message);
        break;
    case RESPONSE_FLAG_SET_IPC_ALARM_INPUT_BATCH:
        ON_RESPONSE_FLAG_SET_IPC_ALARM_INPUT_BATCH(message);
        break;
    }
}

void TabCameraAlarmInput::ON_RESPONSE_FLAG_GET_IPC_ALARM_INPUT(MessageReceive *message)
{
    ms_ipc_alarm_in *ipc_alarm_in = (ms_ipc_alarm_in *)message->data;
    if (!ipc_alarm_in) {
        qMsDebug() << message;
        gEventLoopExit(-1);
        return;
    }

    memset(&m_ipc_alarm_in, 0, sizeof(ms_ipc_alarm_in));
    memcpy(&m_ipc_alarm_in, ipc_alarm_in, sizeof(ms_ipc_alarm_in));
    STRUCT(ms_ipc_alarm_in, &m_ipc_alarm_in,
           FIELD(int, chanid);
           FIELD(int *, alarmEnable);
           FIELD(int *, alarmType);
           FIELD(int *, alarmStatus);
           FIELD(int, alarmCnt));

    gEventLoopExit(0);
}

void TabCameraAlarmInput::ON_RESPONSE_FLAG_SET_IPC_ALARM_INPUT_BATCH(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void TabCameraAlarmInput::setSettingEnable(bool enable)
{
    ui->widget_settings->setEnabled(enable);
    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
}

void TabCameraAlarmInput::showAlarminput(CAMERA_ALARMIN_IN_MODE index)
{
    m_currentAlarmIndex = index;

    ui->checkBox_enable->setChecked(m_ipc_alarm_in.alarmEnable[index]);
    on_checkBox_enable_stateChanged(m_ipc_alarm_in.alarmEnable[index]);
    ui->lineEdit_name->setText(m_alarmName[index]);
    ui->comboBox_type->setCurrentIndexFromData(m_ipc_alarm_in.alarmType[index]);
}

void TabCameraAlarmInput::saveAlarminput(int channel, CAMERA_ALARMIN_IN_MODE index, ms_ipc_alarm_in_cfg *info)
{
    //Alarm Name不可Copy
    if (channel == m_currentChannel) {
        writeAlarminName(index, m_alarmName[index]);
    }
    info->chanid = channel;
    info->enable = m_ipc_alarm_in.alarmEnable[index];
    info->alarmid = index;
    info->alarmType = m_ipc_alarm_in.alarmType[index];
    qMsDebug() << QString("channel: %1, alarm index: %2, enable: %3, type: %4")
                      .arg(info->chanid)
                      .arg(info->alarmid)
                      .arg(info->enable)
                      .arg(info->alarmType);
}

QString TabCameraAlarmInput::readAlarminName(CAMERA_ALARMIN_IN_MODE index)
{
    alarm_chn_name alarm_name;
    memset(&alarm_name, 0, sizeof(alarm_chn_name));

    alarm_chn alarm_channel;
    alarm_channel.chnid = m_currentChannel;
    alarm_channel.alarmid = index;
    read_alarm_chnIn_event_name(SQLITE_FILE_NAME, &alarm_name, &alarm_channel);

    return QString(alarm_name.name);
}

void TabCameraAlarmInput::writeAlarminName(CAMERA_ALARMIN_IN_MODE index, const QString &name)
{
    alarm_chn_name alarm_name;
    memset(&alarm_name, 0, sizeof(alarm_chn_name));
    alarm_name.chnid = m_currentChannel;
    alarm_name.alarmid = index;
    snprintf(alarm_name.name, sizeof(alarm_name.name), "%s", name.toStdString().c_str());

    write_alarm_chnIn_event_name(SQLITE_FILE_NAME, &alarm_name);
}

void TabCameraAlarmInput::copyAlarmInput(CAMERA_ALARMIN_IN_MODE index, quint64 copyFlags)
{
    int type = 0;
    WLED_INFO led_info_schedule;
    led_info_schedule.chnid = m_currentChannel;
    WLED_INFO led_info_params;
    led_info_params.chnid = m_currentChannel;
    switch (index) {
    case ActionCameraAlarmInput1:
        type = ALARM_CHN_IN0_EVT;
        snprintf(led_info_schedule.pDbTable, sizeof(led_info_schedule.pDbTable), "%s", AINCH0_WLED_ESCHE);
        snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", AINCH0_WLED_PARAMS);
        break;
    case ActionCameraAlarmInput2:
        type = ALARM_CHN_IN1_EVT;
        snprintf(led_info_schedule.pDbTable, sizeof(led_info_schedule.pDbTable), "%s", AINCH1_WLED_ESCHE);
        snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", AINCH1_WLED_PARAMS);
        break;
    case ActionCameraAlarmInput3:
        type = ALARM_CHN_IN2_EVT;
        snprintf(led_info_schedule.pDbTable, sizeof(led_info_schedule.pDbTable), "%s", AINCH2_WLED_ESCHE);
        snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", AINCH2_WLED_PARAMS);
        break;
    case ActionCameraAlarmInput4:
        type = ALARM_CHN_IN3_EVT;
        snprintf(led_info_schedule.pDbTable, sizeof(led_info_schedule.pDbTable), "%s", AINCH3_WLED_ESCHE);
        snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", AINCH3_WLED_PARAMS);
        break;
    }
    alarm_chn alarmChannel;
    alarmChannel.chnid = m_currentChannel;
    alarmChannel.alarmid = index;
    //effective time
    smart_event_schedule *currentEffectiveTimeSchedule = new smart_event_schedule;
    memset(currentEffectiveTimeSchedule, 0, sizeof(smart_event_schedule));
    read_alarm_chnIn_effective_schedule(SQLITE_FILE_NAME, currentEffectiveTimeSchedule, &alarmChannel);
    copy_alarm_chnIn_effective_schedules(SQLITE_FILE_NAME, currentEffectiveTimeSchedule, index, static_cast<Int64>(copyFlags));
    delete currentEffectiveTimeSchedule;

    //action
    smart_event *currentAction = new smart_event;
    memset(currentAction, 0, sizeof(smart_event));
    read_alarm_chnIn_event(SQLITE_FILE_NAME, currentAction, &alarmChannel);
    copy_alarm_chnIn_events(SQLITE_FILE_NAME, currentAction, index, static_cast<Int64>(copyFlags));
    delete currentAction;

    //action-audible
    smart_event_schedule *currentActionAudible = new smart_event_schedule;
    memset(currentActionAudible, 0, sizeof(smart_event_schedule));
    read_alarm_chnIn_audible_schedule(SQLITE_FILE_NAME, currentActionAudible, &alarmChannel);
    copy_alarm_chnIn_audible_schedules(SQLITE_FILE_NAME, currentActionAudible, index, static_cast<Int64>(copyFlags));
    delete currentActionAudible;

    //action-email
    smart_event_schedule *currentActionEmail = new smart_event_schedule;
    memset(currentActionEmail, 0, sizeof(smart_event_schedule));
    read_alarm_chnIn_mail_schedule(SQLITE_FILE_NAME, currentActionEmail, &alarmChannel);
    copy_alarm_chnIn_mail_schedules(SQLITE_FILE_NAME, currentActionEmail, index, static_cast<Int64>(copyFlags));
    delete currentActionEmail;

    //action-popup
    smart_event_schedule *currentActionPopup = new smart_event_schedule;
    memset(currentActionPopup, 0, sizeof(smart_event_schedule));
    read_alarm_chnIn_popup_schedule(SQLITE_FILE_NAME, currentActionPopup, &alarmChannel);
    copy_alarm_chnIn_popup_schedules(SQLITE_FILE_NAME, currentActionPopup, index, copyFlags);
    delete currentActionPopup;

    //action-ptz
    smart_event_schedule *currentActionPtz = new smart_event_schedule;
    memset(currentActionPtz, 0, sizeof(smart_event_schedule));
    read_alarm_chnIn_ptz_schedule(SQLITE_FILE_NAME, currentActionPtz, &alarmChannel);
    copy_alarm_chnIn_ptz_schedules(SQLITE_FILE_NAME, currentActionPtz, index, static_cast<Int64>(copyFlags));
    delete currentActionPtz;
    ptz_action_params *ptzActionArray = new ptz_action_params[MAX_CAMERA];
    memset(ptzActionArray, 0, sizeof(ptz_action_params) * MAX_CAMERA);
    int ptzActionItemCount = 0;
    read_ptz_params(SQLITE_FILE_NAME, ptzActionArray, type, m_currentChannel, &ptzActionItemCount);
    copy_ptz_params_all(SQLITE_FILE_NAME, ptzActionArray, type, static_cast<Int64>(copyFlags));
    delete[] ptzActionArray;

    //led schedule
    SMART_SCHEDULE *scheduleLed = new SMART_SCHEDULE;
    memset(scheduleLed, 0, sizeof(SMART_SCHEDULE));
    read_whiteled_effective_schedule(SQLITE_FILE_NAME, scheduleLed, &led_info_schedule);
    copy_whiteled_effective_schedules(SQLITE_FILE_NAME, scheduleLed, &led_info_schedule, copyFlags);
    delete scheduleLed;

    //led params
    WHITE_LED_PARAMS *ledParamsArray = new WHITE_LED_PARAMS[MAX_CAMERA];
    memset(ledParamsArray, 0, sizeof(WHITE_LED_PARAMS) * MAX_CAMERA);
    int led_params_count = 0;
    read_whiteled_params(SQLITE_FILE_NAME, ledParamsArray, &led_info_params, &led_params_count);
    copy_whiteled_params(SQLITE_FILE_NAME, led_info_params.pDbTable, ledParamsArray, copyFlags);
    delete[] ledParamsArray;

    //http
    copy_http_notification_action(static_cast<EVENT_IN_TYPE_E>(type), m_currentChannel, copyFlags);
}

void TabCameraAlarmInput::onLanguageChanged()
{
    ui->label_input_number->setText(GET_TEXT("ALARMIN/52002", "Alarm Input No."));
    ui->label_input->setText(GET_TEXT("ALARMIN/52001", "Alarm Input"));
    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->label_name->setText(GET_TEXT("ALARMIN/52003", "Alarm Name"));
    ui->label_type->setText(GET_TEXT("ALARMIN/52004", "Alarm Type"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->pushButton_effectiveTime->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_action->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabCameraAlarmInput::onChannelGroupClicked(int index)
{
    if (m_action) {
        m_action->clearCache();
    }
    if (m_effective) {
        m_effective->clearCache();
    }
    //
    m_currentChannel = index;
    ui->video->playVideo(index);
    ui->lineEdit_name->clear();
    ui->comboBox_type->setCurrentIndex(0);
    ui->checkBox_enable->setChecked(false);
    ui->pushButton_alarm2->setVisible(true);
    ui->pushButton_alarm3->setVisible(true);
    ui->pushButton_alarm4->setVisible(true);

    //
    do {
        if (!LiveView::instance()->isChannelConnected(m_currentChannel)) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
            break;
        }
        //
        MsWaittingContainer wait;
        memset(&m_ipc_alarm_in, 0, sizeof(ms_ipc_alarm_in));
        m_ipc_alarm_in.chanid = m_currentChannel;
        m_ipc_alarm_in.alarmCnt = MAX_IPC_ALARM_IN;

        sendMessage(REQUEST_FLAG_GET_IPC_ALARM_INPUT, &m_ipc_alarm_in, sizeof(ms_ipc_alarm_in));

        int result = gEventLoopExec();
        if (result < 0) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            break;
        }
        if (m_ipc_alarm_in.alarmCnt <= 0) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            break;
        }
        ui->pushButton_alarm2->setVisible(m_ipc_alarm_in.alarmCnt >= 2);
        ui->pushButton_alarm3->setVisible(m_ipc_alarm_in.alarmCnt >= 3);
        ui->pushButton_alarm4->setVisible(m_ipc_alarm_in.alarmCnt >= 4);
        ui->widgetMessage->hide();
        m_alarmName[ActionCameraAlarmInput1] = readAlarminName(ActionCameraAlarmInput1);
        m_alarmName[ActionCameraAlarmInput2] = readAlarminName(ActionCameraAlarmInput2);
        m_alarmName[ActionCameraAlarmInput3] = readAlarminName(ActionCameraAlarmInput3);
        m_alarmName[ActionCameraAlarmInput4] = readAlarminName(ActionCameraAlarmInput4);
        setSettingEnable(true);
        showAlarminput(ActionCameraAlarmInput1);
        ui->pushButton_alarm1->setChecked(true);
        //
        setSettingEnable(true);
        ui->widgetMessage->hide();
        return;
    } while (0);
    //
    setSettingEnable(false);
}

void TabCameraAlarmInput::onAlarmInputGroupClicked(int index)
{
    showAlarminput(CAMERA_ALARMIN_IN_MODE(index));
}

void TabCameraAlarmInput::on_checkBox_enable_clicked(bool checked)
{
    int alarmIndex = m_alarmButtonGroup->checkedId();
    m_ipc_alarm_in.alarmEnable[alarmIndex] = checked;
}

void TabCameraAlarmInput::on_lineEdit_name_textEdited(const QString &text)
{
    int alarmIndex = m_alarmButtonGroup->checkedId();
    m_alarmName[alarmIndex] = text;
}

void TabCameraAlarmInput::on_comboBox_type_activated(int index)
{
    int alarmIndex = m_alarmButtonGroup->checkedId();
    m_ipc_alarm_in.alarmType[alarmIndex] = ui->comboBox_type->itemData(index).toInt();
}

void TabCameraAlarmInput::on_pushButton_effectiveTime_clicked()
{
    m_effective->showEffectiveTime(m_currentChannel, m_currentAlarmIndex);
}

void TabCameraAlarmInput::on_pushButton_action_clicked()
{
    m_action->showAction(m_currentChannel, m_currentAlarmIndex);
}

void TabCameraAlarmInput::on_pushButton_copy_clicked()
{
    ChannelCopyDialog channelCopy(this);
    channelCopy.setCurrentChannel(m_currentChannel);
    int result = channelCopy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList = channelCopy.checkedList();
        quint64 copyFlags = channelCopy.checkedFlags();

        //
        //showWait();
        //先把当前通道保存
        m_effective->saveEffectiveTime();
        m_action->saveAction();
        ms_ipc_alarm_in_batch_cfg ipc_alarm_in_batch_cfg;
        memset(&ipc_alarm_in_batch_cfg, 0, sizeof(ms_ipc_alarm_in_batch_cfg));
        ipc_alarm_in_batch_cfg.cnt = m_ipc_alarm_in.alarmCnt;
        for (int i = 0; i < m_ipc_alarm_in.alarmCnt; i++) {
            saveAlarminput(m_currentChannel, static_cast<CAMERA_ALARMIN_IN_MODE>(i), &ipc_alarm_in_batch_cfg.cfg[i]);
        }

        //copy effective time and action
        copyAlarmInput(ActionCameraAlarmInput1, copyFlags);
        copyAlarmInput(ActionCameraAlarmInput2, copyFlags);
        copyAlarmInput(ActionCameraAlarmInput3, copyFlags);
        copyAlarmInput(ActionCameraAlarmInput4, copyFlags);
        //
        ipc_alarm_in_batch_cfg.batch = static_cast<long long>(copyFlags);
        sendMessage(REQUEST_FLAG_SET_IPC_ALARM_INPUT_BATCH, &ipc_alarm_in_batch_cfg, sizeof (ms_ipc_alarm_in_batch_cfg));
        gEventLoopExec();
        //
        sendMessageOnly(REQUEST_FLAG_SET_IPC_ALARMIN_EVENT, nullptr, 0);
        //closeWait();
    }

    ui->pushButton_copy->clearUnderMouse();
}

void TabCameraAlarmInput::on_pushButton_apply_clicked()
{
    if (!ui->lineEdit_name->checkValid()) {
        return;
    }
    //showWait();
    //
    m_effective->saveEffectiveTime();
    m_action->saveAction();
    sendMessageOnly(REQUEST_FLAG_SET_IPC_ALARMIN_EVENT, nullptr, 0);
    //

    ms_ipc_alarm_in_batch_cfg ipc_alarm_in_batch_cfg;
    memset(&ipc_alarm_in_batch_cfg, 0, sizeof(ms_ipc_alarm_in_batch_cfg));
    ipc_alarm_in_batch_cfg.cnt = m_ipc_alarm_in.alarmCnt;
    ipc_alarm_in_batch_cfg.batch |= (static_cast<quint64>(1) << m_currentChannel);
    for (int i = 0; i < m_ipc_alarm_in.alarmCnt; i++) {
        saveAlarminput(m_currentChannel, static_cast<CAMERA_ALARMIN_IN_MODE>(i), &ipc_alarm_in_batch_cfg.cfg[i]);
    }
    sendMessage(REQUEST_FLAG_SET_IPC_ALARM_INPUT_BATCH, &ipc_alarm_in_batch_cfg, sizeof (ms_ipc_alarm_in_batch_cfg));
    gEventLoopExec();
    //
    //closeWait();
}

void TabCameraAlarmInput::on_pushButton_back_clicked()
{
    emit sig_back();
}

void TabCameraAlarmInput::on_checkBox_enable_stateChanged(int arg1)
{
    Q_UNUSED(arg1)
    bool isEnable = ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked();
    ui->lineEdit_name->setEnabled(isEnable);
    ui->comboBox_type->setEnabled(isEnable);
    ui->pushButton_action->setEnabled(isEnable);
    ui->pushButton_effectiveTime->setEnabled(isEnable);
}
