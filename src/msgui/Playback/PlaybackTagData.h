#ifndef PLAYBACKTAGDATA_H
#define PLAYBACKTAGDATA_H

#include <QMap>
#include <QDateTime>
#include "MsObject.h"

extern "C" {
#include "msg.h"
}

#define gPlaybackTagData PlaybackTagData::instance()

class PlaybackTagData : public MsObject
{
    Q_OBJECT
public:
    struct TagKey {
        explicit TagKey()
        {

        }
        explicit TagKey(int ch)
        {
            channel = ch;
        }
        explicit TagKey(const req_search_tags &search)
        {
            for (int i = 0; i < MAX_LEN_64; ++i) {
                if (search.chnMaskl[i] == '1') {
                    channel = i;
                }
            }
        }
        bool operator <(const TagKey &other) const
        {
            return channel < other.channel;
        }
        int channel = -1;
    };
    struct TagValue {
        explicit TagValue()
        {

        }
        explicit TagValue(int ch, uint t)
        {
            channel = ch;
            time = t;
        }
        explicit TagValue(const resp_search_tags &tag)
        {
            channel = tag.chnid;
            time = QDateTime::fromString(tag.pTime, "yyyy-MM-dd HH:mm:ss").toTime_t();
            name = tag.pName;
        }
        bool operator ==(const TagValue &other) const
        {
            return channel == other.channel && time == other.time;
        }
        int channel = -1;
        uint time = 0;
        QString name;
    };

    explicit PlaybackTagData(QObject *parent = nullptr);
    ~PlaybackTagData() override;

    static PlaybackTagData &instance();

    void searchTag(const req_search_tags &search);
    void clear(int channel);
    void clearAll();

    void removeTag(int channel, uint time);
    void addTag(int channel, uint time);

    bool hasTag(int channel) const;
    QList<resp_search_tags> tags(int channel) const;
    QMap<QDateTime, int> tagMap(int channel) const;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_CLOSE(MessageReceive *message);

    void dealTagSearch(MessageReceive *message);
    void closeTagSearch(MessageReceive *message);

    void searchNext();
    bool isListContains(const QList<req_search_tags> &list, const req_search_tags &tag);

signals:
    void tagSearchFinished(int channel);
    void tagChanged(int channel);

private:
    bool m_isSearching = false;
    //搜索队列
    QList<req_search_tags> m_searchList;
    //占位列表
    //搜索时，列表已经存在的不去搜
    //搜索完成时，列表中不存在的不接收
    QList<req_search_tags> m_placeholderList;
    //搜索结果
    QMap<int, QList<resp_search_tags>> m_tagMap;
};

#endif // PLAYBACKTAGDATA_H
