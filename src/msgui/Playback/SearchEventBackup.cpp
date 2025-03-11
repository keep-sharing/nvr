#include "SearchEventBackup.h"
#include "MsDevice.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include <qmath.h>

SearchEventBackup::SearchEventBackup(QObject *parent)
    : SearchAbstract(parent)
{
    memset(&m_req, 0, sizeof(m_req));
}

void SearchEventBackup::setSearchInfo(const req_search_event_backup &backup)
{
    m_backupList.clear();

    memcpy(&m_req, &backup, sizeof(m_req));

    m_channel = -1;
    m_searchEvent = backup.enMajor;
    int size = sizeof(backup.chnMaskl);
    for (int i = 0; i < size; ++i) {
        if (backup.chnMaskl[i] == '1') {
            m_channel = i;
            break;
        }
    }
}

void SearchEventBackup::startSearch()
{
    struct ms_socket_packet ms_packet;
    memset(&ms_packet, 0, sizeof(struct ms_socket_packet));
    ms_packet.nSize = sizeof(struct req_search_event_backup);
    ms_packet.nHeaderSize = ms_packet.nSize;
    ms_packet.nBodySize = 0;
    ms_packet.packet = (char *)ms_malloc(ms_packet.nSize);
    ms_packet.packetHeader = ms_packet.packet;
    ms_packet.packetBody = NULL;

    memcpy(ms_packet.packet, &m_req, sizeof(struct req_search_event_backup));
    qMsDebug() << "\n----REQUEST_FLAG_SEARCH_EVT_BACKUP----"
               << "\n----chnMaskl:" << m_req.chnMaskl
               << "\n----chnNum:" << m_req.chnNum
               << "\n----enType:" << m_req.enType
               << "\n----enMajor:" << m_req.enMajor
               << "\n----enMinor:" << m_req.enMinor
               << "\n----object:" << m_req.objtype
               << "\n----ahead:" << m_req.ahead
               << "\n----delay:" << m_req.delay
               << "\n----pStartTime:" << m_req.pStartTime
               << "\n----pEndTime:" << m_req.pEndTime;
    sendMessage(REQUEST_FLAG_SEARCH_EVT_BACKUP, (void *)ms_packet.packet, ms_packet.nSize);

    if (ms_packet.packet) {
        ms_free(ms_packet.packet);
    }
    m_state = StateSearching;
}

bool SearchEventBackup::same(const req_search_event_backup &backup) const
{
    return memcmp(&m_req, &backup, sizeof(m_req)) == 0;
}

bool SearchEventBackup::hasBackup() const
{
    return !m_backupList.isEmpty();
}

uint SearchEventBackup::searchEvent() const
{
    return m_searchEvent;
}

QList<resp_search_event_backup> SearchEventBackup::backupList() const
{
    return m_backupList;
}

void SearchEventBackup::clearSearch()
{
    m_backupList.clear();
}

void SearchEventBackup::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_EVT_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(message);
        break;
    case RESPONSE_FLAG_GET_SEARCH_EVT_PAGE:
        ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(message);
        break;
    case RESPONSE_FLAG_SEARCH_EVT_BACKUP_CLOSE:
        ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP_CLOSE(message);
        break;
    }
}

void SearchEventBackup::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message)
{
    dealEventBackup(message);
}

void SearchEventBackup::ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(MessageReceive *message)
{
    dealEventBackup(message);
}

void SearchEventBackup::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP_CLOSE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void SearchEventBackup::dealEventBackup(MessageReceive *message)
{
    auto *backup_array = (struct resp_search_event_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_event_backup);

    int retChannel = -1;
    int retSid = -1;
    int retCount = 0;

    do {
        if (!backup_array) {
            break;
        }

        if (backup_array->chnid != m_channel) {
            qMsCritical() << QString("chnid(%1) != m_channel(%2)").arg(backup_array->chnid).arg(m_channel);
            break;
        }

        for (int i = 0; i < count; ++i) {
            resp_search_event_backup &backup = backup_array[i];
            m_backupList.append(backup);
        }
        if (m_backupList.size() < backup_array->allCnt) {
            //继续搜索
            int page = m_backupList.size() / 100 + 1;
            int pageCount = qCeil(backup_array->allCnt / 100.0);
            struct rep_get_search_backup search_backup;
            memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
            search_backup.sid = backup_array->sid;
            search_backup.npage = page;
            qMsDebug() << QString("REQUEST_FLAG_GET_SEARCH_EVT_PAGE, channel: %1, sid: %2, page: %3/%4")
                              .arg(backup_array->chnid)
                              .arg(backup_array->sid)
                              .arg(search_backup.npage)
                              .arg(pageCount);
            sendMessage(REQUEST_FLAG_GET_SEARCH_EVT_PAGE, &search_backup, sizeof(struct rep_get_search_backup));
            return;
        }
        retChannel = backup_array->chnid;
        retSid = backup_array->sid;
        retCount = backup_array->allCnt;
    } while (0);

    qMsDebug() << QString("search_event_backup finished, channel:%1, sid:%2, count:%3").arg(retChannel).arg(retSid).arg(retCount);

    //搜索完直接关闭，释放sid资源
    //closeEventBackup(message);

    m_state = StateFinished;
    emit finished(m_channel);
}

void SearchEventBackup::closeEventBackup(MessageReceive *message)
{
    struct resp_search_event_backup *event_backup_array = (struct resp_search_event_backup *)message->data;
    if (event_backup_array) {
        qMsDebug() << QString("REQUEST_FLAG_SEARCH_EVT_BACKUP_CLOSE, channel: %1, sid: %2")
                          .arg(event_backup_array->chnid)
                          .arg(event_backup_array->sid);
        sendMessage(REQUEST_FLAG_SEARCH_EVT_BACKUP_CLOSE, &event_backup_array->sid, sizeof(int));
    }
}
