#include "PlaybackSplitInfo.h"
#include <qmath.h>

PlaybackSplitInfo::PlaybackSplitInfo()
{
}

void PlaybackSplitInfo::appendCommonBackup(resp_search_common_backup *common, int count)
{
    m_channel = common->chnid;
    m_sid = common->sid;
    for (int i = 0; i < count; ++i) {
        resp_search_common_backup common_backup;
        memcpy(&common_backup, &(common[i]), sizeof(common_backup));
        m_backupList.append(common_backup);
    }
}

int PlaybackSplitInfo::commonBackupListSize() const
{
    return m_backupList.size();
}

bool PlaybackSplitInfo::hasBackup() const
{
    return !m_backupList.isEmpty();
}

int PlaybackSplitInfo::nextCommonBackupPage() const
{
    int page = -1;
    if (!m_backupList.isEmpty()) {
        page = m_backupList.size() / 100 + 1;
    }
    return page;
}

int PlaybackSplitInfo::pageCount() const
{
    int count = 0;
    if (!m_backupList.isEmpty()) {
        count = qCeil(m_backupList.first().allCnt / 100.0);
    }
    return count;
}

QList<resp_search_common_backup> PlaybackSplitInfo::commonBackupList() const
{
    return m_backupList;
}

int PlaybackSplitInfo::channel() const
{
    return m_channel;
}

int PlaybackSplitInfo::sid() const
{
    return m_sid;
}

void PlaybackSplitInfo::setDateTime(const QDateTime &startDateTime, const QDateTime &endDateTime)
{
    m_startDateTime = startDateTime;
    m_endDateTime = endDateTime;
}

QDateTime PlaybackSplitInfo::startDateTime() const
{
    return m_startDateTime;
}

QDateTime PlaybackSplitInfo::endDateTime() const
{
    return m_endDateTime;
}
