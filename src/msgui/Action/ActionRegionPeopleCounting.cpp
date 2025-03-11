#include "ActionRegionPeopleCounting.h"
#include "MyDebug.h"

ActionRegionPeopleCounting::ActionRegionPeopleCounting(QWidget *parent)
    : ActionAbstract(parent)
{
    initialize();
}

ActionRegionPeopleCounting::~ActionRegionPeopleCounting()
{
}

void ActionRegionPeopleCounting::showAction(int channel, int region)
{
    clearPageCache();
    m_channel = channel;
    m_region = region;
    if (!hasCache()) {
        auto &data = m_dataMap[m_region];

        memset(&data.event, 0, sizeof(data.event));
        read_regional_pcnt_event(SQLITE_FILE_NAME, &data.event, m_channel, m_region);

        memset(&data.scheduleAudibleWarning, 0, sizeof(data.scheduleAudibleWarning));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &data.scheduleAudibleWarning, m_channel, m_region);

        memset(&data.scheduleEmailLinkage, 0, sizeof(data.scheduleEmailLinkage));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &data.scheduleEmailLinkage, m_channel, m_region);

        memset(&data.scheduleEventPopup, 0, sizeof(data.scheduleEventPopup));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_POPUP, &data.scheduleEventPopup, m_channel, m_region);

        memset(&data.schedulePtzAction, 0, sizeof(data.schedulePtzAction));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &data.schedulePtzAction, m_channel, m_region);

        memset(&data.scheduleWhiteLed, 0, sizeof(data.scheduleWhiteLed));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &data.scheduleWhiteLed, m_channel, m_region);

        memset(data.ptzParamsArray, 0, sizeof(data.ptzParamsArray));
        read_ptz_params(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), m_channel, &data.ptzParamsCount);

        data.led_info_params.chnid = m_channel;
        switch (m_region) {
        case 0:
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", REGIONAL_PCNT_WLED_PARAMS);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", REGIONAL_PCNT_HTTP_PARAMS);
            break;
        case 1:
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", REGIONAL_PCNT1_WLED_PARAMS);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", REGIONAL_PCNT1_HTTP_PARAMS);
            break;
        case 2:
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", REGIONAL_PCNT2_WLED_PARAMS);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", REGIONAL_PCNT2_HTTP_PARAMS);
            break;
        case 3:
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", REGIONAL_PCNT3_WLED_PARAMS);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", REGIONAL_PCNT3_HTTP_PARAMS);
            break;
        default:
            qMsWarning() << "invalid region:" << m_region;
            break;
        }
        memset(data.ledParamsArray, 0, sizeof(data.ledParamsArray));
        read_whiteled_params(SQLITE_FILE_NAME, data.ledParamsArray, &data.led_info_params, &data.ledParamsCount);

        memset(&data.scheduleHttpNotification, 0, sizeof(smart_event_schedule));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_HTTP, &data.scheduleHttpNotification, m_channel, m_region);

        memset(&data.httpNotificationParams, 0, sizeof(HttpNotificationParams));
        data.httpNotificationParams.id = m_channel;
        read_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable, m_channel);

        //
        setCached();
    }

    //
    ActionAbstract::showAction();
}

