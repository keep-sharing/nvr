#ifndef EFFECTIVETIMEPTZAUTOTRACKING_H
#define EFFECTIVETIMEPTZAUTOTRACKING_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimePtzAutoTracking : public EffectiveTimeAbstract {
    Q_OBJECT

public:
    explicit EffectiveTimePtzAutoTracking(QWidget *parent = nullptr);

    void showEffectiveTime(int channel, schedule_day *schedule);

protected:
    QString titleText() const override;
    QString pushButtonEffectiveText() const override;

    bool holidayVisible() override;
    schedule_day *schedule() override;
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;

private:
    schedule_day *m_schedule = nullptr;
};

#endif // EFFECTIVETIMEPTZAUTOTRACKING_H
