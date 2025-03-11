#include "EffectiveTimeNvrAlarmInput.h"

EffectiveTimeNvrAlarmInput::EffectiveTimeNvrAlarmInput(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

QColor EffectiveTimeNvrAlarmInput::scheduleColor() const
{
    return QColor(255, 54, 0);
}

int EffectiveTimeNvrAlarmInput::scheduleType() const
{
    return ALARMIN_ACTION;
}

schedule_day *EffectiveTimeNvrAlarmInput::readSchedule()
{
    auto &data = m_dataMap[dataIndex()];

    //read_alarm_in_schedule(SQLITE_FILE_NAME, (alarm_in_schedule *)&data.schedule, channel());
    read_alarmin_effective_schedule(SQLITE_FILE_NAME, &data.schedule, dataIndex());
    return data.schedule.schedule_day;
}

void EffectiveTimeNvrAlarmInput::saveSchedule()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        Data &data = iter.value();

        write_alarmin_effective_schedule(SQLITE_FILE_NAME, &data.schedule, dataIndex());
    }
}
