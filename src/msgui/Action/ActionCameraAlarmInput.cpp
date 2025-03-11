#include "ActionCameraAlarmInput.h"
#include "EventLoop.h"
#include "MsMessage.h"
#include "MyDebug.h"

extern "C" {
#include "msdb.h"
}

ActionCameraAlarmInput::ActionCameraAlarmInput(QWidget *parent)
    : ActionAbstract(parent)
{
    initialize();
}

ActionCameraAlarmInput::~ActionCameraAlarmInput()
{
}

void ActionCameraAlarmInput::showAction(int channel, CAMERA_ALARMIN_IN_MODE alarmIndex)
{
    clearPageCache();
    m_channel = channel;
    m_alarmIndex = alarmIndex;

    if (!hasCache()) {
        auto &data = m_dataMap[m_alarmIndex];

        data.alarm_id = m_alarmIndex;

        alarm_chn alarmChannel;
        alarmChannel.chnid = m_channel;
        alarmChannel.alarmid = data.alarm_id;

        data.led_info_schedule.chnid = m_channel;
        data.led_info_params.chnid = m_channel;
        switch (data.alarm_id) {
        case ActionCameraAlarmInput1:
            snprintf(data.led_info_schedule.pDbTable, sizeof(data.led_info_schedule.pDbTable), "%s", AINCH0_WLED_ESCHE);
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", AINCH0_WLED_PARAMS);
            snprintf(data.http_info_schedule.pDbTable, sizeof(data.http_info_schedule.pDbTable), "%s", AINCH0_HTTP_SCHE);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", AINCH0_HTTP_PARAMS);
            break;
        case ActionCameraAlarmInput2:
            snprintf(data.led_info_schedule.pDbTable, sizeof(data.led_info_schedule.pDbTable), "%s", AINCH1_WLED_ESCHE);
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", AINCH1_WLED_PARAMS);
            snprintf(data.http_info_schedule.pDbTable, sizeof(data.http_info_schedule.pDbTable), "%s", AINCH1_HTTP_SCHE);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", AINCH1_HTTP_PARAMS);
            break;
        case ActionCameraAlarmInput3:
            snprintf(data.led_info_schedule.pDbTable, sizeof(data.led_info_schedule.pDbTable), "%s", AINCH2_WLED_ESCHE);
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", AINCH2_WLED_PARAMS);
            snprintf(data.http_info_schedule.pDbTable, sizeof(data.http_info_schedule.pDbTable), "%s", AINCH2_HTTP_SCHE);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", AINCH2_HTTP_PARAMS);
            break;
        case ActionCameraAlarmInput4:
            snprintf(data.led_info_schedule.pDbTable, sizeof(data.led_info_schedule.pDbTable), "%s", AINCH3_WLED_ESCHE);
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", AINCH3_WLED_PARAMS);
            snprintf(data.http_info_schedule.pDbTable, sizeof(data.http_info_schedule.pDbTable), "%s", AINCH3_HTTP_SCHE);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", AINCH3_HTTP_PARAMS);
            break;
        }

        memset(&data.event, 0, sizeof(data.event));
        read_alarm_chnIn_event(SQLITE_FILE_NAME, &data.event, &alarmChannel);

        memset(&data.scheduleAudibleWarning, 0, sizeof(data.scheduleAudibleWarning));
        read_alarm_chnIn_audible_schedule(SQLITE_FILE_NAME, &data.scheduleAudibleWarning, &alarmChannel);

        memset(&data.scheduleEmailLinkage, 0, sizeof(data.scheduleEmailLinkage));
        read_alarm_chnIn_mail_schedule(SQLITE_FILE_NAME, &data.scheduleEmailLinkage, &alarmChannel);

        memset(&data.scheduleEventPopup, 0, sizeof(data.scheduleEventPopup));
        read_alarm_chnIn_popup_schedule(SQLITE_FILE_NAME, &data.scheduleEventPopup, &alarmChannel);

        memset(&data.schedulePtzAction, 0, sizeof(data.schedulePtzAction));
        read_alarm_chnIn_ptz_schedule(SQLITE_FILE_NAME, &data.schedulePtzAction, &alarmChannel);

        memset(&data.scheduleWhiteLed, 0, sizeof(data.scheduleWhiteLed));
        read_whiteled_effective_schedule(SQLITE_FILE_NAME, &data.scheduleWhiteLed, &data.led_info_schedule);

        memset(data.ptzParamsArray, 0, sizeof(data.ptzParamsArray));
        read_ptz_params(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), m_channel, &data.ptzParamsCount);

        memset(data.ledParamsArray, 0, sizeof(data.ledParamsArray));
        read_whiteled_params(SQLITE_FILE_NAME, data.ledParamsArray, &data.led_info_params, &data.ledParamsCount);

        memset(&data.scheduleHttpNotification, 0, sizeof(smart_event_schedule));
        read_http_notification_schedule(SQLITE_FILE_NAME, &data.scheduleHttpNotification, data.http_info_schedule.pDbTable, m_channel);

        memset(&data.httpNotificationParams, 0, sizeof(HttpNotificationParams));
        data.httpNotificationParams.id = m_channel;
        read_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable, m_channel);
        setCached();
    }
    //
    ActionAbstract::showAction();
}

