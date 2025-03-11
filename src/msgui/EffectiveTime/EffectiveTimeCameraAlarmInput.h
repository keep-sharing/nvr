#ifndef EFFECTIVETIMECAMERAALARMINPUT_H
#define EFFECTIVETIMECAMERAALARMINPUT_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimeCameraAlarmInput : public EffectiveTimeAbstract {
    Q_OBJECT

public:
    explicit EffectiveTimeCameraAlarmInput(QWidget *parent = nullptr);

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
};

#endif // EFFECTIVETIMECAMERAALARMINPUT_H
