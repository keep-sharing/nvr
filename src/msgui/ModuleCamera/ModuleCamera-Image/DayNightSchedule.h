#ifndef DAYNIGHTSCHEDULE_H
#define DAYNIGHTSCHEDULE_H

#include "BaseShadowDialog.h"
#include "DayNightScheTimeEdit.h"

extern "C" {
#include "msdefs.h"
#include "msg.h"
}

namespace Ui {
class DayNightSchedule;
}

class DayNightSchedule : public BaseShadowDialog {
    Q_OBJECT

public:
    enum ActionDayNightMode {
        ACTION_ERASE,
        ACTION_TEMPLATE1,
        ACTION_TEMPLATE2,
        ACTION_TEMPLATE3,
        ACTION_TEMPLATE4,
        ACTION_TEMPLATE5
    };
    explicit DayNightSchedule(QWidget *parent = nullptr);
    ~DayNightSchedule() override;
    void setSchedule(schedule_day *schedule_day_array);
    void getSchedule(schedule_day *schedule_day_array);
    void showAction(int channel, set_image_day_night_str *info);
    void changeToScheduleDay();
    void changeToImageStr();

private:
    void onLanguageChanged() override;

private slots:
    void onEditingFinished();

    void on_pushButton_default_clicked();
    void on_pushButton_selectAll_clicked();
    void on_pushButton_editTime_clicked();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

    void on_pushButtonTemplate1_clicked();
    void on_pushButtonTemplate2_clicked();
    void on_pushButtonTemplate3_clicked();
    void on_pushButtonTemplate4_clicked();
    void on_pushButtonTemplate5_clicked();
    void on_pushButtonErase_clicked();

private:
    Ui::DayNightSchedule *ui;
    DayNightScheTimeEdit *m_schedultEdit;
    int m_channel;
    int m_mode;
    schedule_day m_dayNightShce[MAX_DAY_NUM];
    set_image_day_night_str *m_info = nullptr;
};

#endif // DAYNIGHTSCHEDULE_H