bool ActionRegionPeopleCounting::saveAction()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        int region = iter.key();
        auto &data = iter.value();
        write_regional_pcnt_event(SQLITE_FILE_NAME, &data.event, region);
        write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &data.scheduleAudibleWarning, m_channel, region);
        write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &data.scheduleEmailLinkage, m_channel, region);
        write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_POPUP, &data.scheduleEventPopup, m_channel, region);
        write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &data.schedulePtzAction, m_channel, region);
        write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &data.scheduleWhiteLed, m_channel, region);
        write_ptz_params_all(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), m_channel);
        //
        WLED_INFO led_info_params;
        led_info_params.chnid = m_channel;
        WLED_INFO http_info_params;
        switch (region) {
        case 0:
            snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", REGIONAL_PCNT_WLED_PARAMS);
            snprintf(http_info_params.pDbTable, sizeof(http_info_params.pDbTable), "%s", REGIONAL_PCNT_HTTP_PARAMS);
            break;
        case 1:
            snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", REGIONAL_PCNT1_WLED_PARAMS);
            snprintf(http_info_params.pDbTable, sizeof(http_info_params.pDbTable), "%s", REGIONAL_PCNT1_HTTP_PARAMS);
            break;
        case 2:
            snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", REGIONAL_PCNT2_WLED_PARAMS);
            snprintf(http_info_params.pDbTable, sizeof(http_info_params.pDbTable), "%s", REGIONAL_PCNT2_HTTP_PARAMS);
            break;
        case 3:
            snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", REGIONAL_PCNT3_WLED_PARAMS);
            snprintf(http_info_params.pDbTable, sizeof(http_info_params.pDbTable), "%s", REGIONAL_PCNT3_HTTP_PARAMS);
            break;
        default:
            qMsWarning() << "invalid region:" << region;
            break;
        }
        write_whiteled_params_all(SQLITE_FILE_NAME, data.ledParamsArray, led_info_params.pDbTable, m_channel);

        write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_HTTP, &data.scheduleHttpNotification, m_channel, region);
        write_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, http_info_params.pDbTable);
    }
    return true;
}

bool ActionRegionPeopleCounting::copyAction(int channel, quint64 copyFlags)
{
    //先读取全部region的数据再copy
    m_channel = channel;
    for (int i = 0; i < MAX_IPC_PCNT_REGION; i++) {
        auto &data = m_dataMap[i];

        memset(&data.event, 0, sizeof(data.event));
        read_regional_pcnt_event(SQLITE_FILE_NAME, &data.event, m_channel, i);

        memset(&data.scheduleAudibleWarning, 0, sizeof(data.scheduleAudibleWarning));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &data.scheduleAudibleWarning, m_channel, i);

        memset(&data.scheduleEmailLinkage, 0, sizeof(data.scheduleEmailLinkage));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &data.scheduleEmailLinkage, m_channel, i);

        memset(&data.scheduleEventPopup, 0, sizeof(data.scheduleEventPopup));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_POPUP, &data.scheduleEventPopup, m_channel, i);

        memset(&data.schedulePtzAction, 0, sizeof(data.schedulePtzAction));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &data.schedulePtzAction, m_channel, i);

        memset(&data.scheduleWhiteLed, 0, sizeof(data.scheduleWhiteLed));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &data.scheduleWhiteLed, m_channel, i);

        memset(data.ptzParamsArray, 0, sizeof(data.ptzParamsArray));
        read_ptz_params(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), m_channel, &data.ptzParamsCount);

        data.led_info_params.chnid = m_channel;
        switch (i) {
        case 0:
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", REGIONAL_PCNT_WLED_PARAMS);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", REGIONAL_PCNT_HTTP_PARAMS);
            break;
        case 1:
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", REGIONAL_PCNT1_WLED_PARAMS);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", REGIONAL_PCNT1_HTTP_PARAMS);
            break;
        case 2:
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", REGIONAL_PCNT2_WLED_PARAMS);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", REGIONAL_PCNT2_HTTP_PARAMS);
            break;
        case 3:
            snprintf(data.led_info_params.pDbTable, sizeof(data.led_info_params.pDbTable), "%s", REGIONAL_PCNT3_WLED_PARAMS);
            snprintf(data.http_info_params.pDbTable, sizeof(data.http_info_params.pDbTable), "%s", REGIONAL_PCNT3_HTTP_PARAMS);
            break;
        default:
            qMsWarning() << "invalid region:" << i;
            break;
        }
        memset(data.ledParamsArray, 0, sizeof(data.ledParamsArray));
        read_whiteled_params(SQLITE_FILE_NAME, data.ledParamsArray, &data.led_info_params, &data.ledParamsCount);

        memset(&data.scheduleHttpNotification, 0, sizeof(smart_event_schedule));
        read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_HTTP, &data.scheduleHttpNotification, m_channel, i);

        memset(&data.httpNotificationParams, 0, sizeof(HttpNotificationParams));
        read_http_notification_params(SQLITE_FILE_NAME, &data.httpNotificationParams, data.http_info_params.pDbTable, m_channel);
    }
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        int region = iter.key();
        auto &data = iter.value();
        copy_regional_pcnt_event(SQLITE_FILE_NAME, &data.event, region, copyFlags);
        copy_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &data.scheduleAudibleWarning, copyFlags, region);
        copy_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &data.scheduleEmailLinkage, copyFlags, region);
        copy_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_POPUP, &data.scheduleEventPopup, copyFlags, region);
        copy_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &data.schedulePtzAction, copyFlags, region);
        copy_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &data.scheduleWhiteLed, copyFlags, region);
        copy_ptz_params_all(SQLITE_FILE_NAME, data.ptzParamsArray, eventType(), static_cast<long long>(copyFlags));
        //
        WLED_INFO led_info_params;
        led_info_params.chnid = m_channel;
        WLED_INFO http_info_params;
        switch (region) {
        case 0:
            snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", REGIONAL_PCNT_WLED_PARAMS);
            snprintf(http_info_params.pDbTable, sizeof(http_info_params.pDbTable), "%s", REGIONAL_PCNT_HTTP_PARAMS);
            break;
        case 1:
            snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", REGIONAL_PCNT1_WLED_PARAMS);
            snprintf(http_info_params.pDbTable, sizeof(http_info_params.pDbTable), "%s", REGIONAL_PCNT1_HTTP_PARAMS);
            break;
        case 2:
            snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", REGIONAL_PCNT2_WLED_PARAMS);
            snprintf(http_info_params.pDbTable, sizeof(http_info_params.pDbTable), "%s", REGIONAL_PCNT2_HTTP_PARAMS);
            break;
        case 3:
            snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", REGIONAL_PCNT3_WLED_PARAMS);
            snprintf(http_info_params.pDbTable, sizeof(http_info_params.pDbTable), "%s", REGIONAL_PCNT3_HTTP_PARAMS);
            break;
        default:
            qMsWarning() << "invalid region:" << region;
            break;
        }
        copy_whiteled_params(SQLITE_FILE_NAME, led_info_params.pDbTable, data.ledParamsArray, static_cast<Uint64>(copyFlags));

        copy_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_HTTP, &data.scheduleHttpNotification, copyFlags, region);
        copy_http_notification_params(SQLITE_FILE_NAME, http_info_params.pDbTable, &data.httpNotificationParams, static_cast<Uint64>(copyFlags));
    }
    return true;
}

