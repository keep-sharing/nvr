#include "ActionFace.h"
#include "EventLoop.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}
ActionFace::ActionFace(QWidget *parent)
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

ActionFace::~ActionFace()
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

void ActionFace::showAction(int channel)
{
    if (!hasCache()) {
        m_channel = channel;

        m_led_info_params.chnid = m_channel;

        snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", FACE_WLED_PARAMS);
        snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", FACE_HTTP_SCHE);
        snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", FACE_HTTP_PARAMS);

        memset(m_event, 0, sizeof(smart_event));
        read_face_event(SQLITE_FILE_NAME, m_event, m_channel);

        memset(m_scheduleAudibleWarning, 0, sizeof(smart_event_schedule));
        read_face_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel);

        memset(m_scheduleEmailLinkage, 0, sizeof(smart_event_schedule));
        read_face_mail_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel);

        memset(m_scheduleEventPopup, 0, sizeof(smart_event_schedule));
        read_face_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel);

        memset(m_schedulePtzAction, 0, sizeof(smart_event_schedule));
        read_face_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel);

        memset(m_ptzParamsArray, 0, sizeof(ptz_action_params) * MAX_CAMERA);

        read_ptz_params(SQLITE_FILE_NAME, m_ptzParamsArray, eventType(), m_channel, &m_ptzParamsCount);

        memset(m_scheduleWhiteLed, 0, sizeof(smart_event_schedule));
        read_face_whiteled_schedule(SQLITE_FILE_NAME, m_scheduleWhiteLed, m_channel);

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
    ActionAbstract::showAction();
}

void ActionFace::saveAction()
{
    if (!hasCache()) {
        return;
    }
    write_face_event(SQLITE_FILE_NAME, m_event);
    write_face_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel);
    write_face_mail_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel);
    write_face_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel);
    write_face_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel);
    write_ptz_params_all(SQLITE_FILE_NAME, m_ptzParamsArray, eventType(), m_channel);
    write_face_whiteled_schedule(SQLITE_FILE_NAME, m_scheduleWhiteLed, m_channel);
    write_whiteled_params_all(SQLITE_FILE_NAME, m_ledParamsArray, m_led_info_params.pDbTable, m_channel);
    write_http_notification_schedule(SQLITE_FILE_NAME, m_scheduleHttpNotification, m_http_info_schedule.pDbTable, m_channel);
    write_http_notification_params(SQLITE_FILE_NAME, m_httpNotificationParams, m_http_info_params.pDbTable);
}

ActionAbstract::Tabs ActionFace::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers| TabHTTP);
}

QColor ActionFace::actionColor()
{
    return QColor(255, 231, 147);
}

int ActionFace::eventType()
{
    return FACE_EVT;
}

int ActionFace::scheduleType()
{
    return FACE_RECORD;
}

schedule_day *ActionFace::audibleWarningSchedule()
{
    return m_scheduleAudibleWarning->schedule_day;
}

schedule_day *ActionFace::emailLinkageSchedule()
{
    return m_scheduleEmailLinkage->schedule_day;
}

schedule_day *ActionFace::eventPopupSchedule()
{
    return m_scheduleEventPopup->schedule_day;
}

schedule_day *ActionFace::ptzActionSchedule()
{
    return m_schedulePtzAction->schedule_day;
}

schedule_day *ActionFace::whiteLedSchedule()
{
    return m_scheduleWhiteLed->schedule_day;
}

int *ActionFace::audibleWarningTriggerInterval()
{
    return &m_event->buzzer_interval;
}

int *ActionFace::emailLinkageTriggerInterval()
{
    return &m_event->email_interval;
}

int *ActionFace::eventPopupTriggerInterval()
{
    return &m_event->popup_interval;
}

int *ActionFace::ptzActionTriggerInterval()
{
    return &m_event->ptzaction_interval;
}

int *ActionFace::alarmOutputTriggerInterval()
{
    return &m_event->alarmout_interval;
}

int *ActionFace::whiteLedTriggerInterval()
{
    return &m_event->whiteled_interval;
}

int *ActionFace::emailLinkagePictureAttached()
{
    return &m_event->email_pic_enable;
}

int *ActionFace::audioFileNo()
{
    return &m_event->tri_audio_id;
}

ptz_action_params *ActionFace::ptzActions()
{
    return m_ptzParamsArray;
}

int *ActionFace::ptzActionsCount()
{
    return &m_ptzParamsCount;
}

uint *ActionFace::nvrTriggerAlarmOutput()
{
    return &m_event->tri_alarms;
}

char *ActionFace::ch1TriggerAlarmOutput()
{
    return m_event->tri_chnout1_alarms;
}

char *ActionFace::ch2TriggerAlarmOutput()
{
    return m_event->tri_chnout2_alarms;
}

white_led_params *ActionFace::whiteLedParams()
{
    return m_ledParamsArray;
}

int *ActionFace::whiteLedParamsCount()
{
    return &m_ledParamsCount;
}

uint *ActionFace::triggerChannelsRecord()
{
    return &m_event->tri_channels;
}

char *ActionFace::triggerChannelsRecordEx()
{
    return m_event->tri_channels_ex;
}

char *ActionFace::triggerChannelsSnapshot()
{
    return m_event->tri_channels_pic;
}

schedule_day *ActionFace::httpNotificationSchedule()
{
    return m_scheduleHttpNotification->schedule_day;
}

int *ActionFace::httpNotificationTriggerInterval()
{
    return  &m_event->http_notification_interval;
}

HttpNotificationParams *ActionFace::httpNotificationParams()
{
    return m_httpNotificationParams;
}
