#include "ActionOccupancy.h"
#include "EventLoop.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}

ActionOccupancy::ActionOccupancy(QWidget *parent)
    : ActionAbstract(parent)
{
}

ActionOccupancy::~ActionOccupancy()
{
}

void ActionOccupancy::showAction(int group)
{
    clearPageCache();

    m_group = group;
    m_channel = group;
    if (!hasCache()) {
        auto &data = m_dataMap[m_group];

        snprintf(data.http_info_schedule.pDbTable, sizeof(data.http_info_schedule.pDbTable), "%s", PCNT_HTTP_SCHE);
        snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", PCNT_HTTP_PARAMS);

        memset(&data.event, 0, sizeof(data.event));
        read_peoplecnt_event(SQLITE_FILE_NAME, &data.event, m_group);

        memset(&data.scheduleAudibleWarning, 0, sizeof(data.scheduleAudibleWarning));
        read_peoplecnt_audible_schedule(SQLITE_FILE_NAME, &data.scheduleAudibleWarning, m_group);

        memset(&data.scheduleEmailLinkage, 0, sizeof(data.scheduleEmailLinkage));
        read_peoplecnt_email_schedule(SQLITE_FILE_NAME, &data.scheduleEmailLinkage, m_group);

        memset(&data.schedulePtzAction, 0, sizeof(data.schedulePtzAction));
        read_peoplecnt_ptz_schedule(SQLITE_FILE_NAME, &data.schedulePtzAction, m_group);

        memset(&data.scheduleWhiteLed, 0, sizeof(data.scheduleWhiteLed));
        read_peoplecnt_wled_schedule(SQLITE_FILE_NAME, &data.scheduleWhiteLed, m_group);

        memset(data.ptzParamsArray, 0, sizeof(data.ptzParamsArray));
        read_ptz_params(SQLITE_FILE_NAME, data.ptzParamsArray, PRIVATE_PEOPLE_CNT, m_group, &data.ptzParamsCount);

        memset(data.ledParamsArray, 0, sizeof(data.ledParamsArray));
        read_peoplecnt_wled_params(SQLITE_FILE_NAME, (peoplecnt_wled_params *)data.ledParamsArray, m_group, &data.ledParamsCount);

        memset(&data.scheduleHttpNotification, 0, sizeof(smart_event_schedule));
        read_http_notification_schedule(SQLITE_FILE_NAME, &data.scheduleHttpNotification, data.http_info_schedule.pDbTable, m_channel);

        memset(&data.httpNotificationParams, 0, sizeof(HttpNotificationParams));
        data.httpNotificationParams.id = m_channel;
        read_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable, m_channel);
    }

    ActionAbstract::showAction();
}

void ActionOccupancy::saveAction()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        int group = iter.key();
        Data &data = iter.value();
        write_peoplecnt_event(SQLITE_FILE_NAME, &data.event);
        write_peoplecnt_ptz_schedule(SQLITE_FILE_NAME, &data.schedulePtzAction, group);
        write_peoplecnt_audible_schedule(SQLITE_FILE_NAME, &data.scheduleAudibleWarning, group);
        write_peoplecnt_email_schedule(SQLITE_FILE_NAME, &data.scheduleEmailLinkage, group);
        write_peoplecnt_wled_schedule(SQLITE_FILE_NAME, &data.scheduleWhiteLed, group);
        write_ptz_params_all(SQLITE_FILE_NAME, data.ptzParamsArray, PRIVATE_PEOPLE_CNT, group);
        //peoplecnt_wled_params *temp = (peoplecnt_wled_params *)data.ledParamsArray;
        write_peoplecnt_wled_params_all(SQLITE_FILE_NAME, (peoplecnt_wled_params *)data.ledParamsArray, group);

        write_http_notification_schedule(SQLITE_FILE_NAME, &data.scheduleHttpNotification, data.http_info_schedule.pDbTable, group);
        write_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable);
    }
    m_dataMap.clear();
}

bool ActionOccupancy::hasCache() const
{
    return m_dataMap.contains(m_group);
}

void ActionOccupancy::clearCache()
{
    m_dataMap.clear();
}

