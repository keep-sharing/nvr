#ifndef TABPTZSCHEDULEDTASKS_H
#define TABPTZSCHEDULEDTASKS_H

#include "PtzScheduleTask.h"
#include "ptzbasepage.h"
#include <QEventLoop>

namespace Ui {
class TabPtzScheduledTasks;
}

class TabPtzScheduledTasks : public PtzBasePage {
    Q_OBJECT

public:
    explicit TabPtzScheduledTasks(QWidget *parent = nullptr);
    ~TabPtzScheduledTasks() override;

    void initializeData() override;
    void processMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_GET_IPC_PTZ_SCHE_TASK(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_PTZ_SCHE_TASK(MessageReceive *message);
    void setSettingEnable(bool enable);

private:
    void setScheduleArray(IPC_PTZ_SCHE_TASK_WEEK_S &schedule);
    void getScheduleArray(IPC_PTZ_SCHE_TASK_WEEK_S &schedule);

private slots:
    void onLanguageChanged();
    void on_comboBoxChannel_activated(int index);
    void on_checkBoxEnable_clicked();
    void on_pushButtonSchedule_clicked();

    void on_pushButtonCopy_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::TabPtzScheduledTasks *ui;
    int m_channel;
    PtzScheduleTask *m_ptzSchedule = nullptr;
    QList<int> m_copyList;
    QEventLoop m_eventLoop;
    IPC_PTZ_SCHE_TASK_INFO_S m_scheInfo;
};

#endif // TABPTZSCHEDULEDTASKS_H
