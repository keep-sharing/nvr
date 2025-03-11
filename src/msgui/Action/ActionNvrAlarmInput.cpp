#include "ActionNvrAlarmInput.h"
#include "EventLoop.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}

ActionNvrAlarmInput::ActionNvrAlarmInput(QWidget *parent)
    : ActionAbstract(parent)
{
    initialize();

    m_alarmin = new alarm_in;
    m_scheduleAudibleWarning = new alarm_in_schedule;
    m_scheduleEmailLinkage = new alarm_in_schedule;
    m_scheduleEventPopup = new alarm_in_schedule;
    m_schedulePtzAction = new alarm_in_schedule;
    m_ptzParamsArray = new ptz_action_params[MAX_CAMERA];
    m_scheduleWhiteLed = new smart_event_schedule;
    m_ledParamsArray = new white_led_params[MAX_CAMERA];
    m_scheduleHttpNotification = new smart_event_schedule;
    m_httpNotificationParams = new HttpNotificationParams;
}

ActionNvrAlarmInput::~ActionNvrAlarmInput()
{
    delete m_alarmin;
    m_alarmin = nullptr;

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

void ActionNvrAlarmInput::showAction(int channel)
{
    if (hasCache()) {

    } else {
        m_channel = channel;

        m_led_info_schedule.chnid = m_channel;
        m_led_info_params.chnid = m_channel;

        snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", AIN_WLED_ESCHE);
        snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", AIN_WLED_PARAMS);
        snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", AIN_HTTP_SCHE);
        snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", AIN_HTTP_PARAMS);

        memset(m_alarmin, 0, sizeof(alarm_in));
        read_alarm_in(SQLITE_FILE_NAME, m_alarmin, m_channel);

        memset(m_scheduleAudibleWarning, 0, sizeof( alarm_in_schedule));
        read_alarmin_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel);

        memset(m_scheduleEmailLinkage, 0, sizeof( alarm_in_schedule));
        read_alarmin_email_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel);

        memset(m_scheduleEventPopup, 0, sizeof( alarm_in_schedule));
        read_alarmin_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel);

        memset(m_schedulePtzAction, 0, sizeof( alarm_in_schedule));
        read_alarmin_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel);

        memset(m_ptzParamsArray, 0, sizeof(ptz_action_params) * MAX_CAMERA);
        read_ptz_params(SQLITE_FILE_NAME, m_ptzParamsArray, ALARMIO, m_channel, &m_ptzParamsCount);

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

void ActionNvrAlarmInput::saveAction()
{
    if (!hasCache()) {
        return;
    }
    write_alarm_in(SQLITE_FILE_NAME, m_alarmin);
    write_alarmin_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel);
    write_alarmin_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel);
    write_alarmin_email_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel);
    write_alarmin_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel);
    write_ptz_params_all(SQLITE_FILE_NAME, m_ptzParamsArray, ALARMIO, m_channel);
    write_whiteled_effective_schedule(SQLITE_FILE_NAME, m_scheduleWhiteLed, &m_led_info_schedule);
    write_whiteled_params_all(SQLITE_FILE_NAME, m_ledParamsArray, m_led_info_params.pDbTable, m_channel);
    write_http_notification_schedule(SQLITE_FILE_NAME, m_scheduleHttpNotification, m_http_info_schedule.pDbTable, m_channel);
    write_http_notification_params(SQLITE_FILE_NAME, m_httpNotificationParams, m_http_info_params.pDbTable);
}

ActionAbstract::Tabs ActionNvrAlarmInput::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers| TabHTTP);
}

QColor ActionNvrAlarmInput::actionColor()
{
    return QColor(QString("#FF3600"));
}

int ActionNvrAlarmInput::eventType()
{

    return ALARMIO;
}

int ActionNvrAlarmInput::scheduleType()
{
    return ALARMIN_ACTION;
}

schedule_day *ActionNvrAlarmInput::audibleWarningSchedule()
{
    return m_scheduleAudibleWarning->schedule_day;
}

