#include "EffectiveTimeMotionDetection.h"

EffectiveTimeMotionDetection::EffectiveTimeMotionDetection(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

QColor EffectiveTimeMotionDetection::scheduleColor() const
{
    return QColor(27, 193, 91);
}

int EffectiveTimeMotionDetection::scheduleType() const
{
    return MOTION_ACTION;
}

schedule_day *EffectiveTimeMotionDetection::readSchedule()
{
    auto &data = m_dataMap[dataIndex()];

    memset(&data.schedule, 0, sizeof(data.schedule));
    read_motion_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel());
    return data.schedule.schedule_day;
}

void EffectiveTimeMotionDetection::saveSchedule()
{
    qDebug()<<"[gsjt debug]write motion effective from motion detection page by channel:"<<channel();
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        Data &data = iter.value();
        write_motion_effective_schedule(SQLITE_FILE_NAME, &data.schedule, channel());
    }
}
