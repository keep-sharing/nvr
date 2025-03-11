#include "EffectiveTimePtzAutoTracking.h"
#include "MsLanguage.h"

EffectiveTimePtzAutoTracking::EffectiveTimePtzAutoTracking(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

void EffectiveTimePtzAutoTracking::showEffectiveTime(int channel, schedule_day *schedule)
{
    m_schedule = schedule;
    EffectiveTimeAbstract::showEffectiveTime(channel);
}

QString EffectiveTimePtzAutoTracking::titleText() const
{
    return GET_TEXT("PTZCONFIG/36060", "Auto Tracking Schedule");
}

QString EffectiveTimePtzAutoTracking::pushButtonEffectiveText() const
{
    return GET_TEXT("PTZCONFIG/36052", "Auto Tracking");
}

bool EffectiveTimePtzAutoTracking::holidayVisible()
{
    return false;
}

schedule_day *EffectiveTimePtzAutoTracking::schedule()
{
    return m_schedule;
}

QColor EffectiveTimePtzAutoTracking::scheduleColor() const
{
    return QColor(44, 184, 228);
}

int EffectiveTimePtzAutoTracking::scheduleType() const
{
    return 13;
}

schedule_day *EffectiveTimePtzAutoTracking::readSchedule()
{
    return m_schedule;
}

void EffectiveTimePtzAutoTracking::saveSchedule()
{
}
