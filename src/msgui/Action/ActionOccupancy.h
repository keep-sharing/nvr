#ifndef ACTIONOCCUPANCY_H
#define ACTIONOCCUPANCY_H

#include "ActionAbstract.h"

class ActionOccupancy : public ActionAbstract
{
    Q_OBJECT

    struct Data {
        peoplecnt_event event;

        smart_event_schedule scheduleAudibleWarning;
        smart_event_schedule scheduleEmailLinkage;
        smart_event_schedule schedulePtzAction;
        smart_event_schedule scheduleWhiteLed;

        ptz_action_params ptzParamsArray[MAX_CAMERA];
        int ptzParamsCount = 0;

        white_led_params ledParamsArray[MAX_CAMERA];
        int ledParamsCount = 0;

        smart_event_schedule scheduleHttpNotification;
        HttpNotificationParams httpNotificationParams;
        WLED_INFO http_info_schedule;
        WLED_INFO http_info_params;
    };

public:
    explicit ActionOccupancy(QWidget *parent = nullptr);
    ~ActionOccupancy() override;

    void showAction(int group);
    void saveAction();

    bool hasCache() const override;
    void clearCache() override;

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
    bool isNoteVisible() const override;

    schedule_day *httpNotificationSchedule() override;
    int *httpNotificationTriggerInterval() override;
    HttpNotificationParams *httpNotificationParams() override;

private:
    int m_group = -1;
    QMap<int, Data> m_dataMap;
};

#endif // ACTIONOCCUPANCY_H
