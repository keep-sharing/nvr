#ifndef EFFECTIVETIMECAMERAALARMOUTPUT_H
#define EFFECTIVETIMECAMERAALARMOUTPUT_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimeCameraAlarmOutput : public EffectiveTimeAbstract
{
    Q_OBJECT
public:
    explicit EffectiveTimeCameraAlarmOutput(QWidget *parent = nullptr);

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
};

#endif // EFFECTIVETIMECAMERAALARMOUTPUT_H
