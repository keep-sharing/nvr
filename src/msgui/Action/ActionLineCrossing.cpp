#include "ActionLineCrossing.h"

ActionLineCrossing::ActionLineCrossing(QWidget *parent)
    : ActionAbstract(parent)
{
}

void ActionLineCrossing::showAction(int channel, SMART_EVENT_TYPE mode)
{
    clearPageCache();
    m_eventType = mode;
    m_channel = channel;

    if (!hasCache()) {
        auto &data = m_dataMap[m_eventType];

        data.led_info_schedule.chnid = m_channel;
        data.led_info_params.chnid = m_channel;
        switch (m_eventType) {
        case LINECROSS:
            snprintf(data.led_info_schedule.pDbTable, sizeof(data.led_info_schedule.pDbTable), "%s", VLSS_WLED_ESCHE);
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", VLSS_WLED_PARAMS);
            snprintf(data.http_info_schedule.pDbTable, sizeof(data.http_info_schedule.pDbTable), "%s", VLSS_HTTP_SCHE);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", VLSS_HTTP_PARAMS);
            break;
        default:
            break;
        }

        memset(&data.event, 0, sizeof(data.event));
        read_smart_event(SQLITE_FILE_NAME, &data.event, m_channel, m_eventType);

        memset(&data.scheduleAudibleWarning, 0, sizeof(data.scheduleAudibleWarning));
        read_smart_event_audible_schedule(SQLITE_FILE_NAME, &data.scheduleAudibleWarning, m_channel, m_eventType);

        memset(&data.scheduleEmailLinkage, 0, sizeof(data.scheduleEmailLinkage));
        read_smart_event_mail_schedule(SQLITE_FILE_NAME, &data.scheduleEmailLinkage, m_channel, m_eventType);

        memset(&data.scheduleEventPopup, 0, sizeof(data.scheduleEventPopup));
        read_smart_event_popup_schedule(SQLITE_FILE_NAME, &data.scheduleEventPopup, m_channel, m_eventType);

        memset(&data.schedulePtzAction, 0, sizeof(data.schedulePtzAction));
        read_smart_event_ptz_schedule(SQLITE_FILE_NAME, &data.schedulePtzAction, m_channel, m_eventType);

        memset(&data.scheduleWhiteLed, 0, sizeof(data.scheduleWhiteLed));
        read_whiteled_effective_schedule(SQLITE_FILE_NAME, &data.scheduleWhiteLed, &data.led_info_schedule);

        memset(&data.ptzParamsArray, 0, sizeof(data.ptzParamsArray));
        read_ptz_params(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), m_channel, &data.ptzParamsCount);

        memset(&data.ledParamsArray, 0, sizeof(data.ledParamsArray));
        read_whiteled_params(SQLITE_FILE_NAME, data.ledParamsArray, &data.led_info_params, &data.ledParamsCount);

        memset(&data.scheduleHttpNotification, 0, sizeof(smart_event_schedule));
        read_http_notification_schedule(SQLITE_FILE_NAME, &data.scheduleHttpNotification, data.http_info_schedule.pDbTable, m_channel);

        memset(&data.httpNotificationParams, 0, sizeof(HttpNotificationParams));
        data.httpNotificationParams.id = m_channel;
        read_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable, m_channel);
    }
    ActionAbstract::showAction();
}

void ActionLineCrossing::saveAction()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        int type = iter.key();
        Data &data = iter.value();
        write_smart_event(SQLITE_FILE_NAME, &data.event, static_cast<SMART_EVENT_TYPE>(type));
        write_smart_event_audible_schedule(SQLITE_FILE_NAME, &data.scheduleAudibleWarning, m_channel, static_cast<SMART_EVENT_TYPE>(type));
        write_smart_event_mail_schedule(SQLITE_FILE_NAME, &data.scheduleEmailLinkage, m_channel, static_cast<SMART_EVENT_TYPE>(type));
        write_smart_event_popup_schedule(SQLITE_FILE_NAME, &data.scheduleEventPopup, m_channel, static_cast<SMART_EVENT_TYPE>(type));
        write_smart_event_ptz_schedule(SQLITE_FILE_NAME, &data.schedulePtzAction, m_channel, static_cast<SMART_EVENT_TYPE>(type));
        write_whiteled_effective_schedule(SQLITE_FILE_NAME, &data.scheduleWhiteLed, &data.led_info_schedule);
        m_eventType = static_cast<SMART_EVENT_TYPE>(type);
        write_ptz_params_all(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), m_channel);
        write_whiteled_params_all(SQLITE_FILE_NAME, data.ledParamsArray, data.led_info_params.pDbTable, m_channel);

        write_http_notification_schedule(SQLITE_FILE_NAME, &data.scheduleHttpNotification, data.http_info_schedule.pDbTable, m_channel);
        write_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable);
    }
}

