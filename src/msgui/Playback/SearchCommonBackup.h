#ifndef SEARCHCOMMONBACKUP_H
#define SEARCHCOMMONBACKUP_H

#include "SearchAbstract.h"

extern "C" {
#include "msg.h"
}

class SearchCommonBackup : public SearchAbstract {
    Q_OBJECT

public:
    explicit SearchCommonBackup(QObject *parent = nullptr);

    void setSearchInfo(const req_search_common_backup &backup);
    void startSearch();

    bool same(const req_search_common_backup &backup) const;

    bool hasBackup() const;
    QList<resp_search_common_backup> backupList() const;

    void closeCommonBackup();

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_CLOSE(MessageReceive *message);

    void dealCommonBackup(MessageReceive *message);

private:
    req_search_common_backup m_req;
    QList<resp_search_common_backup> m_backupList;
};

#endif // SEARCHCOMMONBACKUP_H
