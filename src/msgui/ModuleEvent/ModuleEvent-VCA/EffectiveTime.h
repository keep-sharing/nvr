#ifndef EFFECTIVETIME_H
#define EFFECTIVETIME_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class EffectiveTime;
}

class EffectiveTime : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit EffectiveTime(QWidget *parent = 0);
    ~EffectiveTime();

    void readSmartData(int channel, SMART_EVENT_TYPE type);
    void readMotionData(int channel);
    void readAlarminData(int channel);
    void readAlarmoutData(int channel);
    void readAnprData(int channel, ANPR_MODE_TYPE type);

    virtual void clearCache();
    virtual void apply();

protected:
    bool isAddToVisibleList() override;

protected slots:
    void onLanguageChanged();

    virtual void on_pushButton_effective_clicked();
    virtual void on_pushButton_erase_clicked();

    virtual void onPushButton_okClicked();
    virtual void onPushButton_cancelClicked();

protected:
    Ui::EffectiveTime *ui;

    smart_event_schedule *m_eventSchedule = nullptr;
    alarm_out_schedule *m_alarm_out_schedule = nullptr;
    int m_channel = 0;
    int m_actionType;
    int m_smartType;
    int m_anprType;
};

#endif // EFFECTIVETIME_H
