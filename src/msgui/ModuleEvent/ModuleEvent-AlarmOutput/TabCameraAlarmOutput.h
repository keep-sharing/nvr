#ifndef TABCAMERAALARMOUTPUT_H
#define TABCAMERAALARMOUTPUT_H

#include "AbstractSettingTab.h"

class MyButtonGroup;
class EffectiveTimeCameraAlarmOutput;

namespace Ui {
class TabCameraAlarmOutput;
}

class TabCameraAlarmOutput : public AbstractSettingTab {
    Q_OBJECT

    enum AlarmIndex {
        Alarmoutput1,
        Alarmoutput2
    };

public:
    explicit TabCameraAlarmOutput(QWidget *parent = nullptr);
    ~TabCameraAlarmOutput();

    void initializeData();
    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_ALARM_OUTPUT(MessageReceive *message);

    void setSettingEnable(bool enable);
    void showAlarmoutput(AlarmIndex index);
    void saveAlarmoutput(int channel, AlarmIndex index);

    void readAlarmoutName();
    void writeAlarmoutName(int channel);

signals:
    void back();

private slots:
    void onLanguageChanged();

    void onChannelGroupClicked(int index);
    void onAlarmOutputGroupClicked(int index);

    void on_lineEdit_name_textEdited(const QString &text);
    void on_comboBox_type_activated(int index);
    void on_comboBox_delay_activated(int index);
    void on_comboBox_delay_currentIndexChanged(int index);

    void on_pushButton_effectiveTime_clicked();

    void on_pushButton_clearAlarm_clicked();
    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::TabCameraAlarmOutput *ui;

    MyButtonGroup *m_alarmButtonGroup = nullptr;

    int m_currentChannel = 0;
    ms_ipc_alarm_out m_ipc_alarm_out;
    alarm_chn_out_name m_alarm_name_array[2];

    EffectiveTimeCameraAlarmOutput *m_effective = nullptr;

};

#endif // TABCAMERAALARMOUTPUT_H
