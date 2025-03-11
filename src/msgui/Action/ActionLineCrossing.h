#ifndef ACTIONLINECROSSING_H
#define ACTIONLINECROSSING_H

#include <QObject>
#include "ActionAbstract.h"

class ActionLineCrossing : public ActionAbstract
{
    Q_OBJECT

    struct Data {
        smart_event event;

        smart_event_schedule scheduleAudibleWarning;
        smart_event_schedule scheduleEmailLinkage;
        smart_event_schedule scheduleEventPopup;
        smart_event_schedule schedulePtzAction;
        smart_event_schedule scheduleWhiteLed;

        ptz_action_params ptzParamsArray[MAX_CAMERA];
        int ptzParamsCount = 0;

        WLED_INFO led_info_schedule;
        WLED_INFO led_info_params;
        white_led_params ledParamsArray[MAX_CAMERA];
        int ledParamsCount = 0;

        smart_event_schedule scheduleHttpNotification;
        HttpNotificationParams httpNotificationParams;
        WLED_INFO http_info_schedule;
        WLED_INFO http_info_params;
    };

public:
    explicit ActionLineCrossing(QWidget *parent = nullptr);

    void showAction(int channel, SMART_EVENT_TYPE mode);
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

    schedule_day *httpNotificationSchedule() override;
    int *httpNotificationTriggerInterval() override;
    HttpNotificationParams *httpNotificationParams() override;

signals:

private:
    SMART_EVENT_TYPE m_eventType;
    QMap<int, Data> m_dataMap;
};

#endif // ACTIONLINECROSSING_H
