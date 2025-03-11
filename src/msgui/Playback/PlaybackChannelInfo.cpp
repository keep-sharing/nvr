#include "PlaybackChannelInfo.h"
#include <QDateTime>
#include <qmath.h>

PlaybackChannelInfo::PlaybackChannelInfo()
{
}

PlaybackChannelInfo::PlaybackChannelInfo(int channel)
    : m_channel(channel)
{
}

bool PlaybackChannelInfo::isValid() const
{
    return m_channel >= 0;
}

int PlaybackChannelInfo::channel() const
{
    return m_channel;
}

int PlaybackChannelInfo::sid() const
{
    return m_sid;
}

int PlaybackChannelInfo::eventSid() const
{
    return m_eventSid;
}

int PlaybackChannelInfo::eventSid_smartSearch() const
{
    return m_eventSid_smartSearch;
}

void PlaybackChannelInfo::setState(MsChannelState state)
{
    m_state = state;

    switch (m_state) {
    case ChannelStoped:
        m_channel = -1;
        m_sid = -1;
        m_eventSid = -1;
        break;
    default:
        break;
    }
}

MsChannelState PlaybackChannelInfo::state() const
{
    return m_state;
}

void PlaybackChannelInfo::setCommonBackup(resp_search_common_backup *common, int size)
{
    m_commonBackupList.clear();

    appendCommonBackup(common, size);
    m_channel = common->chnid;
}

void PlaybackChannelInfo::appendCommonBackup(resp_search_common_backup *common, int size)
{
    m_sid = common->sid;
    for (int i = 0; i < size; ++i) {
        resp_search_common_backup common_backup;
        memcpy(&common_backup, &(common[i]), sizeof(resp_search_common_backup));
        m_commonBackupList.append(common_backup);
    }
    m_channel = common->chnid;
}

void PlaybackChannelInfo::appendCommonBackup(const resp_search_common_backup &common)
{
    m_sid = common.sid;
    m_channel = common.chnid;
    m_commonBackupList.append(common);
}

int PlaybackChannelInfo::commonBackupSize() const
{
    return m_commonBackupList.size();
}

int PlaybackChannelInfo::nextCommonBackupPage() const
{
    int page = -1;
    if (!m_commonBackupList.isEmpty()) {
        page = m_commonBackupList.size() / 100 + 1;
    }
    return page;
}

QList<resp_search_common_backup> PlaybackChannelInfo::commonBackupList() const
{
    return m_commonBackupList;
}

bool PlaybackChannelInfo::hasCommonBackup() const
{
    if (m_commonBackupList.isEmpty()) {
        return false;
    } else {
        return true;
    }
}

void PlaybackChannelInfo::clearCommonBackup()
{
    m_channel = -1;
    m_sid = -1;
    m_commonBackupList.clear();
}

void PlaybackChannelInfo::setEventBackup(resp_search_event_backup *event, int size)
{
    clearEventBackup();
    appendEventBackup(event, size);
}

void PlaybackChannelInfo::appendEventBackup(resp_search_event_backup *event, int size)
{
    m_eventSid = event->sid;
    for (int i = 0; i < size; ++i) {
        resp_search_event_backup event_backup;
        memcpy(&event_backup, &(event[i]), sizeof(resp_search_event_backup));
        m_eventBackupList.append(event_backup);
    }
}

int PlaybackChannelInfo::eventBackupSize() const
{
    return m_eventBackupList.size();
}

int PlaybackChannelInfo::nextEventBackupPage() const
{
    int page = -1;
    if (!m_eventBackupList.isEmpty()) {
        page = m_eventBackupList.size() / 100 + 1;
    }
    return page;
}

int PlaybackChannelInfo::eventBackupPageCount() const
{
    int count = 0;
    if (!m_eventBackupList.isEmpty()) {
        count = qCeil(m_eventBackupList.first().allCnt / 100.0);
    }
    return count;
}

QList<resp_search_event_backup> PlaybackChannelInfo::eventBackupList() const
{
    return m_eventBackupList;
}

void PlaybackChannelInfo::clearEventBackup()
{
    m_eventSid = -1;
    m_eventBackupList.clear();
    m_eventSearchStateMap.clear();
}

void PlaybackChannelInfo::setEventBackup_smartSearch(resp_search_event_backup *event, int size)
{
    clearEventBackup_smartSearch();
    appendEventBackup_smartSearch(event, size);
}

void PlaybackChannelInfo::appendEventBackup_smartSearch(resp_search_event_backup *event, int size)
{
    m_eventSid_smartSearch = event->sid;
    for (int i = 0; i < size; ++i) {
        resp_search_event_backup event_backup;
        memcpy(&event_backup, &(event[i]), sizeof(resp_search_event_backup));
        m_eventBackupList_smartSearch.append(event_backup);
    }
}

int PlaybackChannelInfo::eventBackupSize_smartSearch() const
{
    return m_eventBackupList_smartSearch.size();
}

int PlaybackChannelInfo::nextEventBackupPage_smartSearch() const
{
    int page = -1;
    if (!m_eventBackupList_smartSearch.isEmpty()) {
        page = m_eventBackupList_smartSearch.size() / 100 + 1;
    }
    return page;
}

int PlaybackChannelInfo::eventBackupPageCount_smartSearch() const
{
    int count = 0;
    if (!m_eventBackupList_smartSearch.isEmpty()) {
        count = qCeil(m_eventBackupList_smartSearch.first().allCnt / 100.0);
    }
    return count;
}

QList<resp_search_event_backup> PlaybackChannelInfo::eventBackupList_smartSearch() const
{
    return m_eventBackupList_smartSearch;
}

void PlaybackChannelInfo::clearEventBackup_smartSearch()
{
    m_eventSid_smartSearch = -1;
    m_eventBackupList_smartSearch.clear();
}

void PlaybackChannelInfo::setEventSearchState(const QDate &date, const EventSearchState &state)
{
    m_eventSearchStateMap.insert(date, state);
}

bool PlaybackChannelInfo::hasSearchEventBackup(const QDate &date) const
{
    return m_eventSearchStateMap.value(date, EventSearchNone) != EventSearchNone;
}

void PlaybackChannelInfo::setCommonBackupLock(const resp_search_common_backup &backup, int isLock)
{
    for (int i = 0; i < m_commonBackupList.size(); ++i) {
        resp_search_common_backup &tempBackup = m_commonBackupList[i];
        if (QString(tempBackup.pStartTime) == QString(backup.pStartTime) && QString(tempBackup.pEndTime) == QString(backup.pEndTime)) {
            tempBackup.isLock = isLock;
        }
    }
}
