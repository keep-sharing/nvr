#ifndef EFFECTIVETIMENVRALARMOUTPUT_H
#define EFFECTIVETIMENVRALARMOUTPUT_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimeNvrAlarmOutput : public EffectiveTimeAbstract {
    Q_OBJECT

public:
    explicit EffectiveTimeNvrAlarmOutput(QWidget *parent = nullptr);

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
};

#endif // EFFECTIVETIMENVRALARMOUTPUT_H
