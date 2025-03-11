#ifndef EFFECTIVETIMEAUDIOALARM_H
#define EFFECTIVETIMEAUDIOALARM_H

#include "EffectiveTimeAbstract.h"

class EffectiveTimeAudioAlarm : public EffectiveTimeAbstract
{
    Q_OBJECT
public:
    explicit EffectiveTimeAudioAlarm(QWidget *parent = nullptr);
    void saveEffectiveTime(int channel, Uint64 chnMask);

protected:
    QColor scheduleColor() const override;
    int scheduleType() const override;
    schedule_day *readSchedule() override;
    void saveSchedule() override;
};

#endif // EFFECTIVETIMEAUDIOALARM_H
