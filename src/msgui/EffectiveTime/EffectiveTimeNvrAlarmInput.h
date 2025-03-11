#ifndef EFFECTIVETIMENVRALARMINPUT_H
#define EFFECTIVETIMENVRALARMINPUT_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimeNvrAlarmInput : public EffectiveTimeAbstract
{
    Q_OBJECT
public:
    explicit EffectiveTimeNvrAlarmInput(QWidget *parent = nullptr);

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
};

#endif // EFFECTIVETIMENVRALARMINPUT_H
