#ifndef EFFECTIVETIME_PTZAUTOTRACKING_H
#define EFFECTIVETIME_PTZAUTOTRACKING_H

#include "EffectiveTime.h"

class EffectiveTime_PtzAutoTracking : public EffectiveTime
{
    Q_OBJECT
public:
    explicit EffectiveTime_PtzAutoTracking(QWidget *parent = nullptr);

    void showEffectiveTime(schedule_day *day_array);

signals:

public slots:
    void onPushButton_okClicked() override;
    void onPushButton_cancelClicked() override;

private:
    schedule_day *m_dayArray = nullptr;
};

#endif // EFFECTIVETIME_PTZAUTOTRACKING_H
