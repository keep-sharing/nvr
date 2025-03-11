#ifndef PLAYBACKEVENTDATA_H
#define PLAYBACKEVENTDATA_H

#include "MsObject.h"
#include "SearchEventBackup.h"
#include <QDateTime>
#include <QMap>

extern "C" {
#include "msg.h"
}

#define gPlaybackEventData PlaybackEventData::instance()

class PlaybackEventData : public MsObject
{
    Q_OBJECT

public:
    enum State {
        StateError,
        StateStart,
        StateSearching,
        StateFinished
    };

    explicit PlaybackEventData(QObject *parent = nullptr);

    static PlaybackEventData &instance();

    State searchEventBackup(const req_search_event_backup &backup);

    void clear(int channel);
    void clearAll();

    bool isSearching() const;

    bool hasEventBackup(int channel, uint searchEvent);
    QList<resp_search_event_backup> backupList(int channel, uint searchEvent);

private:
    int searchNext();

signals:
    void searchFinished(int channel);

private slots:
    void onSearchFinished(int channel);

private:
    QList<SearchEventBackup *> m_searchList;
};

#endif // PLAYBACKEVENTDATA_H