bool ActionRegionPeopleCounting::hasCache() const
{
    return m_dataMap.contains(m_region);
}

void ActionRegionPeopleCounting::clearCache()
{
    m_dataMap.clear();
}

ActionAbstract::Tabs ActionRegionPeopleCounting::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers | TabHTTP);
}

QColor ActionRegionPeopleCounting::actionColor()
{
    return QColor(255, 231, 147);
}

int ActionRegionPeopleCounting::eventType()
{
    int type = 0;
    switch (m_region) {
    case 0:
        return REGIONAL_PEOPLE_CNT0;
    case 1:
        return REGIONAL_PEOPLE_CNT1;
    case 2:
        return REGIONAL_PEOPLE_CNT2;
    case 3:
        return REGIONAL_PEOPLE_CNT3;
    }
    return type;
}

int ActionRegionPeopleCounting::scheduleType()
{
    return REGIONAL_RECORD;
}

schedule_day *ActionRegionPeopleCounting::audibleWarningSchedule()
{
    auto &data = m_dataMap[m_region];
    return data.scheduleAudibleWarning.schedule_day;
}

schedule_day *ActionRegionPeopleCounting::emailLinkageSchedule()
{
    auto &data = m_dataMap[m_region];
    return data.scheduleEmailLinkage.schedule_day;
}

