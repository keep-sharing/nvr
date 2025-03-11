#include "ActionAudioAlarm.h"
#include "EventLoop.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}

ActionAudioAlarm::ActionAudioAlarm(QWidget *parent)
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

ActionAudioAlarm::~ActionAudioAlarm()
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

void ActionAudioAlarm::readAction(int channel)
{
    m_channel = channel;

    m_led_info_params.chnid = m_channel;

    snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", AUD_WLED_PARAMS);
    snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", AUD_HTTP_SCHE);
    snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", AUD_HTTP_PARAMS);

    memset(m_event, 0, sizeof(smart_event));
    read_audio_alarm(SQLITE_FILE_NAME, m_event, m_channel);

    memset(m_scheduleAudibleWarning, 0, sizeof(smart_event_schedule));
    read_audio_alarm_sche(SQLITE_FILE_NAME, m_scheduleAudibleWarning, const_cast<char *>(AUD_AUDI_SCHE), m_channel);

    memset(m_scheduleEmailLinkage, 0, sizeof(smart_event_schedule));
    read_audio_alarm_sche(SQLITE_FILE_NAME, m_scheduleEmailLinkage, const_cast<char *>(AUD_EMAIL_SCHE), m_channel);

    memset(m_scheduleEventPopup, 0, sizeof(smart_event_schedule));
    read_audio_alarm_sche(SQLITE_FILE_NAME, m_scheduleEventPopup, const_cast<char *>(AUD_POP_SCHE), m_channel);

    memset(m_schedulePtzAction, 0, sizeof(smart_event_schedule));
    read_audio_alarm_sche(SQLITE_FILE_NAME, m_schedulePtzAction, const_cast<char *>(AUD_PTZ_SCHE), m_channel);

    memset(m_ptzParamsArray, 0, sizeof(ptz_action_params) * MAX_CAMERA);
    read_ptz_params(SQLITE_FILE_NAME, m_ptzParamsArray, eventType(), m_channel, &m_ptzParamsCount);

    memset(m_scheduleWhiteLed, 0, sizeof(smart_event_schedule));
    read_audio_alarm_sche(SQLITE_FILE_NAME, m_scheduleWhiteLed, const_cast<char *>(AUD_WLED_SCHE), m_channel);

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

void ActionAudioAlarm::showAction(int channel)
{
    if (!hasCache()) {
        readAction(channel);
    }
    ActionAbstract::showAction();
}

void ActionAudioAlarm::saveAction(int channel, Uint64 chnMask)
{
    if (!hasCache()) {
        readAction(channel);
    }
    write_audio_alarm(SQLITE_FILE_NAME, m_event);
    copy_audio_alarms(SQLITE_FILE_NAME, m_event, chnMask);
    copy_audio_alarm_sche(SQLITE_FILE_NAME, m_scheduleAudibleWarning, const_cast<char *>(AUD_AUDI_SCHE), chnMask);
    copy_audio_alarm_sche(SQLITE_FILE_NAME, m_scheduleEmailLinkage, const_cast<char *>(AUD_EMAIL_SCHE), chnMask);
    copy_audio_alarm_sche(SQLITE_FILE_NAME, m_scheduleEventPopup, const_cast<char *>(AUD_POP_SCHE), chnMask);
    copy_audio_alarm_sche(SQLITE_FILE_NAME, m_schedulePtzAction, const_cast<char *>(AUD_PTZ_SCHE), chnMask);
    copy_audio_alarm_sche(SQLITE_FILE_NAME, m_scheduleWhiteLed, const_cast<char *>(AUD_WLED_SCHE), chnMask);
    for (int i = 0; i < MAX_REAL_CAMERA; i++) {
        if (!(chnMask >> i & 0x01)) {
            continue;
        }
        m_http_info_schedule.chnid = i;
        m_http_info_params.chnid = i;
        write_ptz_params_all(SQLITE_FILE_NAME, m_ptzParamsArray, eventType(), i);
        write_whiteled_params_all(SQLITE_FILE_NAME, m_ledParamsArray, m_led_info_params.pDbTable, i);
        write_http_notification_schedule(SQLITE_FILE_NAME, m_scheduleHttpNotification, m_http_info_schedule.pDbTable, i);
        write_http_notification_params(SQLITE_FILE_NAME, m_httpNotificationParams, m_http_info_params.pDbTable);
    }
}

ActionAbstract::Tabs ActionAudioAlarm::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers| TabHTTP);
}

