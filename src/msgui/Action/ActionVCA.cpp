#include "ActionVCA.h"
#include "MsMessage.h"
#include "MyDebug.h"

ActionSmartEvent::ActionSmartEvent(QWidget *parent)
    : ActionAbstract(parent)
{
    initialize();

    m_smartEvent = new smart_event;
    m_scheduleAudibleWarning = new smart_event_schedule;
    m_scheduleEmailLinkage = new smart_event_schedule;
    m_scheduleEventPopup = new smart_event_schedule;
    m_schedulePtzAction = new smart_event_schedule;
    m_ptzParamsArray = new ptz_action_params[MAX_CAMERA];
    m_scheduleWhiteLed = new smart_event_schedule;
    m_ledParamsArray = new white_led_params[MAX_CAMERA];
    m_scheduleHttpNotification = new smart_event_schedule;
    m_httpNotificationParams = new HttpNotificationParams;
}

ActionSmartEvent::~ActionSmartEvent()
{
    delete m_smartEvent;
    m_smartEvent = nullptr;

    delete m_scheduleAudibleWarning;
    m_scheduleAudibleWarning = nullptr;

    delete m_scheduleEmailLinkage;
    m_scheduleEmailLinkage = nullptr;

    delete m_scheduleEventPopup;
    m_scheduleEventPopup = nullptr;

    delete m_schedulePtzAction;
    m_schedulePtzAction = nullptr;

    delete[] m_ptzParamsArray;
    m_ptzParamsArray = nullptr;

    delete m_scheduleWhiteLed;
    m_scheduleWhiteLed = nullptr;

    delete[] m_ledParamsArray;
    m_ledParamsArray = nullptr;

    delete m_scheduleHttpNotification;
    m_scheduleHttpNotification = nullptr;

    delete m_httpNotificationParams;
    m_httpNotificationParams = nullptr;
}

void ActionSmartEvent::showAction(int channel, SMART_EVENT_TYPE mode)
{
    if (hasCache()) {
    } else {
        m_channel = channel;
        m_smartEventType = mode;

        m_led_info_schedule.chnid = m_channel;
        m_led_info_params.chnid = m_channel;
        switch (m_smartEventType) {
        case REGIONIN:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VREIN_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VREIN_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VREIN_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VREIN_HTTP_PARAMS);
            break;
        case REGIONOUT:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VREEX_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VREEX_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VREEX_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VREEX_HTTP_PARAMS);
            break;
        case ADVANCED_MOTION:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VMOT_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VMOT_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VMOT_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VMOT_HTTP_PARAMS);
            break;
        case TAMPER:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VTEP_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VTEP_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VTEP_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VTEP_HTTP_PARAMS);
            break;
        case LINECROSS:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VLSS_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VLSS_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VLSS_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VLSS_HTTP_PARAMS);
            break;
        case LOITERING:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VLER_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VLER_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VLER_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VLER_HTTP_PARAMS);
            break;
        case HUMAN:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VHMN_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VHMN_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VHMN_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VHMN_HTTP_PARAMS);
            break;
        case PEOPLE_CNT:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VPPE_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VPPE_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VPPE_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VPPE_HTTP_PARAMS);
            break;
        case OBJECT_LEFTREMOVE:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VOBJ_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VOBJ_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VOBJ_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VOBJ_HTTP_PARAMS);
            break;
        default:
            break;
        }

        memset(m_smartEvent, 0, sizeof(smart_event));
        read_smart_event(SQLITE_FILE_NAME, m_smartEvent, m_channel, m_smartEventType);

        memset(m_scheduleAudibleWarning, 0, sizeof(smart_event_schedule));
        read_smart_event_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel, m_smartEventType);

        memset(m_scheduleEmailLinkage, 0, sizeof(smart_event_schedule));
        read_smart_event_mail_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel, m_smartEventType);

        memset(m_scheduleEventPopup, 0, sizeof(smart_event_schedule));
        read_smart_event_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel, m_smartEventType);

        memset(m_schedulePtzAction, 0, sizeof(smart_event_schedule));
        read_smart_event_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel, m_smartEventType);

        memset(m_ptzParamsArray, 0, sizeof(ptz_action_params) * MAX_CAMERA);
        read_ptz_params(SQLITE_FILE_NAME, m_ptzParamsArray, eventType(), m_channel, &m_ptzParamsCount);

        memset(m_scheduleWhiteLed, 0, sizeof(smart_event_schedule));
        read_whiteled_effective_schedule(SQLITE_FILE_NAME, m_scheduleWhiteLed, &m_led_info_schedule);

        memset(m_ledParamsArray, 0, sizeof(white_led_params) * MAX_CAMERA);
        read_whiteled_params(SQLITE_FILE_NAME, m_ledParamsArray, &m_led_info_params, &m_ledParamsCount);

        memset(m_scheduleHttpNotification, 0, sizeof(smart_event_schedule));
        read_http_notification_schedule(SQLITE_FILE_NAME, m_scheduleHttpNotification, m_http_info_schedule.pDbTable, m_channel);

        memset(m_httpNotificationParams, 0, sizeof(HttpNotificationParams));
        m_httpNotificationParams->id = m_channel;
        read_http_notification_params(SQLITE_FILE_NAME, m_httpNotificationParams, m_http_info_params.pDbTable, m_channel);

        //
        setCached();
    }

    //
    ActionAbstract::showAction();
}

