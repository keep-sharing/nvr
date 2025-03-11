#include "EffectiveTimeCameraAlarmInput.h"
#include "ui_EffectiveTime.h"
#include "MsMessage.h"

EffectiveTimeCameraAlarmInput::EffectiveTimeCameraAlarmInput(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

QColor EffectiveTimeCameraAlarmInput::scheduleColor() const
{
    return QColor(255, 54, 0);
}

int EffectiveTimeCameraAlarmInput::scheduleType() const
{
    return ALARMIN_ACTION;
}

schedule_day *EffectiveTimeCameraAlarmInput::readSchedule()
{
    auto &data = m_dataMap[dataIndex()];

    memset(&data.schedule, 0, sizeof(data.schedule));
    alarm_chn alarm_channel;
    alarm_channel.chnid = channel();
    alarm_channel.alarmid = dataIndex();
    read_alarm_chnIn_effective_schedule(SQLITE_FILE_NAME, &data.schedule, &alarm_channel);
    return data.schedule.schedule_day;
}

void EffectiveTimeCameraAlarmInput::saveSchedule()
{
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        int index = iter.key();
        Data &data = iter.value();

        alarm_chn alarm_channel;
        alarm_channel.chnid = channel();
        alarm_channel.alarmid = index;
        write_alarm_chnIn_effective_schedule(SQLITE_FILE_NAME, &data.schedule, &alarm_channel);
    }
}
