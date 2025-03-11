#ifndef EFFECTIVETIMEMOTIONDETECTION_H
#define EFFECTIVETIMEMOTIONDETECTION_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimeMotionDetection : public EffectiveTimeAbstract {
    Q_OBJECT

public:
    explicit EffectiveTimeMotionDetection(QWidget *parent = nullptr);

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
};

#endif // EFFECTIVETIMEMOTIONDETECTION_H
