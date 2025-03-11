#ifndef EDITALARMOUTPUT_H
#define EDITALARMOUTPUT_H

#include "AddAlarmOutput.h"
#include <QWidget>

class EditAlarmOutput : public AddAlarmOutput {
    Q_OBJECT

public:
    explicit EditAlarmOutput(QMap<AlarmKey, bool> *alarmMap, QWidget *parent = nullptr);
    ~EditAlarmOutput();

    static EditAlarmOutput *instance();

    int execEdit(const AlarmKey &currentKey);

signals:

protected slots:
    void on_pushButton_ok_clicked() override;
    void on_pushButton_cancel_clicked() override;

private:
    static EditAlarmOutput *s_editAlarmOutput;

    AlarmKey m_currentKey;
};

#endif // EDITALARMOUTPUT_H
