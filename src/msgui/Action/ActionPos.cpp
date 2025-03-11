#include "ActionPos.h"
#include "MyDebug.h"

ActionPos::ActionPos(QWidget *parent)
    : ActionAbstract(parent)
{
}

ActionPos::~ActionPos()
{
}

void ActionPos::showAction(int pos)
{
    clearPageCache();
    m_posId = pos;
    m_channel = pos;

    if (!hasCache()) {
        auto &data = m_dataMap[m_posId];

        data.led_info_schedule.chnid = m_posId;
        data.led_info_params.chnid = m_posId;
        snprintf(data.led_info_schedule.pDbTable, sizeof(data.led_info_schedule.pDbTable), "%s", POS_WLED_ESCHE);
        snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", POS_WLED_PARAMS);
        snprintf(data.http_info_schedule.pDbTable, sizeof(data.http_info_schedule.pDbTable), "%s", POS_HTTP_SCHE);
        snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", POS_HTTP_PARAMS);

        memset(&data.event, 0, sizeof(data.event));
        read_pos_event(SQLITE_FILE_NAME, &data.event, m_posId);

        memset(&data.scheduleAudibleWarning, 0, sizeof(data.scheduleAudibleWarning));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &data.scheduleAudibleWarning, m_posId);

        memset(&data.scheduleEmailLinkage, 0, sizeof(data.scheduleEmailLinkage));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &data.scheduleEmailLinkage, m_posId);

        memset(&data.scheduleEventPopup, 0, sizeof(data.scheduleEventPopup));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_POPUP, &data.scheduleEventPopup, m_posId);

        memset(&data.schedulePtzAction, 0, sizeof(data.schedulePtzAction));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &data.schedulePtzAction, m_posId);

        memset(&data.scheduleWhiteLed, 0, sizeof(data.scheduleWhiteLed));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &data.scheduleWhiteLed, m_posId);

        memset(&data.ptzParamsArray, 0, sizeof(data.ptzParamsArray));
        read_ptz_params(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), m_posId, &data.ptzParamsCount);

        memset(&data.ledParamsArray, 0, sizeof(data.ledParamsArray));
        read_whiteled_params(SQLITE_FILE_NAME, data.ledParamsArray, &data.led_info_params, &data.ledParamsCount);

        memset(&data.scheduleHttpNotification, 0, sizeof(smart_event_schedule));
        read_http_notification_schedule(SQLITE_FILE_NAME, &data.scheduleHttpNotification, data.http_info_schedule.pDbTable, m_posId);

        memset(&data.httpNotificationParams, 0, sizeof(HttpNotificationParams));
        data.httpNotificationParams.id = m_posId;
        read_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable, m_posId);
    }
    ActionAbstract::showAction();
}

void ActionPos::saveAction(int pos)
{
    if (m_dataMap.contains(pos)) {
        auto &data = m_dataMap[pos];
        write_pos_event(SQLITE_FILE_NAME, &data.event);
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &data.scheduleAudibleWarning, pos);
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &data.scheduleEmailLinkage, pos);
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_POPUP, &data.scheduleEventPopup, pos);
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &data.schedulePtzAction, pos);
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &data.scheduleWhiteLed, pos);
        write_ptz_params_all(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), pos);
        write_whiteled_params_all(SQLITE_FILE_NAME, data.ledParamsArray, data.led_info_params.pDbTable, pos);
        write_http_notification_schedule(SQLITE_FILE_NAME, &data.scheduleHttpNotification, data.http_info_schedule.pDbTable, pos);
        write_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable);
    }
}

bool ActionPos::hasCache() const
{
    return m_dataMap.contains(m_posId);
}

void ActionPos::clearCache()
{
    m_dataMap.clear();
}

ActionAbstract::Tabs ActionPos::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers| TabHTTP);
}

QColor ActionPos::actionColor()
{
    return QColor(255, 231, 147);
}

int ActionPos::eventType()
{
    return POS_EVT;
}

int ActionPos::scheduleType()
{
    return POS_RECORD;
}

schedule_day *ActionPos::audibleWarningSchedule()
{
    auto &data = m_dataMap[m_posId];
    return data.scheduleAudibleWarning.schedule_day;
}

schedule_day *ActionPos::emailLinkageSchedule()
{
    auto &data = m_dataMap[m_posId];
    return data.scheduleEmailLinkage.schedule_day;
}

schedule_day *ActionPos::eventPopupSchedule()
{
    auto &data = m_dataMap[m_posId];
    return data.scheduleEventPopup.schedule_day;
}

schedule_day *ActionPos::ptzActionSchedule()
{
    auto &data = m_dataMap[m_posId];
    return data.schedulePtzAction.schedule_day;
}

schedule_day *ActionPos::whiteLedSchedule()
{
    auto &data = m_dataMap[m_posId];
    return data.scheduleWhiteLed.schedule_day;
}

int *ActionPos::audibleWarningTriggerInterval()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.buzzer_interval;
}

int *ActionPos::emailLinkageTriggerInterval()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.email_interval;
}

int *ActionPos::eventPopupTriggerInterval()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.popup_interval;
}

int *ActionPos::ptzActionTriggerInterval()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.ptzaction_interval;
}

int *ActionPos::alarmOutputTriggerInterval()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.alarmout_interval;
}

int *ActionPos::whiteLedTriggerInterval()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.whiteled_interval;
}

int *ActionPos::emailLinkagePictureAttached()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.email_pic_enable;
}

int *ActionPos::audioFileNo()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.tri_audio_id;
}

ptz_action_params *ActionPos::ptzActions()
{
    auto &data = m_dataMap[m_posId];
    return data.ptzParamsArray;
}

int *ActionPos::ptzActionsCount()
{
    auto &data = m_dataMap[m_posId];
    return &data.ptzParamsCount;
}

uint *ActionPos::nvrTriggerAlarmOutput()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.tri_alarms;
}

char *ActionPos::ch1TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_posId];
    return data.event.tri_chnout1_alarms;
}

char *ActionPos::ch2TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_posId];
    return data.event.tri_chnout2_alarms;
}

white_led_params *ActionPos::whiteLedParams()
{
    auto &data = m_dataMap[m_posId];
    return data.ledParamsArray;
}

int *ActionPos::whiteLedParamsCount()
{
    auto &data = m_dataMap[m_posId];
    return &data.ledParamsCount;
}

uint *ActionPos::triggerChannelsRecord()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.tri_channels;
}

char *ActionPos::triggerChannelsRecordEx()
{
    auto &data = m_dataMap[m_posId];
    return data.event.tri_channels_ex;
}

char *ActionPos::triggerChannelsSnapshot()
{
    auto &data = m_dataMap[m_posId];
    return data.event.tri_channels_pic;
}

bool ActionPos::isNoteVisible() const
{
    return false;
}

schedule_day *ActionPos::httpNotificationSchedule()
{
    auto &data = m_dataMap[m_posId];
    return data.scheduleHttpNotification.schedule_day;
}

int *ActionPos::httpNotificationTriggerInterval()
{
    auto &data = m_dataMap[m_posId];
    return &data.event.http_notification_interval;
}

HttpNotificationParams *ActionPos::httpNotificationParams()
{
    auto &data = m_dataMap[m_posId];
    return &data.httpNotificationParams;
}
