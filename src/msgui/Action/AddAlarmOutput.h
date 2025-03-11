#ifndef ADDALARMOUTPUT_H
#define ADDALARMOUTPUT_H

#include "AlarmKey.h"
#include "BaseShadowDialog.h"
#include "MsWaitting.h"

class MessageReceive;

namespace Ui {
class AddAlarmOutput;
}

class AddAlarmOutput : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit AddAlarmOutput(QMap<AlarmKey, bool> *alarmMap, QWidget *parent = nullptr);
    virtual ~AddAlarmOutput();

    static AddAlarmOutput *instance();

    void processMessage(MessageReceive *message);

protected:
    void ON_RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT(MessageReceive *message);

protected slots:
    void onLanguageChanged();

    virtual void on_pushButton_ok_clicked();
    virtual void on_pushButton_cancel_clicked();

protected:
    Ui::AddAlarmOutput *ui;
    MsWaitting *m_waitting = nullptr;

    QMap<AlarmKey, bool> *m_alarmMap = nullptr;

private slots:
    void on_comboBox_channel_currentIndexChanged(int index);

private:
    static AddAlarmOutput *s_addAlarmOutput;
};

#endif // ADDALARMOUTPUT_H
