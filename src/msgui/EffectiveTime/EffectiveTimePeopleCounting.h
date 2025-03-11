#ifndef EFFECTIVETIMEPEOPLECOUNTING_H
#define EFFECTIVETIMEPEOPLECOUNTING_H

#include "EffectiveTimeAbstract.h"
enum EffectiveTimePeopleCountingMode {
    PeopleCountingSettingsMode,
    RegionalPeopleCountingSettingsMode,
    HeatMapMode
};

class EffectiveTimePeopleCounting : public EffectiveTimeAbstract
{
    Q_OBJECT
public:
    explicit EffectiveTimePeopleCounting(QWidget *parent = nullptr);
    explicit EffectiveTimePeopleCounting(int mode, QWidget *parent = nullptr);
    ~EffectiveTimePeopleCounting() override;

    void setSchedule(schedule_day *scheduleDay);
    void getSchedule(schedule_day *scheduleDay);

protected:
    QString titleText() const override;
    QString pushButtonEffectiveText() const override;
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
    void saveSchedule(int dataIndex) override;
private:
    int m_mode = 0;
};

#endif // EFFECTIVETIMEPEOPLECOUNTING_H