bool ActionLineCrossing::hasCache() const
{
    return m_dataMap.contains(m_eventType);
}

void ActionLineCrossing::clearCache()
{
    m_dataMap.clear();
}

ActionAbstract::Tabs ActionLineCrossing::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers| TabHTTP);
}

QColor ActionLineCrossing::actionColor()
{
    return QColor(252, 174, 200);
}

int ActionLineCrossing::eventType()
{
    eventInType mode;
    switch (m_eventType) {
    case LINECROSS:
        mode = LINE_CROSS;
        break;
    default:
        mode = MAXEVT;
        break;
    }
    return mode;
}

int ActionLineCrossing::scheduleType()
{
    return SMART_EVT_RECORD;
}

schedule_day *ActionLineCrossing::audibleWarningSchedule()
{
    auto &data = m_dataMap[m_eventType];
    return data.scheduleAudibleWarning.schedule_day;
}

schedule_day *ActionLineCrossing::emailLinkageSchedule()
{
    auto &data = m_dataMap[m_eventType];
    return data.scheduleEmailLinkage.schedule_day;
}

schedule_day *ActionLineCrossing::eventPopupSchedule()
{
    auto &data = m_dataMap[m_eventType];
    return data.scheduleEventPopup.schedule_day;
}

schedule_day *ActionLineCrossing::ptzActionSchedule()
{
    auto &data = m_dataMap[m_eventType];
    return data.schedulePtzAction.schedule_day;
}

schedule_day *ActionLineCrossing::whiteLedSchedule()
{
    auto &data = m_dataMap[m_eventType];
    return data.scheduleWhiteLed.schedule_day;
}

int *ActionLineCrossing::audibleWarningTriggerInterval()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.buzzer_interval;
}

int *ActionLineCrossing::emailLinkageTriggerInterval()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.email_interval;
}

int *ActionLineCrossing::eventPopupTriggerInterval()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.popup_interval;
}

int *ActionLineCrossing::ptzActionTriggerInterval()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.ptzaction_interval;
}

int *ActionLineCrossing::alarmOutputTriggerInterval()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.alarmout_interval;
}

int *ActionLineCrossing::whiteLedTriggerInterval()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.whiteled_interval;
}

int *ActionLineCrossing::emailLinkagePictureAttached()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.email_pic_enable;
}

int *ActionLineCrossing::audioFileNo()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.tri_audio_id;
}

ptz_action_params *ActionLineCrossing::ptzActions()
{
    auto &data = m_dataMap[m_eventType];
    return data.ptzParamsArray;
}

int *ActionLineCrossing::ptzActionsCount()
{
    auto &data = m_dataMap[m_eventType];
    return &data.ptzParamsCount;
}

uint *ActionLineCrossing::nvrTriggerAlarmOutput()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.tri_alarms;
}

char *ActionLineCrossing::ch1TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_eventType];
    return data.event.tri_chnout1_alarms;
}

char *ActionLineCrossing::ch2TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_eventType];
    return data.event.tri_chnout2_alarms;
}

white_led_params *ActionLineCrossing::whiteLedParams()
{
    auto &data = m_dataMap[m_eventType];
    return data.ledParamsArray;
}

int *ActionLineCrossing::whiteLedParamsCount()
{
    auto &data = m_dataMap[m_eventType];
    return &data.ledParamsCount;
}

uint *ActionLineCrossing::triggerChannelsRecord()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.tri_channels;
}

char *ActionLineCrossing::triggerChannelsRecordEx()
{
    auto &data = m_dataMap[m_eventType];
    return data.event.tri_channels_ex;
}

char *ActionLineCrossing::triggerChannelsSnapshot()
{
    auto &data = m_dataMap[m_eventType];
    return data.event.tri_channels_pic;
}

schedule_day *ActionLineCrossing::httpNotificationSchedule()
{
    auto &data = m_dataMap[m_eventType];
    return data.scheduleHttpNotification.schedule_day;
}

int *ActionLineCrossing::httpNotificationTriggerInterval()
{
    auto &data = m_dataMap[m_eventType];
    return &data.event.http_notification_interval;
}

HttpNotificationParams *ActionLineCrossing::httpNotificationParams()
{
    auto &data = m_dataMap[m_eventType];
    return &data.httpNotificationParams;
}
