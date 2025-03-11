#include "EffectiveTimeAudioAlarm.h"

EffectiveTimeAudioAlarm::EffectiveTimeAudioAlarm(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
}

void EffectiveTimeAudioAlarm::saveEffectiveTime(int channel, Uint64 chnMask)
{
    setChannel(channel);
    setDataIndex(0);
    if (!m_dataMap.contains(dataIndex())) {
        readSchedule();
    }
    auto &data = m_dataMap[dataIndex()];
    copy_audio_alarm_sche(SQLITE_FILE_NAME, &data.schedule,const_cast<char *>(AUD_EFFE_SCHE), chnMask);
}
QColor EffectiveTimeAudioAlarm::scheduleColor() const
{
    return QColor("#A484FF");
}

int EffectiveTimeAudioAlarm::scheduleType() const
{
    return AUDIO_ALARM_RECORD;
}

schedule_day *EffectiveTimeAudioAlarm::readSchedule()
{
    auto &data = m_dataMap[dataIndex()];

    memset(&data.schedule, 0, sizeof(data.schedule));
    read_audio_alarm_sche(SQLITE_FILE_NAME, &data.schedule, const_cast<char *>(AUD_EFFE_SCHE), channel());
    return data.schedule.schedule_day;
}

void EffectiveTimeAudioAlarm::saveSchedule()
{

}
