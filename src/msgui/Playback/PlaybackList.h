#ifndef PLAYBACKLIST_H
#define PLAYBACKLIST_H

#include "BasePlayback.h"
#include "itemtextdelegate.h"
#include <QDateTime>

namespace Ui {
class PlaybackList;
}

class PlaybackList : public BasePlayback {
    Q_OBJECT

protected:
    enum TreeColumn {
        ColumnChannel,
        ColumnTime,
        ColumnPlay
    };

    struct ItemKey {
        int row = -1;
        int page = -1;

        ItemKey()
        {
        }
        ItemKey(int r, int p)
        {
            row = r;
            page = p;
        }
        bool operator==(const ItemKey &other) const
        {
            return row == other.row && page == other.page;
        }
    };

public:
    explicit PlaybackList(QWidget *parent = 0);
    ~PlaybackList();

    //实际播放的起止时间
    static QDateTime s_beginDateTime;
    static QDateTime s_endDateTime;

    void initializeData();
    void dealMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_START_ALL_PLAYBACK(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message) override;

    void showEvent(QShowEvent *event) override;

    //tree
    virtual QStringList treeHeaders() = 0;
    virtual void initializeTreeView();
    virtual void updateTreeList();

    //
    virtual void playItem(int row);
    virtual void waitForSearchCommonPlayback(int channel, const QDateTime &begin, const QDateTime &end);
    virtual void waitForStartAllPlayback();

    void setItemTextColor(int row, const QString &color);
    void clearItemTextColor(int row);
    void clearAllItemTextColor();

signals:
    void sig_back();

protected slots:
    void onLanguageChanged();

    virtual void onItemClicked(const QModelIndex &index);
    virtual void onItemDoubleClicked(const QModelIndex &index);

    virtual void selectPrevious();
    virtual void selectNext();

    virtual void on_toolButton_firstPage_clicked();
    virtual void on_toolButton_previousPage_clicked();
    virtual void on_toolButton_nextPage_clicked();
    virtual void on_toolButton_lastPage_clicked();
    virtual void on_pushButton_go_clicked();

    virtual void on_comboBox_prePlayback_activated(int index);
    virtual void on_comboBox_postPlayback_activated(int index);

    virtual void on_pushButton_export_clicked();
    virtual void on_pushButton_exportAll_clicked();

    virtual void on_toolButton_close_clicked();

protected:
    Ui::PlaybackList *ui;

    int m_currentPlaySid = -1;

    int m_pageIndex = 0;
    int m_pageCount = 0;
    int m_allCount = 0;

    int m_currentRow = -1;
    int m_searchSid = -1;
    int m_currentChannel = -1;

    ItemKey m_currentItemKey;

    ItemTextDelegate *m_channelTextDelegate = nullptr;
    ItemTextDelegate *m_timeTextDelegate = nullptr;
};

#endif // PLAYBACKLIST_H
