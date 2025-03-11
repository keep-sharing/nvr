#ifndef HEATMAPSCHEDULE_H
#define HEATMAPSCHEDULE_H

#include "EffectiveTime.h"

class HeatMapSchedule : public EffectiveTime
{
    Q_OBJECT

public:
    explicit HeatMapSchedule(QWidget *parent = nullptr);

    void showSchedule(schedule_day *day_array);

signals:

protected slots:
    void onPushButton_okClicked() override;
    void onPushButton_cancelClicked() override;

private:
    schedule_day *m_dayArray = nullptr;
};

#endif // HEATMAPSCHEDULE_H