QColor ActionAudioAlarm::actionColor()
{
    return QColor("#A484FF");
}

int ActionAudioAlarm::eventType()
{
    return AUDIO_ALARM;
}

int ActionAudioAlarm::scheduleType()
{
    return AUDIO_ALARM_RECORD;
}

schedule_day *ActionAudioAlarm::audibleWarningSchedule()
{
    return m_scheduleAudibleWarning->schedule_day;
}

schedule_day *ActionAudioAlarm::emailLinkageSchedule()
{
    return m_scheduleEmailLinkage->schedule_day;
}

schedule_day *ActionAudioAlarm::eventPopupSchedule()
{
    return m_scheduleEventPopup->schedule_day;
}

schedule_day *ActionAudioAlarm::ptzActionSchedule()
{
    return m_schedulePtzAction->schedule_day;
}

schedule_day *ActionAudioAlarm::whiteLedSchedule()
{
    return m_scheduleWhiteLed->schedule_day;
}

int *ActionAudioAlarm::audibleWarningTriggerInterval()
{
    return &m_event->buzzer_interval;
}

int *ActionAudioAlarm::emailLinkageTriggerInterval()
{
    return &m_event->email_interval;
}

int *ActionAudioAlarm::eventPopupTriggerInterval()
{
    return &m_event->popup_interval;
}

int *ActionAudioAlarm::ptzActionTriggerInterval()
{
    return &m_event->ptzaction_interval;
}

int *ActionAudioAlarm::alarmOutputTriggerInterval()
{
    return &m_event->alarmout_interval;
}

int *ActionAudioAlarm::whiteLedTriggerInterval()
{
    return &m_event->whiteled_interval;
}

int *ActionAudioAlarm::emailLinkagePictureAttached()
{
    return &m_event->email_pic_enable;
}

int *ActionAudioAlarm::audioFileNo()
{
    return &m_event->tri_audio_id;
}

ptz_action_params *ActionAudioAlarm::ptzActions()
{
    return m_ptzParamsArray;
}

int *ActionAudioAlarm::ptzActionsCount()
{
    return &m_ptzParamsCount;
}

uint *ActionAudioAlarm::nvrTriggerAlarmOutput()
{
    return &m_event->tri_alarms;
}

char *ActionAudioAlarm::ch1TriggerAlarmOutput()
{
    return m_event->tri_chnout1_alarms;
}

char *ActionAudioAlarm::ch2TriggerAlarmOutput()
{
    return m_event->tri_chnout2_alarms;
}

white_led_params *ActionAudioAlarm::whiteLedParams()
{
    return m_ledParamsArray;
}

int *ActionAudioAlarm::whiteLedParamsCount()
{
    return &m_ledParamsCount;
}

uint *ActionAudioAlarm::triggerChannelsRecord()
{
    return &m_event->tri_channels;
}

char *ActionAudioAlarm::triggerChannelsRecordEx()
{
    return m_event->tri_channels_ex;
}

char *ActionAudioAlarm::triggerChannelsSnapshot()
{
    return m_event->tri_channels_pic;
}

schedule_day *ActionAudioAlarm::httpNotificationSchedule()
{
    return m_scheduleHttpNotification->schedule_day;
}

int *ActionAudioAlarm::httpNotificationTriggerInterval()
{
    return  &m_event->http_notification_interval;
}

HttpNotificationParams *ActionAudioAlarm::httpNotificationParams()
{
    return m_httpNotificationParams;
}
