#include "ActionMotion.h"
#include "EventLoop.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}

ActionMotion::ActionMotion(QWidget *parent)
    : ActionAbstract(parent)
{
    initialize();

    m_motion = new motion;
    m_scheduleAudibleWarning = new motion_schedule;
    m_scheduleEmailLinkage = new motion_schedule;
    m_scheduleEventPopup = new motion_schedule;
    m_schedulePtzAction = new motion_schedule;
    m_ptzParamsArray = new ptz_action_params[MAX_CAMERA];
    m_scheduleWhiteLed = new smart_event_schedule;
    m_ledParamsArray = new white_led_params[MAX_CAMERA];
    m_scheduleHttpNotification = new smart_event_schedule;
    m_httpNotificationParams = new HttpNotificationParams;
}

ActionMotion::~ActionMotion()
{
    delete m_motion;
    m_motion = nullptr;

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

void ActionMotion::showAction(int channel)
{
    if (!hasCache()) {
        m_channel = channel;

        m_led_info_schedule.chnid = m_channel;
        m_led_info_params.chnid = m_channel;

        snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", MOT_WLED_ESCHE);
        snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", MOT_WLED_PARAMS);
        snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", MOT_HTTP_SCHE);
        snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", MOT_HTTP_PARAMS);

        memset(m_motion, 0, sizeof(motion));
        read_motion(SQLITE_FILE_NAME, m_motion, m_channel);

        memset(m_scheduleAudibleWarning, 0, sizeof(motion_schedule));
        read_motion_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel);

        memset(m_scheduleEmailLinkage, 0, sizeof(motion_schedule));
        read_motion_email_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel);

        memset(m_scheduleEventPopup, 0, sizeof(motion_schedule));
        read_motion_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel);

        memset(m_schedulePtzAction, 0, sizeof(motion_schedule));
        read_motion_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel);

        memset(m_ptzParamsArray, 0x0, sizeof(ptz_action_params) * MAX_CAMERA);
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
    ActionAbstract::showAction();
}

void ActionMotion::saveAction()
{
    if (!hasCache()) {
        return;
    }
    write_motion(SQLITE_FILE_NAME, m_motion);
    write_motion_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel);
    write_motion_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel);
    write_motion_email_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel);
    write_motion_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel);
    write_whiteled_effective_schedule(SQLITE_FILE_NAME, m_scheduleWhiteLed, &m_led_info_schedule);
    write_ptz_params_all(SQLITE_FILE_NAME, m_ptzParamsArray, eventType(), m_channel);
    write_whiteled_params_all(SQLITE_FILE_NAME, m_ledParamsArray, m_led_info_params.pDbTable, m_channel);
    write_http_notification_schedule(SQLITE_FILE_NAME, m_scheduleHttpNotification, m_http_info_schedule.pDbTable, m_channel);
    write_http_notification_params(SQLITE_FILE_NAME, m_httpNotificationParams, m_http_info_params.pDbTable);
}

ActionAbstract::Tabs ActionMotion::actionTabs()
{
     return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers| TabHTTP);
}

QColor ActionMotion::actionColor()
{
    return QColor(27, 193, 91);
}

int ActionMotion::eventType()
{
    return MOTION;
}

int ActionMotion::scheduleType()
{
    return MOTION_ACTION;
}

schedule_day *ActionMotion::audibleWarningSchedule()
{
    return m_scheduleAudibleWarning->schedule_day;
}

schedule_day *ActionMotion::emailLinkageSchedule()
{
    return m_scheduleEmailLinkage->schedule_day;
}

schedule_day *ActionMotion::eventPopupSchedule()
{
    return m_scheduleEventPopup->schedule_day;
}

schedule_day *ActionMotion::ptzActionSchedule()
{
    return m_schedulePtzAction->schedule_day;
}

schedule_day *ActionMotion::whiteLedSchedule()
{
    return m_scheduleWhiteLed->schedule_day;
}

int *ActionMotion::audibleWarningTriggerInterval()
{
    return &m_motion->buzzer_interval;
}

int *ActionMotion::emailLinkageTriggerInterval()
{
    return &m_motion->email_buzzer_interval;
}

int *ActionMotion::eventPopupTriggerInterval()
{
    return &m_motion->popup_interval;
}

int *ActionMotion::ptzActionTriggerInterval()
{
    return &m_motion->ptzaction_interval;
}

int *ActionMotion::alarmOutputTriggerInterval()
{
    return &m_motion->alarmout_interval;
}

int *ActionMotion::whiteLedTriggerInterval()
{
    return &m_motion->whiteled_interval;
}

int *ActionMotion::emailLinkagePictureAttached()
{
    return &m_motion->email_pic_enable;
}

int *ActionMotion::audioFileNo()
{
    return &m_motion->tri_audio_id;
}

ptz_action_params *ActionMotion::ptzActions()
{
    return m_ptzParamsArray;
}

int *ActionMotion::ptzActionsCount()
{
    return &m_ptzParamsCount;
}

uint *ActionMotion::nvrTriggerAlarmOutput()
{
    return &m_motion->tri_alarms;
}

char *ActionMotion::ch1TriggerAlarmOutput()
{
    return m_motion->tri_chnout1_alarms;
}

char *ActionMotion::ch2TriggerAlarmOutput()
{
    return m_motion->tri_chnout2_alarms;
}

white_led_params *ActionMotion::whiteLedParams()
{
    return m_ledParamsArray;
}

int *ActionMotion::whiteLedParamsCount()
{
    return &m_ledParamsCount;
}

uint *ActionMotion::triggerChannelsRecord()
{
    return &m_motion->tri_channels;
}

char *ActionMotion::triggerChannelsRecordEx()
{
    return m_motion->tri_channels_ex;
}

char *ActionMotion::triggerChannelsSnapshot()
{
    return m_motion->tri_channels_pic;
}

schedule_day *ActionMotion::httpNotificationSchedule()
{
    return m_scheduleHttpNotification->schedule_day;
}

int *ActionMotion::httpNotificationTriggerInterval()
{
    return  &m_motion->http_notification_interval;
}

HttpNotificationParams *ActionMotion::httpNotificationParams()
{
    return m_httpNotificationParams;
}
