#ifndef ACTIONVIDEOLOSS_H
#define ACTIONVIDEOLOSS_H

#include "ActionAbstract.h"

struct video_loss;
struct smart_event_schedule;
struct video_loss_schedule;
struct white_led_params;
struct ptz_action_params;

extern "C" {
#include "msdb.h"
#include "msdefs.h"
}

class ActionVideoLoss : public ActionAbstract
{
    Q_OBJECT
public:
    explicit ActionVideoLoss(QWidget *parent = nullptr);
    ~ActionVideoLoss() override;

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

    video_loss *m_video_loss = nullptr;
    video_loss_schedule *m_scheduleAudibleWarning = nullptr;
    video_loss_schedule *m_scheduleEmailLinkage = nullptr;
    video_loss_schedule *m_scheduleEventPopup = nullptr;
    video_loss_schedule *m_schedulePtzAction = nullptr;
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

#endif // ACTIONVIDEOLOSS_H
