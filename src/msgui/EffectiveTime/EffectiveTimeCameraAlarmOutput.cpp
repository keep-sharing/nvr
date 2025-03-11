#include "EffectiveTimeCameraAlarmOutput.h"
#include "ui_EffectiveTime.h"
#include "centralmessage.h"

EffectiveTimeCameraAlarmOutput::EffectiveTimeCameraAlarmOutput(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

QColor EffectiveTimeCameraAlarmOutput::scheduleColor() const
{
    return QColor(255, 54, 0);
}

int EffectiveTimeCameraAlarmOutput::scheduleType() const
{
    return ALARMOUT_ACTION;
}

schedule_day *EffectiveTimeCameraAlarmOutput::readSchedule()
{
    auto &data = m_dataMap[dataIndex()];

    memset(&data.schedule, 0, sizeof(data.schedule));
    alarm_chn alarm_channel;
    alarm_channel.chnid = channel();
    alarm_channel.alarmid = dataIndex();
    read_alarm_chnOut_effective_schedule(SQLITE_FILE_NAME, &data.schedule, &alarm_channel);
    return data.schedule.schedule_day;
}

void EffectiveTimeCameraAlarmOutput::saveSchedule()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        int index = iter.key();
        Data &data = iter.value();

        alarm_chn alarm_channel;
        alarm_channel.chnid = channel();
        alarm_channel.alarmid = index;
        write_alarm_chnOut_effective_schedule(SQLITE_FILE_NAME, &data.schedule, &alarm_channel);
    }
}
