#ifndef SNAPSHOTSCHEDULE_H
#define SNAPSHOTSCHEDULE_H

#include "AbstractSettingPage.h"

class QButtonGroup;
class BlueLabel;

extern "C" {
#include "msdb.h"
}

namespace Ui {
class SnapshotSchedule;
}

class SnapshotSchedule : public AbstractSettingPage {
    Q_OBJECT

    enum Tab {
        TabSnapshotSchedule,
        TabBatchSettings,
        TabSnapshotSettings,
        TabNone
    };

public:
    explicit SnapshotSchedule(QWidget *parent = 0);
    ~SnapshotSchedule();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SET_SNAPSHOT_SCHE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_SNAPSHOT(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_ALL_SNAPSHOT(MessageReceive *message);

    void showRecordSchedule();

private slots:
    void onLanguageChanged();
    void onTabClicked(int index);
    void onChannelButtonClicked(int index);
    void onPushButtonCopyClicked();
    void onPushButtonApplyClicked();
    void onPushButtonBackClicked();

    void on_comboBox_dueTime_activated(int index);
    void on_pushButton_erase_clicked();
    void on_pushButton_continuous_clicked();
    void on_pushButton_event_clicked();
    void on_pushButton_motion_clicked();
    void on_pushButton_alarm_clicked();
    void on_pushButton_smartEvent_clicked();
    void on_pushButtonAudioAlarm_clicked();
    void onCopyData();
    void on_comboBox_interval_activated(int index);
    void onChannelCheckBoxClicked(int channel, bool checked);
    void onPushButtonStartSnapshotClicked();
    void onPushButtonStopSnapshotClicked();

    void on_pushButton_smartAnalysis_clicked();

private:
    Ui::SnapshotSchedule *ui;

    snapshot_schedule m_dbSchedule;
    QButtonGroup *m_scheduleTypeGroup;
    int m_currentChannel = 0;
    int m_currenrtTab = TabSnapshotSchedule;
    QList<int> m_copyList;

    snapshot m_dbSnapshot;
};

#endif // SNAPSHOTSCHEDULE_H
