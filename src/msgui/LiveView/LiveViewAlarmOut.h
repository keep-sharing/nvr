#ifndef LIVEVIEWALARMOUT_H
#define LIVEVIEWALARMOUT_H

#include "BaseDialog.h"
#include <QWidget>

extern "C" {
#include "msdb.h"
#include "msg.h"
}

class MessageReceive;
class MsWaitting;

namespace Ui {
class LiveViewAlarmOut;
}

class LiveViewAlarmOut : public BaseDialog
{
    Q_OBJECT

    enum AlarmIndex {
        Alarmoutput1,
        Alarmoutput2
    };
public:
    explicit LiveViewAlarmOut(QWidget *parent = nullptr);
    ~LiveViewAlarmOut();

    void initializeData(int channel);
    void moveAlarmOut(const QRect &videoGeometry);
    bool isSupport();
    void showAlarm(int channel);

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_ALARMOUT_STATE(MessageReceive *message);

    void saveAlarmOut(AlarmIndex index);
    QString changeText(QString text);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:

private slots:
    void onLanguageChanged();
    void on_pushButtonClose_clicked();

    void on_comboBoxAlarm1_currentIndexChanged(int index);
    void on_comboBoxAlarm2_currentIndexChanged(int index);

private:
    void readAlarmoutName();

private:
    Ui::LiveViewAlarmOut *ui;

    MsWaitting *m_waitting = nullptr;

    ms_ipc_alarm_out m_ipc_alarm_out;
    alarm_chn_out_name m_alarm_name_array[2];
    int m_currentChannel = -1;
    bool m_isSupport = true;//频道是否支持
    bool m_status = false;//读取是否开始
};

#endif // LIVEVIEWALARMOUT_H
