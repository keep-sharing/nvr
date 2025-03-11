#ifndef ACTIONABSTRACT_H
#define ACTIONABSTRACT_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
#include "msdefs.h"
}

class ActionPageAbstract;
class MessageReceive;
class MyButtonGroup;

namespace Ui {
class ActionAbstract;
}

class ActionAbstract : public BaseShadowDialog {
    Q_OBJECT

protected:
    enum Tab {
        TabNone = 0,
        TabAudibleWarning = 0x0001,
        TabEmailLinkage = 0x0002,
        TabEventPopup = 0x0004,
        TabPtzAction = 0x0008,
        TabAlarmOutput = 0x0010,
        TabWhiteLed = 0x0020,
        TabOthers = 0x0040,
        TabHTTP = 0x0080,
    };
    Q_DECLARE_FLAGS(Tabs, Tab)

public:
    explicit ActionAbstract(QWidget *parent = nullptr);
    ~ActionAbstract();

    void showAction();

    virtual bool hasCache() const;
    virtual void setCached();
    virtual void clearCache();

    void processMessage(MessageReceive *message) override;

protected:
    void initialize();
    int channel() const;
    void clearPageCache();

    virtual Tabs actionTabs();
    virtual QColor actionColor() = 0;
    virtual int eventType() = 0;
    virtual int scheduleType() = 0;

    //Schedule
    virtual schedule_day *audibleWarningSchedule() = 0;
    virtual schedule_day *emailLinkageSchedule() = 0;
    virtual schedule_day *eventPopupSchedule() = 0;
    virtual schedule_day *ptzActionSchedule() = 0;
    virtual schedule_day *whiteLedSchedule() = 0;
    virtual schedule_day *httpNotificationSchedule() = 0;

    //Trigger Interval
    virtual int *audibleWarningTriggerInterval() = 0;
    virtual int *emailLinkageTriggerInterval() = 0;
    virtual int *eventPopupTriggerInterval() = 0;
    virtual int *ptzActionTriggerInterval() = 0;
    virtual int *alarmOutputTriggerInterval() = 0;
    virtual int *whiteLedTriggerInterval() = 0;
    virtual int *httpNotificationTriggerInterval() = 0;

    //Picture Attached
    virtual int *emailLinkagePictureAttached() = 0;

    //Trigger Channels Snapshot
    virtual bool hasTriggerChannelsSnapshot();
    virtual char *emailLinkageTriggerChannelsSnapshot();
    virtual int emailLinkageTriggerChannelsSnapshotArraySize();

    //Trigger Channel Event Popup
    virtual bool hasTriggerChannelEventPopup();
    virtual int *eventPopupTriggerLayout();
    virtual int *eventPopupTriggerChannels();

    //audible warning
    virtual int *audioFileNo() = 0;

    //PTZ Action
    virtual ptz_action_params *ptzActions() = 0;
    virtual int *ptzActionsCount() = 0;
    virtual int ptzActionsArraySize();

    //Alarm Output
    virtual uint *nvrTriggerAlarmOutput() = 0;
    virtual char *ch1TriggerAlarmOutput() = 0;
    virtual char *ch2TriggerAlarmOutput() = 0;

    //White LED
    virtual white_led_params *whiteLedParams() = 0;
    virtual int *whiteLedParamsCount() = 0;
    virtual int whiteLedParamsArraySize();

    //Others
    virtual uint *triggerChannelsRecord() = 0;
    virtual char *triggerChannelsRecordEx() = 0;
    virtual char *triggerChannelsSnapshot() = 0;
    virtual bool isNoteVisible() const;

    //http
    virtual HttpNotificationParams *httpNotificationParams() = 0;

signals:
    void okClicked();
    void cancelClicked();

private slots:
    void onLanguageChanged() override;

    void onTabButtonClicked(int id);

    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

protected:
    Ui::ActionAbstract *ui = nullptr;

    MyButtonGroup *m_buttonGroup = nullptr;

    Tab m_currentTab = TabNone;
    QMap<Tab, ActionPageAbstract *> m_pageMap;

    int m_channel = -1;

    friend class ActionPageAudibleWarning;
    friend class ActionPageEmailLinkage;
    friend class ActionPageEventPopup;
    friend class ActionPagePtzAction;
    friend class ActionPageWhiteLed;
    friend class ActionPageAlarmOutput;
    friend class ActionPageHTTPNotification;
    friend class ActionPageOthers;

private:
    bool m_cache = false;
};

#endif // ACTIONABSTRACT_H
