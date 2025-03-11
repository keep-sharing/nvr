#ifndef RECORDSCHEDULE_H
#define RECORDSCHEDULE_H

#include "AbstractSettingPage.h"

class QButtonGroup;
class BlueLabel;

extern "C" {
#include "msdb.h"
}

namespace Ui {
class RecordSchedule;
}

class RecordSchedule : public AbstractSettingPage {
    Q_OBJECT

    enum Tab {
        TabRecoreSchedule,
        TabBatchSettings,
        TabRecordSettings
    };

public:
    explicit RecordSchedule(QWidget *parent = 0);
    ~RecordSchedule();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SET_ALL_RECORD(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_RECPARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_ANR_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_ANR_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_IPC_CONNECT_STATE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_RECORDSCHED(MessageReceive *message);

    void showRecordSchedule();
    bool isSupportPreviousRecord();

private slots:
    void onLanguageChanged() override;

    void onTabClicked(int index);
    void onChannelButtonClicked(int index);
    void onChannelCheckBoxClicked(int channel, bool checked);

    void copyData();

    void on_pushButton_erase_clicked();
    void on_pushButton_continuous_clicked();
    void on_pushButton_event_clicked();
    void on_pushButton_motion_clicked();
    void on_pushButton_alarm_clicked();
    void on_pushButton_smartEvent_clicked();
    void on_pushButton_smartAnalysis_clicked();
    void on_pushButtonAudioAlarm_clicked();

    void onPushButtonCopyClicked();
    void onPushButtonApplyClicked();
    void onPushButtonBackClicked();

    void onPushButtonStartRecordClicked();
    void onPushButtonStopRecordClicked();

    void on_pushButton_copy_2_clicked();
    void on_pushButton_apply_2_clicked();
    void on_pushButton_back_3_clicked();
    void on_comboBox_videoDueTime_activated(int index);
    void onChannelButton2Clicked(int index);
    void onCopySettingsData();

private:
    Ui::RecordSchedule *ui;

    QButtonGroup *m_scheduleTypeGroup;

    int m_currentChannel = 0;
    record_schedule m_record;

    QList<int> m_copyList;

    record m_dbRecord;
    camera m_dbCamera;
    QList<int> m_setAnrList;
};

#endif // RECORDSCHEDULE_H