schedule_day *ActionNvrAlarmInput::emailLinkageSchedule()
{
    return m_scheduleEmailLinkage->schedule_day;
}

schedule_day *ActionNvrAlarmInput::eventPopupSchedule()
{
    return m_scheduleEventPopup->schedule_day;
}

schedule_day *ActionNvrAlarmInput::ptzActionSchedule()
{
    return m_schedulePtzAction->schedule_day;
}

schedule_day *ActionNvrAlarmInput::whiteLedSchedule()
{
    return m_scheduleWhiteLed->schedule_day;
}

int *ActionNvrAlarmInput::audibleWarningTriggerInterval()
{
    return &m_alarmin->buzzer_interval;
}

int *ActionNvrAlarmInput::emailLinkageTriggerInterval()
{
    return &m_alarmin->email_buzzer_interval;
}

int *ActionNvrAlarmInput::eventPopupTriggerInterval()
{
    return &m_alarmin->popup_interval;
}

int *ActionNvrAlarmInput::ptzActionTriggerInterval()
{
    return &m_alarmin->ptzaction_interval;
}

int *ActionNvrAlarmInput::alarmOutputTriggerInterval()
{
    return &m_alarmin->alarmout_interval;
}

int *ActionNvrAlarmInput::whiteLedTriggerInterval()
{
    return &m_alarmin->whiteled_interval;
}

int *ActionNvrAlarmInput::emailLinkagePictureAttached()
{
    return &m_alarmin->email_pic_enable;
}

bool ActionNvrAlarmInput::hasTriggerChannelsSnapshot()
{
    return true;
}

char *ActionNvrAlarmInput::emailLinkageTriggerChannelsSnapshot()
{
    return m_alarmin->tri_channels_snapshot;
}

bool ActionNvrAlarmInput::hasTriggerChannelEventPopup()
{
    return true;
}

int *ActionNvrAlarmInput::eventPopupTriggerLayout()
{
    return &m_alarmin->event_popup_layout;
}

int *ActionNvrAlarmInput::eventPopupTriggerChannels()
{
    return m_alarmin->event_popup_channel;
}

int *ActionNvrAlarmInput::audioFileNo()
{
    return &m_alarmin->tri_audio_id;
}

ptz_action_params *ActionNvrAlarmInput::ptzActions()
{
    return m_ptzParamsArray;
}

int *ActionNvrAlarmInput::ptzActionsCount()
{
    return &m_ptzParamsCount;
}

uint *ActionNvrAlarmInput::nvrTriggerAlarmOutput()
{
    return &m_alarmin->tri_alarms;
}

char *ActionNvrAlarmInput::ch1TriggerAlarmOutput()
{
    return m_alarmin->tri_chnout1_alarms;
}

char *ActionNvrAlarmInput::ch2TriggerAlarmOutput()
{
    return m_alarmin->tri_chnout2_alarms;
}

white_led_params *ActionNvrAlarmInput::whiteLedParams()
{
    return m_ledParamsArray;
}

int *ActionNvrAlarmInput::whiteLedParamsCount()
{
    return &m_ledParamsCount;
}

uint *ActionNvrAlarmInput::triggerChannelsRecord()
{
    return &m_alarmin->tri_channels;
}

char *ActionNvrAlarmInput::triggerChannelsRecordEx()
{
    return m_alarmin->tri_channels_ex;
}

char *ActionNvrAlarmInput::triggerChannelsSnapshot()
{
    return m_alarmin->tri_channels_pic;
}

schedule_day *ActionNvrAlarmInput::httpNotificationSchedule()
{
    return m_scheduleHttpNotification->schedule_day;
}

int *ActionNvrAlarmInput::httpNotificationTriggerInterval()
{
    return  &m_alarmin->http_notification_interval;
}

HttpNotificationParams *ActionNvrAlarmInput::httpNotificationParams()
{
    return m_httpNotificationParams;
}


