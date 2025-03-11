#include "PlaybackTagData.h"
#include "MsMessage.h"
#include "MyDebug.h"

PlaybackTagData::PlaybackTagData(QObject *parent)
    : MsObject(parent)
{
}

PlaybackTagData::~PlaybackTagData()
{
}

PlaybackTagData &PlaybackTagData::instance()
{
    static PlaybackTagData self;
    return self;
}

void PlaybackTagData::searchTag(const req_search_tags &tag)
{
    if (isListContains(m_searchList, tag)) {
        return;
    }
    if (isListContains(m_placeholderList, tag)) {
        return;
    }

    m_searchList.append(tag);

    if (!m_isSearching) {
        searchNext();
        m_isSearching = true;
    }
}

void PlaybackTagData::clear(int channel)
{
    QMutableListIterator<req_search_tags> searchIter(m_searchList);
    while (searchIter.hasNext()) {
        const req_search_tags &search = searchIter.next();
        if (search.chnMaskl[channel] == '1') {
            searchIter.remove();
        }
    }

    QMutableListIterator<req_search_tags> placeholderIter(m_placeholderList);
    while (placeholderIter.hasNext()) {
        const req_search_tags &search = placeholderIter.next();
        if (search.chnMaskl[channel] == '1') {
            placeholderIter.remove();
        }
    }

    m_tagMap.remove(channel);
}

void PlaybackTagData::clearAll()
{
    m_searchList.clear();
    m_placeholderList.clear();
    m_tagMap.clear();
}

void PlaybackTagData::removeTag(int channel, uint time)
{
    Q_UNUSED(time)

    auto &tags = m_tagMap[channel];
    QMutableListIterator<resp_search_tags> placeholderIter(tags);
    while (placeholderIter.hasNext()) {
        const resp_search_tags &tag = placeholderIter.next();
        if (QDateTime::fromString(QString(tag.pTime), "yyyy-MM-dd HH:mm:ss").toTime_t() == time) {
            placeholderIter.remove();
        }
    }

    emit tagChanged(channel);
}

void PlaybackTagData::addTag(int channel, uint time)
{
    auto &tags = m_tagMap[channel];
    resp_search_tags tag;
    tag.chnid = channel;
    snprintf(tag.pTime, sizeof(tag.pTime), "%s", QDateTime::fromTime_t(time).toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    tags.append(tag);

    emit tagChanged(channel);
}

bool PlaybackTagData::hasTag(int channel) const
{
    return m_tagMap.contains(channel);
}

QList<resp_search_tags> PlaybackTagData::tags(int channel) const
{
    return m_tagMap.value(channel);
}

QMap<QDateTime, int> PlaybackTagData::tagMap(int channel) const
{
    QMap<QDateTime, int> map;
    const auto &tags = m_tagMap.value(channel);
    for (int i = 0; i < tags.size(); ++i) {
        const resp_search_tags &tag = tags.at(i);
        map.insert(QDateTime::fromString(QString(tag.pTime), "yyyy-MM-dd HH:mm:ss"), 0);
    }
    return map;
}

void PlaybackTagData::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN:
        ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(message);
        break;
    case RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE:
        ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE(message);
        break;
    case RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_CLOSE:
        ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_CLOSE(message);
        break;
    }
}

void PlaybackTagData::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(MessageReceive *message)
{
    req_search_tags *search = static_cast<req_search_tags *>(message->obj->argument());
    if (!search) {
        qMsCritical() << "argument is nullptr.";
        closeTagSearch(message);
        return;
    }
    if (!isListContains(m_placeholderList, *search)) {
        closeTagSearch(message);
        delete search;
        return;
    }

    //
    dealTagSearch(message);
}

void PlaybackTagData::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE(MessageReceive *message)
{
    ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(message);
}

void PlaybackTagData::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_CLOSE(MessageReceive *message)
{
    Q_UNUSED(message);

    searchNext();
}

void PlaybackTagData::dealTagSearch(MessageReceive *message)
{
    struct resp_search_tags *tags_array = (struct resp_search_tags *)message->data;
    if (!tags_array) {
        qMsCritical() << message;
        searchNext();
        return;
    }

    int count = message->header.size / sizeof(struct resp_search_tags);
    auto &tagList = m_tagMap[tags_array->chnid];
    for (int i = 0; i < count; ++i) {
        const resp_search_tags &tag = tags_array[i];
        tagList.append(tag);
    }

    //是否还有分页
    req_search_tags *search = static_cast<req_search_tags *>(message->obj->argument());
    if (tagList.size() < tags_array->allCnt) {
        int page = tagList.size() / 100 + 1;
        struct rep_get_search_backup search_backup;
        memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
        search_backup.sid = tags_array->sid;
        search_backup.npage = page;
        qMsDebug() << QString("REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_PAGE, sid: %1, page: %2").arg(tags_array->sid).arg(page);
        MessageObject *obj = new MessageObject(this, REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_PAGE);
        obj->setArgument(search);
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_PAGE, &search_backup, sizeof(rep_get_search_backup), obj);
        return;
    }

    //
    delete search;
    closeTagSearch(message);

    //
    emit tagSearchFinished(tags_array->chnid);
}

void PlaybackTagData::closeTagSearch(MessageReceive *message)
{
    struct resp_search_tags *tags_array = (struct resp_search_tags *)message->data;
    if (tags_array) {
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_CLOSE, &tags_array->sid, sizeof(tags_array->sid), this);
    }
}

void PlaybackTagData::searchNext()
{
    if (m_searchList.isEmpty()) {
        m_isSearching = false;
        return;
    }

    req_search_tags search = m_searchList.takeFirst();
    m_placeholderList.append(search);

    qMsDebug() << QString("REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_OPEN, chnNum:%1, chnMaskl:%2, enType:%3, pStartTime:%4, pEndTime:%5")
                      .arg(search.chnNum)
                      .arg(search.chnMaskl)
                      .arg(search.enType)
                      .arg(search.pStartTime)
                      .arg(search.pEndTime);
    MessageObject *obj = new MessageObject(this, REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_OPEN);
    req_search_tags *arg = new req_search_tags;
    memcpy(arg, &search, sizeof(req_search_tags));
    obj->setArgument(arg);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_OPEN, &search, sizeof(search), obj);
}

bool PlaybackTagData::isListContains(const QList<req_search_tags> &list, const req_search_tags &tag)
{
    for (int i = 0; i < list.size(); ++i) {
        const req_search_tags &t = list.at(i);
        if (memcmp(&t, &tag, sizeof(req_search_tags)) == 0) {
            return true;
        }
    }
    return false;
}
