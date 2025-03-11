#ifndef EFFECTIVETIMEANPR_H
#define EFFECTIVETIMEANPR_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimeAnpr : public EffectiveTimeAbstract {
    Q_OBJECT

public:
    explicit EffectiveTimeAnpr(QWidget *parent = nullptr);

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
    void saveSchedule(int dataIndex) override;
};

#endif // EFFECTIVETIMEANPR_H
