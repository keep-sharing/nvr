#include "ActionVideoLoss.h"
#include "EventLoop.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}

ActionVideoLoss::ActionVideoLoss(QWidget *parent)
    : ActionAbstract(parent)
{
    initialize();

    m_video_loss = new video_loss;
    m_scheduleAudibleWarning = new video_loss_schedule;
    m_scheduleEmailLinkage = new video_loss_schedule;
    m_scheduleEventPopup = new video_loss_schedule;
    m_schedulePtzAction = new video_loss_schedule;
    m_ptzParamsArray = new ptz_action_params[MAX_CAMERA];
    m_scheduleWhiteLed = new smart_event_schedule;
    m_ledParamsArray = new white_led_params[MAX_CAMERA];
    m_scheduleHttpNotification = new smart_event_schedule;
    m_httpNotificationParams = new HttpNotificationParams;
}

ActionVideoLoss::~ActionVideoLoss()
{
    delete m_video_loss;
    m_video_loss = nullptr;

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

void ActionVideoLoss::showAction(int channel)
{
    if (!hasCache()) {
        m_channel = channel;

        m_led_info_schedule.chnid = m_channel;
        m_led_info_params.chnid = m_channel;

        snprintf(m_led_info_schedule.pDbTable, sizeof(m_led_info_schedule.pDbTable), "%s", VDL_WLED_ESCHE);
        snprintf(m_led_info_params.pDbTable, sizeof(m_led_info_params.pDbTable), "%s", VDL_WLED_PARAMS);
        snprintf(m_http_info_schedule.pDbTable, sizeof(m_http_info_schedule.pDbTable), "%s", VDL_HTTP_SCHE);
        snprintf(m_http_info_params.pDbTable, sizeof(m_http_info_params.pDbTable), "%s", VDL_HTTP_PARAMS);

        memset(m_video_loss, 0, sizeof(video_loss));
        read_video_lost(SQLITE_FILE_NAME, m_video_loss, m_channel);

        memset(m_scheduleAudibleWarning, 0, sizeof(video_loss_schedule));
        read_videoloss_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel);

        memset(m_scheduleEmailLinkage, 0, sizeof(video_loss_schedule));
        read_videoloss_email_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel);

        memset(m_scheduleEventPopup, 0, sizeof(video_loss_schedule));
        read_videoloss_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel);

        memset(m_schedulePtzAction, 0, sizeof(video_loss_schedule));
        read_videoloss_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel);

        memset(m_ptzParamsArray, 0x0, sizeof(ptz_action_params) * MAX_CAMERA);
        read_ptz_params(SQLITE_FILE_NAME, m_ptzParamsArray, VIDEOLOSS, m_channel, &m_ptzParamsCount);

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

void ActionVideoLoss::saveAction()
{
    if (!hasCache()) {
        return;
    }
    write_video_lost(SQLITE_FILE_NAME, m_video_loss);
    write_videoloss_ptz_schedule(SQLITE_FILE_NAME, m_schedulePtzAction, m_channel);
    write_videoloss_audible_schedule(SQLITE_FILE_NAME, m_scheduleAudibleWarning, m_channel);
    write_videoloss_email_schedule(SQLITE_FILE_NAME, m_scheduleEmailLinkage, m_channel);
    write_videoloss_popup_schedule(SQLITE_FILE_NAME, m_scheduleEventPopup, m_channel);
    write_ptz_params_all(SQLITE_FILE_NAME, m_ptzParamsArray, VIDEOLOSS, m_channel);
    write_whiteled_effective_schedule(SQLITE_FILE_NAME, m_scheduleWhiteLed, &m_led_info_schedule);
    write_whiteled_params_all(SQLITE_FILE_NAME, m_ledParamsArray, m_led_info_params.pDbTable, m_channel);
    write_http_notification_schedule(SQLITE_FILE_NAME, m_scheduleHttpNotification, m_http_info_schedule.pDbTable, m_channel);
    write_http_notification_params(SQLITE_FILE_NAME, m_httpNotificationParams, m_http_info_params.pDbTable);
}

ActionAbstract::Tabs ActionVideoLoss::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed| TabHTTP);
}

QColor ActionVideoLoss::actionColor()
{
    return QColor(QString("#B8D7F4"));
}

int ActionVideoLoss::eventType()
{
    return VIDEOLOSS;
}

int ActionVideoLoss::scheduleType()
{
    return VIDEOLOSS_ACTION;
}

schedule_day *ActionVideoLoss::audibleWarningSchedule()
{
    return m_scheduleAudibleWarning->schedule_day;
}

schedule_day *ActionVideoLoss::emailLinkageSchedule()
{
    return m_scheduleEmailLinkage->schedule_day;
}

schedule_day *ActionVideoLoss::eventPopupSchedule()
{
    return m_scheduleEventPopup->schedule_day;
}

schedule_day *ActionVideoLoss::ptzActionSchedule()
{
    return m_schedulePtzAction->schedule_day;
}

schedule_day *ActionVideoLoss::whiteLedSchedule()
{
    return m_scheduleWhiteLed->schedule_day;
}

int *ActionVideoLoss::audibleWarningTriggerInterval()
{
    return &m_video_loss->buzzer_interval;
}

int *ActionVideoLoss::emailLinkageTriggerInterval()
{
    return &m_video_loss->email_buzzer_interval;
}

int *ActionVideoLoss::eventPopupTriggerInterval()
{
    return &m_video_loss->popup_interval;
}

int *ActionVideoLoss::ptzActionTriggerInterval()
{
    return &m_video_loss->ptzaction_interval;
}

int *ActionVideoLoss::alarmOutputTriggerInterval()
{
    return &m_video_loss->alarmout_interval;
}

int *ActionVideoLoss::whiteLedTriggerInterval()
{
    return &m_video_loss->whiteled_interval;
}

int *ActionVideoLoss::emailLinkagePictureAttached()
{
    return nullptr;
}

int *ActionVideoLoss::audioFileNo()
{
    return &m_video_loss->tri_audio_id;
}

ptz_action_params *ActionVideoLoss::ptzActions()
{
    return m_ptzParamsArray;
}

int *ActionVideoLoss::ptzActionsCount()
{
    return &m_ptzParamsCount;
}

uint *ActionVideoLoss::nvrTriggerAlarmOutput()
{
    return &m_video_loss->tri_alarms;
}

char *ActionVideoLoss::ch1TriggerAlarmOutput()
{
    return m_video_loss->tri_chnout1_alarms;
}

char *ActionVideoLoss::ch2TriggerAlarmOutput()
{
    return m_video_loss->tri_chnout2_alarms;
}

white_led_params *ActionVideoLoss::whiteLedParams()
{
    return m_ledParamsArray;
}

int *ActionVideoLoss::whiteLedParamsCount()
{
    return &m_ledParamsCount;
}

uint *ActionVideoLoss::triggerChannelsRecord()
{
    return nullptr;
}

char *ActionVideoLoss::triggerChannelsRecordEx()
{
    return nullptr;
}

char *ActionVideoLoss::triggerChannelsSnapshot()
{
    return nullptr;
}

schedule_day *ActionVideoLoss::httpNotificationSchedule()
{
    return m_scheduleHttpNotification->schedule_day;
}

int *ActionVideoLoss::httpNotificationTriggerInterval()
{
    return  &m_video_loss->http_notification_interval;
}

HttpNotificationParams *ActionVideoLoss::httpNotificationParams()
{
    return m_httpNotificationParams;
}
