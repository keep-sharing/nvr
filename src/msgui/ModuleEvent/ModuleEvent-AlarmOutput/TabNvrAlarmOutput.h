#ifndef TABNVRALARMOUTPUT_H
#define TABNVRALARMOUTPUT_H

#include "AbstractSettingTab.h"

class EffectiveTimeNvrAlarmOutput;

namespace Ui {
class TabNvrAlarmOutput;
}

class TabNvrAlarmOutput : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabNvrAlarmOutput(QWidget *parent = nullptr);
    ~TabNvrAlarmOutput();

    void initializeData();
    void processMessage(MessageReceive *message);

signals:
    void back();

private slots:
    void onLanguageChanged();

    void copyData();
    void onButtonGroupClicked(int index);

    void on_comboBox_dwellTime_currentIndexChanged(int index);
    void on_pushButton_effectiveTime_clicked();

    void on_pushButton_clearAlarm_clicked();
    void onPushButtonCopyClicked();
    void onPushButtonApplyClicked();
    void onPushButtonBackClicked();

private:
    Ui::TabNvrAlarmOutput *ui;

    int m_currentIndex = 0;
    alarm_out m_alarmout;
    alarm_out_schedule *m_alarmoutSchedule = nullptr;

    EffectiveTimeNvrAlarmOutput *m_effective = nullptr;

    QList<int> m_copyList;
};

#endif // TABNVRALARMOUTPUT_H
