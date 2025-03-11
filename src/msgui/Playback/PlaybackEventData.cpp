#include "PlaybackEventData.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include <qmath.h>

PlaybackEventData::PlaybackEventData(QObject *parent)
    : MsObject(parent)
{
}

PlaybackEventData &PlaybackEventData::instance()
{
    static PlaybackEventData self;
    return self;
}

PlaybackEventData::State PlaybackEventData::searchEventBackup(const req_search_event_backup &backup)
{
    SearchEventBackup *search = nullptr;
    for (int i = 0; i < m_searchList.size(); ++i) {
        auto *s = m_searchList.at(i);
        if (s->same(backup)) {
            search = s;
            break;
        }
    }
    if (search) {
        if (search->isSearching()) {
            return StateSearching;
        } else if (search->isSearchFinished()) {
            return StateFinished;
        } else {
            return StateError;
        }
    } else {
        search = new SearchEventBackup(this);
        connect(search, SIGNAL(finished(int)), this, SLOT(onSearchFinished(int)));
        m_searchList.append(search);
    }
    search->setSearchInfo(backup);

    if (!isSearching()) {
        if (searchNext() < 0) {
            return StateError;
        }
    }

    return StateStart;
}

void PlaybackEventData::clear(int channel)
{
    QMutableListIterator<SearchEventBackup *> iter(m_searchList);
    while (iter.hasNext()) {
        auto *s = iter.next();
        if (s->channel() == channel) {
            iter.remove();
            delete s;
        }
    }
}

void PlaybackEventData::clearAll()
{
    QMutableListIterator<SearchEventBackup *> iter(m_searchList);
    while (iter.hasNext()) {
        auto *s = iter.next();
        iter.remove();
        delete s;
    }
}

bool PlaybackEventData::isSearching() const
{
    for (int i = 0; i < m_searchList.size(); ++i) {
        auto *s = m_searchList.at(i);
        if (s->isSearching()) {
            return true;
        }
    }

    return false;
}

bool PlaybackEventData::hasEventBackup(int channel, uint searchEvent)
{
    QMutableListIterator<SearchEventBackup *> iter(m_searchList);
    while (iter.hasNext()) {
        auto *s = iter.next();
        if (s->channel() == channel && s->searchEvent() == searchEvent) {
            return s->hasBackup();
        }
    }
    return false;
}

QList<resp_search_event_backup> PlaybackEventData::backupList(int channel, uint searchEvent)
{
    QMutableListIterator<SearchEventBackup *> iter(m_searchList);
    while (iter.hasNext()) {
        auto *s = iter.next();
        if (s->channel() == channel && s->searchEvent() == searchEvent) {
            return s->backupList();
        }
    }
    return QList<resp_search_event_backup>();
}

int PlaybackEventData::searchNext()
{
    if (m_searchList.isEmpty()) {
        return -1;
    }

    QMutableListIterator<SearchEventBackup *> iter(m_searchList);
    while (iter.hasNext()) {
        auto *s = iter.next();
        if (s->isSearching()) {
            continue;
        } else if (s->isSearchFinished()) {
            continue;
        } else {
            s->startSearch();
            return 0;
        }
    }

    return -1;
}

void PlaybackEventData::onSearchFinished(int channel)
{
    emit searchFinished(channel);

    searchNext();
}
