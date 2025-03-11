#include "SearchCommonBackup.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include <qmath.h>

SearchCommonBackup::SearchCommonBackup(QObject *parent)
    : SearchAbstract(parent)
{
}

void SearchCommonBackup::setSearchInfo(const req_search_common_backup &backup)
{
    m_backupList.clear();

    memcpy(&m_req, &backup, sizeof(m_req));

    m_channel = -1;
    int size = sizeof(backup.chnMaskl);
    for (int i = 0; i < size; ++i) {
        if (backup.chnMaskl[i] == '1') {
            m_channel = i;
            break;
        }
    }
}

void SearchCommonBackup::startSearch()
{
    sendMessage(REQUEST_FLAG_SEARCH_COM_PLAYBACK_OPEN, &m_req, sizeof(req_search_common_backup));

    m_state = StateSearching;
}

bool SearchCommonBackup::same(const req_search_common_backup &backup) const
{
    return memcmp(&m_req, &backup, sizeof(m_req)) == 0;
}

bool SearchCommonBackup::hasBackup() const
{
    return !m_backupList.isEmpty();
}

QList<resp_search_common_backup> SearchCommonBackup::backupList() const
{
    return m_backupList;
}

void SearchCommonBackup::closeCommonBackup()
{
    if (m_backupList.isEmpty()) {
        return;
    }
    int sid = m_backupList.first().sid;
    sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP_CLOSE, &sid, sizeof(int));
}

void SearchCommonBackup::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN:
        ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(message);
        break;
    case RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE:
        ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE(message);
        break;
    case RESPONSE_FLAG_SEARCH_COM_PLAYBACK_CLOSE:
        ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_CLOSE(message);
        break;
    }
}

void SearchCommonBackup::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(MessageReceive *message)
{
    dealCommonBackup(message);
}

void SearchCommonBackup::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE(MessageReceive *message)
{
    dealCommonBackup(message);
}

void SearchCommonBackup::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_CLOSE(MessageReceive *message)
{
    Q_UNUSED(message);
}

void SearchCommonBackup::dealCommonBackup(MessageReceive *message)
{
    auto *backup_array = (struct resp_search_common_backup *)message->data;
    int count          = message->header.size / sizeof(struct resp_search_common_backup);

    do {
        if (!backup_array) {
            break;
        }
        if (backup_array->allCnt < 1) {
          break;
        }

        if (backup_array->chnid != m_channel) {
            qMsCritical() << QString("chnid(%1) != m_channel(%2)").arg(backup_array->chnid).arg(m_channel);
            break;
        }

        for (int i = 0; i < count; ++i) {
            const resp_search_common_backup &backup = backup_array[i];
            m_backupList.append(backup);
        }
        if (m_backupList.size() < backup_array->allCnt) {
            //继续搜索
            int page      = m_backupList.size() / 100 + 1;
            int pageCount = qCeil(backup_array->allCnt / 100.0);
            struct rep_get_search_backup search_backup;
            memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
            search_backup.sid   = backup_array->sid;
            search_backup.npage = page;
            qMsDebug() << QString("REQUEST_FLAG_SEARCH_COM_PLAYBACK_PAGE, channel: %1, sid: %2, page: %3/%4")
                              .arg(backup_array->chnid)
                              .arg(backup_array->sid)
                              .arg(search_backup.npage)
                              .arg(pageCount);
            sendMessage(REQUEST_FLAG_SEARCH_COM_PLAYBACK_PAGE, &search_backup, sizeof(struct rep_get_search_backup));
            return;
        }
        qMsDebug() << QString("search_common_backup finished, channel:%1, sid:%2, count:%3").arg(backup_array->chnid).arg(backup_array->sid).arg(backup_array->allCnt);
    } while (0);

    //
    m_state = StateFinished;
    emit finished(m_channel);
}
