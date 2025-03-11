#ifndef EFFECTIVETIMELINECROSSING_H
#define EFFECTIVETIMELINECROSSING_H

#include <QWidget>
#include "EffectiveTimeAbstract.h"

class EffectiveTimeLineCrossing : public EffectiveTimeAbstract
{
    Q_OBJECT
public:
    explicit EffectiveTimeLineCrossing(QWidget *parent = nullptr);

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
    void saveSchedule(int dataIndex) override;
};

#endif // EFFECTIVETIMELINECROSSING_H
