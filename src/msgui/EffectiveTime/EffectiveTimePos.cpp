#include "EffectiveTimePos.h"

EffectiveTimePos::EffectiveTimePos(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{

}

EffectiveTimePos::~EffectiveTimePos()
{

}

QColor EffectiveTimePos::scheduleColor() const
{
    return QColor(255, 231, 147);
}

int EffectiveTimePos::scheduleType() const
{
    return POS_RECORD;
}

schedule_day *EffectiveTimePos::readSchedule()
{
    if (m_dataMap.contains(dataIndex())) {
        auto &data = m_dataMap[dataIndex()];
        return data.schedule.schedule_day;
    } else {
        auto &data = m_dataMap[dataIndex()];
        memset(&data.schedule, 0, sizeof(data.schedule));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_EFFECTIVE, &data.schedule, dataIndex());
        return data.schedule.schedule_day;
    }
}

void EffectiveTimePos::saveSchedule()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        int index = iter.key();
        Data &data = iter.value();
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_EFFECTIVE, &data.schedule, index);
    }
}

void EffectiveTimePos::saveSchedule(int dataIndex)
{
    if (m_dataMap.contains(dataIndex)) {
        Data &data = m_dataMap[dataIndex];
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_EFFECTIVE, &data.schedule, dataIndex);
    }
}
