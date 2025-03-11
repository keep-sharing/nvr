#ifndef TABCAMERAALARMINPUT_H
#define TABCAMERAALARMINPUT_H

#include "AbstractSettingTab.h"

class EffectiveTimeCameraAlarmInput;
class ActionCameraAlarmInput;

class MyButtonGroup;

namespace Ui {
class TabCameraAlarmInput;
}

class TabCameraAlarmInput : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabCameraAlarmInput(QWidget *parent = nullptr);
    ~TabCameraAlarmInput();

    void initializeData();
    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_GET_IPC_ALARM_INPUT(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_ALARM_INPUT_BATCH(MessageReceive *message);

    void setSettingEnable(bool enable);
    void showAlarminput(CAMERA_ALARMIN_IN_MODE index);
    void saveAlarminput(int channel, CAMERA_ALARMIN_IN_MODE index,ms_ipc_alarm_in_cfg *info);

    QString readAlarminName(CAMERA_ALARMIN_IN_MODE index);
    void writeAlarminName(CAMERA_ALARMIN_IN_MODE index, const QString &name);
    void copyAlarmInput(CAMERA_ALARMIN_IN_MODE index, quint64 copyFlags);

signals:
    void back();

private slots:
    void onLanguageChanged();

    void onChannelGroupClicked(int index);
    void onAlarmInputGroupClicked(int index);

    void on_checkBox_enable_clicked(bool checked);
    void on_lineEdit_name_textEdited(const QString &text);
    void on_comboBox_type_activated(int index);

    void on_pushButton_effectiveTime_clicked();
    void on_pushButton_action_clicked();

    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_checkBox_enable_stateChanged(int arg1);

private:
    Ui::TabCameraAlarmInput *ui;

    MyButtonGroup *m_alarmButtonGroup = nullptr;

    int m_currentChannel = 0;
    CAMERA_ALARMIN_IN_MODE m_currentAlarmIndex = ActionCameraAlarmInput1;
    ms_ipc_alarm_in m_ipc_alarm_in;
    QString m_alarmName[4];

    EffectiveTimeCameraAlarmInput *m_effective = nullptr;
    ActionCameraAlarmInput *m_action = nullptr;
};

#endif // TABCAMERAALARMINPUT_H