schedule_day *ActionRegionPeopleCounting::eventPopupSchedule()
{
    auto &data = m_dataMap[m_region];
    return data.scheduleEventPopup.schedule_day;
}

schedule_day *ActionRegionPeopleCounting::ptzActionSchedule()
{
    auto &data = m_dataMap[m_region];
    return data.schedulePtzAction.schedule_day;
}

schedule_day *ActionRegionPeopleCounting::whiteLedSchedule()
{
    auto &data = m_dataMap[m_region];
    return data.scheduleWhiteLed.schedule_day;
}

int *ActionRegionPeopleCounting::audibleWarningTriggerInterval()
{
    auto &data = m_dataMap[m_region];
    return &data.event.buzzer_interval;
}

int *ActionRegionPeopleCounting::emailLinkageTriggerInterval()
{
    auto &data = m_dataMap[m_region];
    return &data.event.email_interval;
}

int *ActionRegionPeopleCounting::eventPopupTriggerInterval()
{
    auto &data = m_dataMap[m_region];
    return &data.event.popup_interval;
}

int *ActionRegionPeopleCounting::ptzActionTriggerInterval()
{
    auto &data = m_dataMap[m_region];
    return &data.event.ptzaction_interval;
}

int *ActionRegionPeopleCounting::alarmOutputTriggerInterval()
{
    auto &data = m_dataMap[m_region];
    return &data.event.alarmout_interval;
}

int *ActionRegionPeopleCounting::whiteLedTriggerInterval()
{
    auto &data = m_dataMap[m_region];
    return &data.event.whiteled_interval;
}

int *ActionRegionPeopleCounting::emailLinkagePictureAttached()
{
    auto &data = m_dataMap[m_region];
    return &data.event.email_pic_enable;
}

int *ActionRegionPeopleCounting::audioFileNo()
{
    auto &data = m_dataMap[m_region];
    return &data.event.tri_audio_id;
}

ptz_action_params *ActionRegionPeopleCounting::ptzActions()
{
    auto &data = m_dataMap[m_region];
    return data.ptzParamsArray;
}

int *ActionRegionPeopleCounting::ptzActionsCount()
{
    auto &data = m_dataMap[m_region];
    return &data.ptzParamsCount;
}

uint *ActionRegionPeopleCounting::nvrTriggerAlarmOutput()
{
    auto &data = m_dataMap[m_region];
    return &data.event.tri_alarms;
}

char *ActionRegionPeopleCounting::ch1TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_region];
    return data.event.tri_chnout1_alarms;
}

char *ActionRegionPeopleCounting::ch2TriggerAlarmOutput()
{
    auto &data = m_dataMap[m_region];
    return data.event.tri_chnout2_alarms;
}

white_led_params *ActionRegionPeopleCounting::whiteLedParams()
{
    auto &data = m_dataMap[m_region];
    return data.ledParamsArray;
}

int *ActionRegionPeopleCounting::whiteLedParamsCount()
{
    auto &data = m_dataMap[m_region];
    return &data.ledParamsCount;
}

uint *ActionRegionPeopleCounting::triggerChannelsRecord()
{
    auto &data = m_dataMap[m_region];
    return &data.event.tri_channels;
}

char *ActionRegionPeopleCounting::triggerChannelsRecordEx()
{
    auto &data = m_dataMap[m_region];
    return data.event.tri_channels_ex;
}

char *ActionRegionPeopleCounting::triggerChannelsSnapshot()
{
    auto &data = m_dataMap[m_region];
    return data.event.tri_channels_pic;
}

schedule_day *ActionRegionPeopleCounting::httpNotificationSchedule()
{
    auto &data = m_dataMap[m_region];
    return data.scheduleHttpNotification.schedule_day;
}

int *ActionRegionPeopleCounting::httpNotificationTriggerInterval()
{
    auto &data = m_dataMap[m_region];
    return &data.event.http_notification_interval;
}

HttpNotificationParams *ActionRegionPeopleCounting::httpNotificationParams()
{
    auto &data = m_dataMap[m_region];
    return &data.httpNotificationParams;
}
