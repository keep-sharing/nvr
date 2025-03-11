#ifndef PTZSCHEDULETASK_H
#define PTZSCHEDULETASK_H

#include "BaseShadowDialog.h"
#include "PtzScheduleTaskEdit.h"
#include "imagescheduleedit.h"
#include <QAbstractButton>

extern "C" {
#include "msdefs.h"
#include "msg.h"
}

namespace Ui {
class PtzScheduleTask;
}

class PtzScheduleTask : public BaseShadowDialog {
    Q_OBJECT

public:
    enum ActionPTZScheMode {
        ACTION_ERASE,
        ACTION_CLOSE,
        ACTION_AUTOSCAN,
        ACTION_PRESET,//id 101~400
        ACTION_PATROL,//id 401~408
        ACTION_PATTERN,//id 501~504
        ACTION_CHECK,
        ACTION_TILTSCAN,
        ACTION_PANORAMASCAN,
    };
    explicit PtzScheduleTask(QWidget *parent = nullptr);
    ~PtzScheduleTask();
    void showAction(IPC_PTZ_SCHE_TASK_WEEK_S &schedule, int supportWiper,int supportSpeed);
    void setSchedule(IPC_PTZ_SCHE_TASK_WEEK_S &schedule);
    void getSchedule(IPC_PTZ_SCHE_TASK_WEEK_S &schedule);

private slots:
    void onEditingFinished();
    void onLanguageChanged() override;
    void onButtonGroupClicked(QAbstractButton *Button);
    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

    void on_pushButtonClose_clicked();
    void on_pushButtonAutoScan_clicked();
    void on_pushButtonPreset_clicked();
    void on_pushButtonPatrol_clicked();
    void on_pushButtonPattern_clicked();
    void on_pushButtonSelfCheck_clicked();
    void on_pushButtonTiltScan_clicked();
    void on_pushButtonPanoramaScan_clicked();
    void on_pushButtonErase2_clicked();
    void on_pushButtonErase_clicked();

    void on_pushButtonDefault_clicked();
    void on_pushButtonSelectAll_clicked();
    void on_pushButtonEditTime_clicked();

private:
    Ui::PtzScheduleTask *ui;
    PtzScheduleTaskEdit *m_schedultEdit;
    schedule_day m_schedule_day[MAX_DAY_NUM];
    bool m_supportWiper = false;
    bool m_supportSpeed = false;
};

#endif // PTZSCHEDULETASK_H
