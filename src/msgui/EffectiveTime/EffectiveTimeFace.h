#ifndef EFFECTIVETIMEFACE_H
#define EFFECTIVETIMEFACE_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimeFace : public EffectiveTimeAbstract
{
    Q_OBJECT
public:
    explicit EffectiveTimeFace(QWidget *parent = nullptr);
protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
};

#endif // EFFECTIVETIMEFACE_H
