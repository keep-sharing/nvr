#ifndef ACTIONMOTION_H
#define ACTIONMOTION_H

#include "ActionAbstract.h"

class ActionMotion : public ActionAbstract
{
    Q_OBJECT
public:
    explicit ActionMotion(QWidget *parent = nullptr);
    ~ActionMotion() override;

    void showAction(int channel);
    void saveAction();

protected:
    Tabs actionTabs() override;
    QColor actionColor() override;
    int eventType() override;
    int scheduleType() override;

    schedule_day *audibleWarningSchedule() override;
    schedule_day *emailLinkageSchedule() override;
    schedule_day *eventPopupSchedule() override;
    schedule_day *ptzActionSchedule() override;
    schedule_day *whiteLedSchedule() override;

    int *audibleWarningTriggerInterval() override;
    int *emailLinkageTriggerInterval() override;
    int *eventPopupTriggerInterval() override;
    int *ptzActionTriggerInterval() override;
    int *alarmOutputTriggerInterval() override;
    int *whiteLedTriggerInterval() override;

    int *emailLinkagePictureAttached() override;

    int *audioFileNo() override;

    ptz_action_params *ptzActions() override;
    int *ptzActionsCount() override;

    uint *nvrTriggerAlarmOutput() override;
    char *ch1TriggerAlarmOutput() override;
    char *ch2TriggerAlarmOutput() override;

    white_led_params *whiteLedParams() override;
    int *whiteLedParamsCount() override;

    uint *triggerChannelsRecord() override;
    char *triggerChannelsRecordEx() override;
    char *triggerChannelsSnapshot() override;

    schedule_day *httpNotificationSchedule() override;
    int *httpNotificationTriggerInterval() override;
    HttpNotificationParams *httpNotificationParams() override;

private:

signals:

private:
    WLED_INFO m_led_info_schedule;
    WLED_INFO m_led_info_params;

    motion *m_motion = nullptr;
    motion_schedule *m_scheduleAudibleWarning = nullptr;
    motion_schedule *m_scheduleEmailLinkage = nullptr;
    motion_schedule *m_scheduleEventPopup = nullptr;
    motion_schedule *m_schedulePtzAction = nullptr;
    ptz_action_params *m_ptzParamsArray = nullptr;
    int m_ptzParamsCount = 0;

    smart_event_schedule *m_scheduleWhiteLed = nullptr;
    white_led_params *m_ledParamsArray = nullptr;
    int m_ledParamsCount = 0;

    smart_event_schedule *m_scheduleHttpNotification = nullptr;
    HttpNotificationParams *m_httpNotificationParams = nullptr;
    WLED_INFO m_http_info_schedule;
    WLED_INFO m_http_info_params;
};

#endif // ACTIONMOTION_H
