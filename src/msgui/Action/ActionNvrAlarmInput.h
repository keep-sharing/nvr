#ifndef ACTIONNVRALARMINPUT_H
#define ACTIONNVRALARMINPUT_H

#include "ActionAbstract.h"

class ActionNvrAlarmInput : public ActionAbstract {
    Q_OBJECT
public:
    explicit ActionNvrAlarmInput(QWidget *parent = nullptr);
    ~ActionNvrAlarmInput() override;

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

    bool hasTriggerChannelsSnapshot() override;
    char *emailLinkageTriggerChannelsSnapshot() override;

    bool hasTriggerChannelEventPopup() override;
    int *eventPopupTriggerLayout() override;
    int *eventPopupTriggerChannels() override;

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

    alarm_in *m_alarmin = nullptr;

    alarm_in_schedule *m_scheduleAudibleWarning = nullptr;
    alarm_in_schedule *m_scheduleEmailLinkage = nullptr;
    alarm_in_schedule *m_scheduleEventPopup = nullptr;

    alarm_in_schedule *m_schedulePtzAction = nullptr;
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
#endif // ACTIONNVRALARMINPUT_H