ActionAbstract::Tabs ActionOccupancy::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabPtzAction | TabAlarmOutput | TabWhiteLed| TabHTTP);
}

QColor ActionOccupancy::actionColor()
{
    return QColor(255, 231, 147);
}

int ActionOccupancy::eventType()
{
    return PRIVATE_PEOPLE_CNT;
}

int ActionOccupancy::scheduleType()
{
    return PRI_PEOPLECNT_ACTION;
}

schedule_day *ActionOccupancy::audibleWarningSchedule()
{
    auto &data = m_dataMap[m_group];
    return data.scheduleAudibleWarning.schedule_day;
}

schedule_day *ActionOccupancy::emailLinkageSchedule()
{
    auto &data = m_dataMap[m_group];
    return data.scheduleEmailLinkage.schedule_day;
}

schedule_day *ActionOccupancy::eventPopupSchedule()
{
    return nullptr;
}

schedule_day *ActionOccupancy::ptzActionSchedule()
{
    auto &data = m_dataMap[m_group];
    return data.schedulePtzAction.schedule_day;
}

schedule_day *ActionOccupancy::whiteLedSchedule()
{
    auto &data = m_dataMap[m_group];
    return data.scheduleWhiteLed.schedule_day;
}

int *ActionOccupancy::audibleWarningTriggerInterval()
{
    auto &data = m_dataMap[m_group];
    return &data.event.buzzer_interval;
}

int *ActionOccupancy::emailLinkageTriggerInterval()
{
    auto &data = m_dataMap[m_group];
    return &data.event.email_interval;
}

int *ActionOccupancy::eventPopupTriggerInterval()
{
    return nullptr;
}

int *ActionOccupancy::ptzActionTriggerInterval()
{
    auto &data = m_dataMap[m_group];
    return &data.event.ptzaction_interval;
}

int *ActionOccupancy::alarmOutputTriggerInterval()
{
    auto &data = m_dataMap[m_group];
    return &data.event.alarmout_interval;
}

int *ActionOccupancy::whiteLedTriggerInterval()
{
    auto &data = m_dataMap[m_group];
    return &data.event.whiteled_interval;
}

int *ActionOccupancy::emailLinkagePictureAttached()
{
    return nullptr;
}

int *ActionOccupancy::audioFileNo()
{
    auto &data = m_dataMap[m_group];
    return &data.event.tri_audio_id;
}

ptz_action_params *ActionOccupancy::ptzActions()
{
    auto &data = m_dataMap[m_group];
    return data.ptzParamsArray;
}

int *ActionOccupancy::ptzActionsCount()
{
    auto &data = m_dataMap[m_group];
    return &data.ptzParamsCount;
}

uint *ActionOccupancy::nvrTriggerAlarmOutput()
{
    auto &data = m_dataMap[m_group];
    return &data.event.tri_alarms;
}

char *ActionOccupancy::ch1TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_group];
    return data.event.tri_chnout1_alarms;
}

char *ActionOccupancy::ch2TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_group];
    return data.event.tri_chnout2_alarms;
}

white_led_params *ActionOccupancy::whiteLedParams()
{
    auto &data = m_dataMap[m_group];
    return data.ledParamsArray;
}

int *ActionOccupancy::whiteLedParamsCount()
{
    auto &data = m_dataMap[m_group];
    return &data.ledParamsCount;
}

uint *ActionOccupancy::triggerChannelsRecord()
{
    return nullptr;
}

char *ActionOccupancy::triggerChannelsRecordEx()
{
    return nullptr;
}

char *ActionOccupancy::triggerChannelsSnapshot()
{
    return nullptr;
}

bool ActionOccupancy::isNoteVisible() const
{
    return false;
}

schedule_day *ActionOccupancy::httpNotificationSchedule()
{
    auto &data = m_dataMap[m_group];
    return data.scheduleHttpNotification.schedule_day;
}

int *ActionOccupancy::httpNotificationTriggerInterval()
{
    auto &data = m_dataMap[m_group];
    return &data.event.http_notification_interval;
}

HttpNotificationParams *ActionOccupancy::httpNotificationParams()
{
    auto &data = m_dataMap[m_group];
    return &data.httpNotificationParams;
}
