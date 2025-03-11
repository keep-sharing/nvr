#ifndef EFFECTIVETIMEVCA_H
#define EFFECTIVETIMEVCA_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimeVCA : public EffectiveTimeAbstract {
    Q_OBJECT

public:
    explicit EffectiveTimeVCA(QWidget *parent = nullptr);

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
};

#endif // EFFECTIVETIMEVCA_H