void ActionSmartEvent::saveAction()
{
    if (!hasCache()) {
        return;
    }
    write_smart_event(SQLITE_FILE_NAME, m_smartEvent, m_smartEventType);
    write_smart_event_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel, m_smartEventType);
    write_smart_event_mail_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel, m_smartEventType);
    write_smart_event_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel, m_smartEventType);
    write_smart_event_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel, m_smartEventType);
    write_ptz_params_all(SQLITE_FILE_NAME, m_ptzParamsArray, eventType(), m_channel);
    write_whiteled_effective_schedule(SQLITE_FILE_NAME, m_scheduleWhiteLed, &m_led_info_schedule);
    write_whiteled_params_all(SQLITE_FILE_NAME, m_ledParamsArray, m_led_info_params.pDbTable, m_channel);

    write_http_notification_schedule(SQLITE_FILE_NAME, m_scheduleHttpNotification, m_http_info_schedule.pDbTable, m_channel);
    write_http_notification_params(SQLITE_FILE_NAME, m_httpNotificationParams, m_http_info_params.pDbTable);
}

ActionAbstract::Tabs ActionSmartEvent::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers | TabHTTP);
}

QColor ActionSmartEvent::actionColor()
{
    if (m_smartEventType == PEOPLE_CNT) {
        return QColor("#FFE793");
    } else {
        return QColor(252, 174, 200);
    }
}

int ActionSmartEvent::eventType()
{
    eventInType mode;
    switch (m_smartEventType) {
    case REGIONIN:
        mode = REGION_EN;
        break;
    case REGIONOUT:
        mode = REGION_EXIT;
        break;
    case ADVANCED_MOTION:
        mode = ADVANCED_MOT;
        break;
    case TAMPER:
        mode = TAMPER_DET;
        break;
    case LINECROSS:
        mode = LINE_CROSS;
        break;
    case LOITERING:
        mode = LOITER;
        break;
    case HUMAN:
        mode = HUMAN_DET;
        break;
    case PEOPLE_CNT:
        mode = PEOPLE_COUNT;
        break;
    case OBJECT_LEFTREMOVE:
        mode = LEFTREMOVE;
        break;
    default:
        mode = MAXEVT;
        break;
    }
    return mode;
}

int ActionSmartEvent::scheduleType()
{
    return SMART_EVT_RECORD;
}

schedule_day *ActionSmartEvent::audibleWarningSchedule()
{
    return m_scheduleAudibleWarning->schedule_day;
}

schedule_day *ActionSmartEvent::emailLinkageSchedule()
{
    return m_scheduleEmailLinkage->schedule_day;
}

schedule_day *ActionSmartEvent::eventPopupSchedule()
{
    return m_scheduleEventPopup->schedule_day;
}

schedule_day *ActionSmartEvent::ptzActionSchedule()
{
    return m_schedulePtzAction->schedule_day;
}

schedule_day *ActionSmartEvent::whiteLedSchedule()
{
    return m_scheduleWhiteLed->schedule_day;
}

int *ActionSmartEvent::audibleWarningTriggerInterval()
{
    return &m_smartEvent->buzzer_interval;
}

int *ActionSmartEvent::emailLinkageTriggerInterval()
{
    return &m_smartEvent->email_interval;
}

int *ActionSmartEvent::eventPopupTriggerInterval()
{
    return &m_smartEvent->popup_interval;
}

int *ActionSmartEvent::ptzActionTriggerInterval()
{
    return &m_smartEvent->ptzaction_interval;
}

int *ActionSmartEvent::alarmOutputTriggerInterval()
{
    return &m_smartEvent->alarmout_interval;
}

int *ActionSmartEvent::whiteLedTriggerInterval()
{
    return &m_smartEvent->whiteled_interval;
}

int *ActionSmartEvent::emailLinkagePictureAttached()
{
    return &m_smartEvent->email_pic_enable;
}

int *ActionSmartEvent::audioFileNo()
{
    return &m_smartEvent->tri_audio_id;
}

ptz_action_params *ActionSmartEvent::ptzActions()
{
    return m_ptzParamsArray;
}

int *ActionSmartEvent::ptzActionsCount()
{
    return &m_ptzParamsCount;
}

uint *ActionSmartEvent::nvrTriggerAlarmOutput()
{
    return &m_smartEvent->tri_alarms;
}

char *ActionSmartEvent::ch1TriggerAlarmOutput()
{
    return m_smartEvent->tri_chnout1_alarms;
}

char *ActionSmartEvent::ch2TriggerAlarmOutput()
{
    return m_smartEvent->tri_chnout2_alarms;
}

white_led_params *ActionSmartEvent::whiteLedParams()
{
    return m_ledParamsArray;
}

int *ActionSmartEvent::whiteLedParamsCount()
{
    return &m_ledParamsCount;
}

uint *ActionSmartEvent::triggerChannelsRecord()
{
    return &m_smartEvent->tri_channels;
}

char *ActionSmartEvent::triggerChannelsRecordEx()
{
    return m_smartEvent->tri_channels_ex;
}

char *ActionSmartEvent::triggerChannelsSnapshot()
{
    return m_smartEvent->tri_channels_pic;
}

schedule_day *ActionSmartEvent::httpNotificationSchedule()
{
    return m_scheduleHttpNotification->schedule_day;
}

int *ActionSmartEvent::httpNotificationTriggerInterval()
{
    return &m_smartEvent->http_notification_interval;
}

HttpNotificationParams *ActionSmartEvent::httpNotificationParams()
{
    return m_httpNotificationParams;
}
