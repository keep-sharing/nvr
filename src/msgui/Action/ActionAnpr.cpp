#include "ActionAnpr.h"
#include "EventLoop.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}

ActionAnpr::ActionAnpr(QWidget *parent)
    : ActionAbstract(parent)
{
    initialize();

    m_event = new smart_event;
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

ActionAnpr::~ActionAnpr()
{
    delete m_event;
    m_event = nullptr;

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

void ActionAnpr::showAction(int channel, ANPR_MODE_TYPE mode)
{
    if (hasCache()) {

    } else {
        m_channel = channel;
        m_mode = mode;

        m_led_info_schedule.chnid = m_channel;
        m_led_info_params.chnid = m_channel;
        switch (m_mode) {
        case ANPR_BLACK:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", LPRB_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", LPRB_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", LPRB_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", LPRB_HTTP_PARAMS);
            break;
        case ANPR_WHITE:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", LPRW_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", LPRW_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", LPRW_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", LPRW_HTTP_PARAMS);
            break;
        case ANPR_VISTOR:
            snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", LPRV_WLED_ESCHE);
            snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", LPRV_WLED_PARAMS);
            snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", LPRV_HTTP_SCHE);
            snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", LPRV_HTTP_PARAMS);
            break;
        default:
            break;
        }

        memset(m_event, 0, sizeof(smart_event));
        read_anpr_event(SQLITE_FILE_NAME, m_event, m_channel, m_mode);

        memset(m_scheduleAudibleWarning, 0, sizeof(smart_event_schedule));
        read_anpr_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel, m_mode);

        memset(m_scheduleEmailLinkage, 0, sizeof(smart_event_schedule));
        read_anpr_mail_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel, m_mode);

        memset(m_scheduleEventPopup, 0, sizeof(smart_event_schedule));
        read_anpr_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel, m_mode);

        memset(m_schedulePtzAction, 0, sizeof(smart_event_schedule));
        read_anpr_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel, m_mode);

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

void ActionAnpr::saveAction()
{
    if (!hasCache()) {
        return;
    }
    write_anpr_event(SQLITE_FILE_NAME, m_event, m_mode);
    write_anpr_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel, m_mode);
    write_anpr_mail_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel, m_mode);
    write_anpr_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel, m_mode);
    write_anpr_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel, m_mode);
    write_ptz_params_all(SQLITE_FILE_NAME, m_ptzParamsArray, eventType(), m_channel);
    write_whiteled_effective_schedule(SQLITE_FILE_NAME, m_scheduleWhiteLed, &m_led_info_schedule);
    write_whiteled_params_all(SQLITE_FILE_NAME, m_ledParamsArray, m_led_info_params.pDbTable, m_channel);
    write_http_notification_schedule(SQLITE_FILE_NAME, m_scheduleHttpNotification, m_http_info_schedule.pDbTable, m_channel);
    write_http_notification_params(SQLITE_FILE_NAME, m_httpNotificationParams, m_http_info_params.pDbTable);
}

ActionAbstract::Tabs ActionAnpr::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers | TabHTTP);
}

QColor ActionAnpr::actionColor()
{
    return QColor(255, 231, 147);
}

int ActionAnpr::eventType()
{
    int type = 0;
    switch (m_mode) {
    case ANPR_BLACK:
        type = ANPR_BLACK_EVT;
        break;
    case ANPR_WHITE:
        type = ANPR_WHITE_EVT;
        break;
    case ANPR_VISTOR:
        type = ANPR_VISTOR_EVT;
        break;
    default:
        qMsWarning() << "invalid mode:" << m_mode;
        break;
    }
    return type;
}

int ActionAnpr::scheduleType()
{
    return ANPT_EVT_RECORD;
}

schedule_day *ActionAnpr::audibleWarningSchedule()
{
    return m_scheduleAudibleWarning->schedule_day;
}

schedule_day *ActionAnpr::emailLinkageSchedule()
{
    return m_scheduleEmailLinkage->schedule_day;
}

schedule_day *ActionAnpr::eventPopupSchedule()
{
    return m_scheduleEventPopup->schedule_day;
}

schedule_day *ActionAnpr::ptzActionSchedule()
{
    return m_schedulePtzAction->schedule_day;
}

schedule_day *ActionAnpr::whiteLedSchedule()
{
    return m_scheduleWhiteLed->schedule_day;
}

schedule_day *ActionAnpr::httpNotificationSchedule()
{
    return m_scheduleHttpNotification->schedule_day;
}

int *ActionAnpr::audibleWarningTriggerInterval()
{
    return &m_event->buzzer_interval;
}

int *ActionAnpr::emailLinkageTriggerInterval()
{
    return &m_event->email_interval;
}

int *ActionAnpr::eventPopupTriggerInterval()
{
    return &m_event->popup_interval;
}

int *ActionAnpr::ptzActionTriggerInterval()
{
    return &m_event->ptzaction_interval;
}

int *ActionAnpr::alarmOutputTriggerInterval()
{
    return &m_event->alarmout_interval;
}

int *ActionAnpr::whiteLedTriggerInterval()
{
    return &m_event->whiteled_interval;
}

int *ActionAnpr::httpNotificationTriggerInterval()
{
    return &m_event->http_notification_interval;
}

int *ActionAnpr::emailLinkagePictureAttached()
{
    return &m_event->email_pic_enable;
}

int *ActionAnpr::audioFileNo()
{
    return &m_event->tri_audio_id;
}

ptz_action_params *ActionAnpr::ptzActions()
{
    return m_ptzParamsArray;
}

int *ActionAnpr::ptzActionsCount()
{
    return &m_ptzParamsCount;
}

uint *ActionAnpr::nvrTriggerAlarmOutput()
{
    return &m_event->tri_alarms;
}

char *ActionAnpr::ch1TriggerAlarmOutput()
{
    return m_event->tri_chnout1_alarms;
}

char *ActionAnpr::ch2TriggerAlarmOutput()
{
    return m_event->tri_chnout2_alarms;
}

white_led_params *ActionAnpr::whiteLedParams()
{
    return m_ledParamsArray;
}

int *ActionAnpr::whiteLedParamsCount()
{
    return &m_ledParamsCount;
}

uint *ActionAnpr::triggerChannelsRecord()
{
    return &m_event->tri_channels;
}

char *ActionAnpr::triggerChannelsRecordEx()
{
    return m_event->tri_channels_ex;
}

char *ActionAnpr::triggerChannelsSnapshot()
{
    return m_event->tri_channels_pic;
}

HttpNotificationParams *ActionAnpr::httpNotificationParams()
{
    return m_httpNotificationParams;
}
