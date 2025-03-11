#include "PlaybackFileKey.h"

PlaybackFileKey::PlaybackFileKey()
{
}

PlaybackFileKey::PlaybackFileKey(const resp_snapshot_state &snapshot)
{
    setChannel(snapshot.chnid);
    setStartDateTime(snapshot.starTime);
    setEndDateTime(snapshot.endTime);
}

PlaybackFileKey::PlaybackFileKey(const resp_search_common_backup &backup)
{
    setChannel(backup.chnid);
    setStartDateTime(backup.pStartTime);
    setEndDateTime(backup.pEndTime);
}

PlaybackFileKey::PlaybackFileKey(const resp_search_tags &tag)
{
    setChannel(tag.chnid);
    setIndex(tag.index);
    setStartDateTime(tag.pTime);
}

PlaybackFileKey::PlaybackFileKey(int channel, int index, const QDateTime &start, const QDateTime &end)
    : m_channel(channel)
    , m_index(index)
    , m_startTime(start)
    , m_endTime(end)
{
}

void PlaybackFileKey::setChannel(int channel)
{
    m_channel = channel;
}

void PlaybackFileKey::setIndex(int index)
{
    m_index = index;
}

void PlaybackFileKey::setStartDateTime(uint time)
{
    setStartDateTime(QDateTime::fromTime_t(time));
}

void PlaybackFileKey::setStartDateTime(const char *time)
{
    setStartDateTime(QDateTime::fromString(time, "yyyy-MM-dd HH:mm:ss"));
}

void PlaybackFileKey::setStartDateTime(const QDateTime &dateTime)
{
    m_startTime = dateTime;
}

void PlaybackFileKey::setEndDateTime(uint time)
{
    setEndDateTime(QDateTime::fromTime_t(time));
}

void PlaybackFileKey::setEndDateTime(const char *time)
{
    setEndDateTime(QDateTime::fromString(time, "yyyy-MM-dd HH:mm:ss"));
}

void PlaybackFileKey::setEndDateTime(const QDateTime &dateTime)
{
    m_endTime = dateTime;
}

QString PlaybackFileKey::startDateTimeString() const
{
    return m_startTime.toString("yyyy-MM-dd HH:mm:ss");
}

QString PlaybackFileKey::dateTimeString() const
{
    return QString("%1-%2").arg(m_startTime.toString("yyyy-MM-dd HH:mm:ss")).arg(m_endTime.toString("yyyy-MM-dd HH:mm:ss"));
}

bool PlaybackFileKey::operator<(const PlaybackFileKey &other) const
{
    if (m_channel != other.m_channel) {
        return m_channel < other.m_channel;
    } else if (m_startTime != other.m_startTime) {
        return m_startTime < other.m_startTime;
    } else if (m_endTime != other.m_endTime) {
        return m_endTime < other.m_endTime;
    } else {
        return m_index < other.m_index;
    }
}

bool PlaybackFileKey::operator==(const PlaybackFileKey &other) const
{
    if (m_channel != other.m_channel) {
        return false;
    } else if (m_startTime != other.m_startTime) {
        return false;
    } else if (m_endTime != other.m_endTime) {
        return false;
    } else {
        return m_index == other.m_index;
    }
}
