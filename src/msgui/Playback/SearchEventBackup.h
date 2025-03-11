#ifndef SEARCHEVENTBACKUP_H
#define SEARCHEVENTBACKUP_H

#include "SearchAbstract.h"

extern "C" {
#include "msg.h"
}

class SearchEventBackup : public SearchAbstract {
    Q_OBJECT

public:
    explicit SearchEventBackup(QObject *parent = nullptr);

    void setSearchInfo(const req_search_event_backup &backup);
    void startSearch();

    bool same(const req_search_event_backup &backup) const;

    bool hasBackup() const;
    uint searchEvent() const;
    QList<resp_search_event_backup> backupList() const;
    void clearSearch();

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP_CLOSE(MessageReceive *message);

    void dealEventBackup(MessageReceive *message);
    void closeEventBackup(MessageReceive *message);

private:
    req_search_event_backup m_req;
    QList<resp_search_event_backup> m_backupList;
    uint m_searchEvent;
};

#endif // SEARCHEVENTBACKUP_H
