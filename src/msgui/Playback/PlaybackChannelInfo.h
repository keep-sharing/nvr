#ifndef PLAYBACKCHANNELINFO_H
#define PLAYBACKCHANNELINFO_H

#include <QDate>
#include <QList>
#include <QMap>

extern "C" {
#include "msg.h"
}

enum MsChannelState {
    ChannelStoped, //未开始或者已结束回放
    ChannelSearch, //正在搜索录像
    ChannelStartting, //播放请求已发送，还未返回
    ChannelStarted, //播放结果已返回
    ChannelStopping, //停止播放已发送，还未返回
    ChannelUnknown
};

enum EventSearchState {
    EventSearchNone,
    EventSearching,
    EventSearchEnd
};

class PlaybackChannelInfo {
public:
    PlaybackChannelInfo();
    PlaybackChannelInfo(int channel);

    bool isValid() const;

    int channel() const;
    int sid() const;
    int eventSid() const;
    int eventSid_smartSearch() const;

    void setState(MsChannelState state);
    MsChannelState state() const;

    void setCommonBackup(resp_search_common_backup *common, int size);
    void appendCommonBackup(resp_search_common_backup *common, int size);
    void appendCommonBackup(const resp_search_common_backup &common);
    int commonBackupSize() const;
    int nextCommonBackupPage() const;
    QList<resp_search_common_backup> commonBackupList() const;
    bool hasCommonBackup() const;
    void clearCommonBackup();

    void setEventBackup(resp_search_event_backup *event, int size);
    void appendEventBackup(resp_search_event_backup *event, int size);
    int eventBackupSize() const;
    int nextEventBackupPage() const;
    int eventBackupPageCount() const;
    QList<resp_search_event_backup> eventBackupList() const;
    void clearEventBackup();

    //smart search
    void setEventBackup_smartSearch(resp_search_event_backup *event, int size);
    void appendEventBackup_smartSearch(resp_search_event_backup *event, int size);
    int eventBackupSize_smartSearch() const;
    int nextEventBackupPage_smartSearch() const;
    int eventBackupPageCount_smartSearch() const;
    QList<resp_search_event_backup> eventBackupList_smartSearch() const;
    void clearEventBackup_smartSearch();

    //
    void setEventSearchState(const QDate &date, const EventSearchState &state);
    bool hasSearchEventBackup(const QDate &date) const;

    void setCommonBackupLock(const resp_search_common_backup &backup, int isLock);

private:
    int m_channel = -1;
    int m_sid = -1;
    int m_eventSid = -1;
    int m_eventSid_smartSearch = -1;
    MsChannelState m_state = ChannelStoped;
    QList<resp_search_common_backup> m_commonBackupList;
    QList<resp_search_event_backup> m_eventBackupList;
    QMap<QDate, EventSearchState> m_eventSearchStateMap;
    //smart search
    QList<resp_search_event_backup> m_eventBackupList_smartSearch;
};

#endif // PLAYBACKCHANNELINFO_H
