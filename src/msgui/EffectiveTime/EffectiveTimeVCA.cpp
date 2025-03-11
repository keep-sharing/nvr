#include "EffectiveTimeVCA.h"

EffectiveTimeVCA::EffectiveTimeVCA(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

QColor EffectiveTimeVCA::scheduleColor() const
{
    return QColor(252, 174, 200);
}

int EffectiveTimeVCA::scheduleType() const
{
    return SMART_EVT_RECORD;
}

schedule_day *EffectiveTimeVCA::readSchedule()
{
    SMART_EVENT_TYPE vcaType = static_cast<SMART_EVENT_TYPE>(dataIndex());
    auto &data = m_dataMap[vcaType];
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel(), vcaType);
    return data.schedule.schedule_day;
}

void EffectiveTimeVCA::saveSchedule()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        SMART_EVENT_TYPE vcaType = static_cast<SMART_EVENT_TYPE>(iter.key());
        Data &data = iter.value();
        write_smart_event_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel(), vcaType);
    }
}
