#ifndef PLAYBACKFILEKEY_H
#define PLAYBACKFILEKEY_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

extern "C" {
#include "msg.h"
}

class PlaybackFileKey {
public:
    PlaybackFileKey();
    PlaybackFileKey(const resp_snapshot_state &snapshot);
    PlaybackFileKey(const resp_search_common_backup &backup);
    PlaybackFileKey(const resp_search_tags &tag);
    PlaybackFileKey(int channel, int index, const QDateTime &start, const QDateTime &end);

    void setChannel(int channel);
    void setIndex(int index);
    void setStartDateTime(uint time);
    void setStartDateTime(const char *time);
    void setStartDateTime(const QDateTime &dateTime);
    void setEndDateTime(uint time);
    void setEndDateTime(const char *time);
    void setEndDateTime(const QDateTime &dateTime);

    QString startDateTimeString() const;
    QString dateTimeString() const;

    bool operator<(const PlaybackFileKey &other) const;
    bool operator==(const PlaybackFileKey &other) const;

private:
    int m_channel = -1;
    int m_index = -1;
    QDateTime m_startTime;
    QDateTime m_endTime;
};
Q_DECLARE_METATYPE(PlaybackFileKey)

#endif // PLAYBACKFILEKEY_H