void ActionCameraAlarmInput::saveAction()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        Data &data = iter.value();

        alarm_chn alarmChannel;
        alarmChannel.chnid = m_channel;
        alarmChannel.alarmid = data.alarm_id;

        write_alarm_chnIn_event(SQLITE_FILE_NAME, &data.event, &alarmChannel);

        write_alarm_chnIn_ptz_schedule(SQLITE_FILE_NAME, &data.schedulePtzAction, &alarmChannel);
        write_alarm_chnIn_audible_schedule(SQLITE_FILE_NAME, &data.scheduleAudibleWarning, &alarmChannel);
        write_alarm_chnIn_mail_schedule(SQLITE_FILE_NAME, &data.scheduleEmailLinkage, &alarmChannel);
        write_alarm_chnIn_popup_schedule(SQLITE_FILE_NAME, &data.scheduleEventPopup, &alarmChannel);
        write_whiteled_effective_schedule(SQLITE_FILE_NAME, &data.scheduleWhiteLed, &data.led_info_schedule);

        write_ptz_params_all(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), m_channel);
        write_whiteled_params_all(SQLITE_FILE_NAME, data.ledParamsArray, data.led_info_params.pDbTable, m_channel);

        write_http_notification_schedule(SQLITE_FILE_NAME, &data.scheduleHttpNotification, data.http_info_schedule.pDbTable, m_channel);
        write_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable);
    }
}

bool ActionCameraAlarmInput::hasCache() const
{
    return m_dataMap.contains(m_alarmIndex);
}

void ActionCameraAlarmInput::clearCache()
{
    m_dataMap.clear();
    clearPageCache();
}

ActionAbstract::Tabs ActionCameraAlarmInput::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers | TabHTTP);
}

QColor ActionCameraAlarmInput::actionColor()
{
    return QColor(QString("#FF3600"));
}

int ActionCameraAlarmInput::eventType()
{
    int type = 0;
    switch (m_alarmIndex) {
    case ActionCameraAlarmInput1:
        type = ALARM_CHN_IN0_EVT;
        break;
    case ActionCameraAlarmInput2:
        type = ALARM_CHN_IN1_EVT;
        break;
    case ActionCameraAlarmInput3:
        type = ALARM_CHN_IN2_EVT;
        break;
    case ActionCameraAlarmInput4:
        type = ALARM_CHN_IN3_EVT;
        break;
    }
    return type;
}

int ActionCameraAlarmInput::scheduleType()
{
    return ALARMIN_ACTION;
}

schedule_day *ActionCameraAlarmInput::audibleWarningSchedule()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.scheduleAudibleWarning.schedule_day;
}

schedule_day *ActionCameraAlarmInput::emailLinkageSchedule()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.scheduleEmailLinkage.schedule_day;
}

schedule_day *ActionCameraAlarmInput::eventPopupSchedule()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.scheduleEventPopup.schedule_day;
}

schedule_day *ActionCameraAlarmInput::ptzActionSchedule()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.schedulePtzAction.schedule_day;
}

schedule_day *ActionCameraAlarmInput::whiteLedSchedule()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.scheduleWhiteLed.schedule_day;
}

int *ActionCameraAlarmInput::audibleWarningTriggerInterval()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.buzzer_interval;
}

int *ActionCameraAlarmInput::emailLinkageTriggerInterval()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.email_interval;
}

int *ActionCameraAlarmInput::eventPopupTriggerInterval()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.popup_interval;
}

int *ActionCameraAlarmInput::ptzActionTriggerInterval()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.ptzaction_interval;
}

int *ActionCameraAlarmInput::alarmOutputTriggerInterval()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.alarmout_interval;
}

int *ActionCameraAlarmInput::whiteLedTriggerInterval()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.whiteled_interval;
}

int *ActionCameraAlarmInput::emailLinkagePictureAttached()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.email_pic_enable;
}

int *ActionCameraAlarmInput::audioFileNo()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.tri_audio_id;
}

ptz_action_params *ActionCameraAlarmInput::ptzActions()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.ptzParamsArray;
}

int *ActionCameraAlarmInput::ptzActionsCount()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.ptzParamsCount;
}

uint *ActionCameraAlarmInput::nvrTriggerAlarmOutput()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.tri_alarms;
}

char *ActionCameraAlarmInput::ch1TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.event.tri_chnout1_alarms;
}

char *ActionCameraAlarmInput::ch2TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.event.tri_chnout2_alarms;
}

white_led_params *ActionCameraAlarmInput::whiteLedParams()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.ledParamsArray;
}

int *ActionCameraAlarmInput::whiteLedParamsCount()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.ledParamsCount;
}

uint *ActionCameraAlarmInput::triggerChannelsRecord()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.tri_channels;
}

char *ActionCameraAlarmInput::triggerChannelsRecordEx()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.event.tri_channels_ex;
}

char *ActionCameraAlarmInput::triggerChannelsSnapshot()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.event.tri_channels_pic;
}

schedule_day *ActionCameraAlarmInput::httpNotificationSchedule()
{
    auto &data = m_dataMap[m_alarmIndex];
    return data.scheduleHttpNotification.schedule_day;
}

int *ActionCameraAlarmInput::httpNotificationTriggerInterval()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.event.http_notification_interval;
}

HttpNotificationParams *ActionCameraAlarmInput::httpNotificationParams()
{
    auto &data = m_dataMap[m_alarmIndex];
    return &data.httpNotificationParams;
}
