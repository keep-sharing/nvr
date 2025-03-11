#ifndef PLAYBACKTAGLIST_H
#define PLAYBACKTAGLIST_H

#include "PlaybackList.h"
#include "networkcommond.h"

class PlaybackTagList : public PlaybackList {
    Q_OBJECT

protected:
    enum TreeColumn {
        ColumnChannel,
        ColumnName,
        ColumnPlay
    };

public:
    explicit PlaybackTagList(QWidget *parent = nullptr);

    void closePlayback();

    NetworkResult dealNetworkCommond(const QString &commond);

    void dealMessage(MessageReceive *message) override;
    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE(MessageReceive *message);

    QStringList treeHeaders() override;
    void updateTreeList() override;

private:
    void playItem(int row) override;

    void searchTagBackupPage(int page);

signals:

public slots:

protected slots:
    virtual void onItemClicked(const QModelIndex &index);
    virtual void onItemDoubleClicked(const QModelIndex &index);

    virtual void on_toolButton_firstPage_clicked();
    virtual void on_toolButton_previousPage_clicked();
    virtual void on_toolButton_nextPage_clicked();
    virtual void on_toolButton_lastPage_clicked();
    virtual void on_pushButton_go_clicked();

    virtual void on_toolButton_close_clicked();

private:
    QList<resp_search_tags> m_tagList;
};

#endif // PLAYBACKTAGLIST_H
