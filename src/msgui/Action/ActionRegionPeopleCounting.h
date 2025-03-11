#ifndef ACTIONREGIONPEOPLECOUNTING_H
#define ACTIONREGIONPEOPLECOUNTING_H

#include "ActionAbstract.h"

class ActionRegionPeopleCounting : public ActionAbstract
{
    Q_OBJECT

    struct Data {
        smart_event event;

        smart_event_schedule scheduleAudibleWarning;
        smart_event_schedule scheduleEmailLinkage;
        smart_event_schedule scheduleEventPopup;

        smart_event_schedule schedulePtzAction;
        ptz_action_params ptzParamsArray[MAX_CAMERA];
        int ptzParamsCount = 0;

        WLED_INFO led_info_params;
        smart_event_schedule scheduleWhiteLed;
        white_led_params ledParamsArray[MAX_CAMERA];
        int ledParamsCount = 0;

        smart_event_schedule scheduleHttpNotification;
        HttpNotificationParams httpNotificationParams;
        WLED_INFO http_info_schedule;
        WLED_INFO http_info_params;
    };

public:
    explicit ActionRegionPeopleCounting(QWidget *parent = nullptr);
    ~ActionRegionPeopleCounting() override;

    void showAction(int channel, int region);
    bool saveAction();
    bool copyAction(int channel, quint64 copyFlags);

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

    schedule_day *httpNotificationSchedule() override;
    int *httpNotificationTriggerInterval() override;
    HttpNotificationParams *httpNotificationParams() override;
signals:

private:
    int m_region = 0;
    QMap<int, Data> m_dataMap;
};

#endif // ACTIONREGIONPEOPLECOUNTING_H
