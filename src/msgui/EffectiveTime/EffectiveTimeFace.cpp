#include "EffectiveTimeFace.h"

EffectiveTimeFace::EffectiveTimeFace(QWidget *parent) : EffectiveTimeAbstract(parent)
{

}

QColor EffectiveTimeFace::scheduleColor() const
{
    return QColor(255, 231, 147);
}

int EffectiveTimeFace::scheduleType() const
{
    return FACE_RECORD;
}

schedule_day *EffectiveTimeFace::readSchedule()
{
    auto &data = m_dataMap[dataIndex()];

    memset(&data.schedule, 0, sizeof(data.schedule));
    read_face_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel());
    return data.schedule.schedule_day;
}

void EffectiveTimeFace::saveSchedule()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        Data &data = iter.value();
        write_face_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel());
    }
}
