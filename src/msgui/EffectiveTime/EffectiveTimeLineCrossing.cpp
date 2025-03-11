#include "EffectiveTimeLineCrossing.h"

EffectiveTimeLineCrossing::EffectiveTimeLineCrossing(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

QColor EffectiveTimeLineCrossing::scheduleColor() const
{
    return QColor(252, 174, 200);
}

int EffectiveTimeLineCrossing::scheduleType() const
{
    return SMART_EVT_RECORD;
}

schedule_day *EffectiveTimeLineCrossing::readSchedule()
{
    if (m_dataMap.contains(dataIndex())) {
        auto &data = m_dataMap[dataIndex()];
        return data.schedule.schedule_day;
    } else {
        auto &data = m_dataMap[dataIndex()];
        memset(&data.schedule, 0, sizeof(data.schedule));
        read_smart_event_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel(), static_cast<SMART_EVENT_TYPE>(dataIndex()));
        return data.schedule.schedule_day;
    }
}

void EffectiveTimeLineCrossing::saveSchedule()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        int index = iter.key();
        Data &data = iter.value();
        write_smart_event_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel(), static_cast<SMART_EVENT_TYPE>(index));
    }
}

void EffectiveTimeLineCrossing::saveSchedule(int dataIndex)
{
    if (m_dataMap.contains(dataIndex)) {
        Data &data = m_dataMap[dataIndex];
        write_smart_event_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel(), static_cast<SMART_EVENT_TYPE>(dataIndex));
    }
}
