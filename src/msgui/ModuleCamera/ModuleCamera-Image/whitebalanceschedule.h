#ifndef WHITEBALANCESCHEDULE_H
#define WHITEBALANCESCHEDULE_H

#include "BaseShadowDialog.h"
#include "imagescheduleedit.h"

extern "C" {
#include "msdefs.h"
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class WhiteBalanceSchedule;
}

class WhiteBalanceSchedule : public BaseShadowDialog {
    Q_OBJECT

public:
    enum ActionWhiteBalanceMode {
        ACTION_AUTOMODE,
        ACTION_MANUALMODE,
        ACTION_INCANDESCENT,
        ACTION_WARMLIGHT,
        ACTION_NATURALLIGHT,
        ACTION_FLUORESCENT
    };
    explicit WhiteBalanceSchedule(QWidget *parent = nullptr);
    ~WhiteBalanceSchedule();

    void setSchedule(schedule_day *schedule_day_array);
    void getSchedule(schedule_day *schedule_day_array);
    void showAction(int channel);

    void ON_RESPONSE_FLAG_GET_WHITE_BALANCE_SCHE(MessageReceive *message);
    void apply();

private slots:
    void onLanguageChanged();
    void onEditingFinished();

    void onButtonGroupClicked(int index);

    void on_pushButton_default_clicked();
    void on_pushButton_selectAll_clicked();
    void on_pushButton_editTime_clicked();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void on_pushButton_ManualMode_clicked();
    void on_pushButton_Incandescent_clicked();
    void on_pushButton_WarmLight_clicked();
    void on_pushButton_NaturalLight_clicked();
    void on_pushButton_Fluorescent_clicked();
    void on_pushButton_AutoMode_clicked();

private:
    Ui::WhiteBalanceSchedule *ui;

    ImageScheduleEdit *m_schedultEdit;
    int m_channel;
    int m_mode;

    white_balance_schedule *m_whiteBalanceShce = nullptr;
};

#endif // WHITEBALANCESCHEDULE_H
