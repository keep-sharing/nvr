#ifndef BWHSCHEDULE_H
#define BWHSCHEDULE_H

#include "BaseShadowDialog.h"
#include "imagescheduleedit.h"

extern "C" {
#include "msdefs.h"
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class BWHSchedule;
}

class BWHSchedule : public BaseShadowDialog {
    Q_OBJECT

public:
    enum ActionBlcWdrHlcMode {
        ACTION_BLC,
        ACTION_WDR,
        ACTION_HLC
    };
    explicit BWHSchedule(QWidget *parent = 0);
    ~BWHSchedule() override;

    void setSchedule(schedule_day *schedule_day_array);
    void getSchedule(schedule_day *schedule_day_array);
    void showAction(int channel);

    void ON_RESPONSE_FLAG_GET_BWH_SCHE(MessageReceive *message);

    void apply();

private:
    void onLanguageChanged() override;

private slots:
    void onEditingFinished();

    void onButtonGroupClicked(int index);

    void on_pushButton_default_clicked();
    void on_pushButton_selectAll_clicked();
    void on_pushButton_editTime_clicked();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

    void on_pushButton_WDR_clicked();
    void on_pushButton_HLC_clicked();
    void on_pushButton_BLC_clicked();

private:
    Ui::BWHSchedule *ui;

    ImageScheduleEdit *m_schedultEdit;
    int m_channel;
    int m_mode;
    bwh_schedule *m_BlcWdrHlcShce = nullptr;
};

#endif // BWHSCHEDULE_H
