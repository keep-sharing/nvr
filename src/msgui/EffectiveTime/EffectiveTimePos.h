#ifndef EFFECTIVETIMEPOS_H
#define EFFECTIVETIMEPOS_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimePos : public EffectiveTimeAbstract {
    Q_OBJECT

public:
    explicit EffectiveTimePos(QWidget *parent = nullptr);
    ~EffectiveTimePos() override;

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
    void saveSchedule(int dataIndex) override;
};

#endif // EFFECTIVETIMEPOS_H
