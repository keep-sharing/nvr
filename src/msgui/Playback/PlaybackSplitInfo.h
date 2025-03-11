#ifndef PLAYBACKSPLITINFO_H
#define PLAYBACKSPLITINFO_H

#include <QDateTime>
#include <QMap>

extern "C" {
#include "msg.h"
}

class PlaybackSplitInfo {
public:
    PlaybackSplitInfo();

    void appendCommonBackup(resp_search_common_backup *common, int count);
    int commonBackupListSize() const;
    bool hasBackup() const;
    int nextCommonBackupPage() const;
    int pageCount() const;
    QList<resp_search_common_backup> commonBackupList() const;

    int channel() const;
    int sid() const;

    void setDateTime(const QDateTime &startDateTime, const QDateTime &endDateTime);
    QDateTime startDateTime() const;
    QDateTime endDateTime() const;

private:
    int m_channel = -1;
    int m_sid = -1;
    QDateTime m_startDateTime;
    QDateTime m_endDateTime;
    QList<resp_search_common_backup> m_backupList;
};

#endif // PLAYBACKSPLITINFO_H
