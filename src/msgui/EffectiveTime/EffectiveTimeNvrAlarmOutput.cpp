#include "EffectiveTimeNvrAlarmOutput.h"

EffectiveTimeNvrAlarmOutput::EffectiveTimeNvrAlarmOutput(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

QColor EffectiveTimeNvrAlarmOutput::scheduleColor() const
{
    return QColor(255, 54, 0);
}

int EffectiveTimeNvrAlarmOutput::scheduleType() const
{
    return ALARMOUT_ACTION;
}

schedule_day *EffectiveTimeNvrAlarmOutput::readSchedule()
{
    auto &data = m_dataMap[dataIndex()];
    read_alarm_out_schedule(SQLITE_FILE_NAME, (alarm_out_schedule *)&data.schedule, dataIndex());
    return data.schedule.schedule_day;
}

void EffectiveTimeNvrAlarmOutput::saveSchedule()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        Data &data = iter.value();
        write_alarm_out_schedule(SQLITE_FILE_NAME, (alarm_out_schedule *)&data.schedule, dataIndex());
    }
}
