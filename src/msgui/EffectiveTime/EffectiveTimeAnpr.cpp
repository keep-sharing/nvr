#include "EffectiveTimeAnpr.h"

EffectiveTimeAnpr::EffectiveTimeAnpr(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

QColor EffectiveTimeAnpr::scheduleColor() const
{
    return QColor(255, 231, 147);
}

int EffectiveTimeAnpr::scheduleType() const
{
    return ANPT_EVT_RECORD;
}

schedule_day *EffectiveTimeAnpr::readSchedule()
{
    if (m_dataMap.contains(dataIndex())) {
        auto &data = m_dataMap[dataIndex()];
        return data.schedule.schedule_day;
    } else {
        auto &data = m_dataMap[dataIndex()];
        memset(&data.schedule, 0, sizeof(data.schedule));
        read_anpr_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel(), static_cast<ANPR_MODE>(dataIndex()));
        return data.schedule.schedule_day;
    }
}

void EffectiveTimeAnpr::saveSchedule()
{

}

void EffectiveTimeAnpr::saveSchedule(int dataIndex)
{
    if (m_dataMap.contains(dataIndex)) {
        Data &data = m_dataMap[dataIndex];
        write_anpr_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel(), static_cast<ANPR_MODE>(dataIndex));
    }
}
