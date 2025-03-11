#ifndef PLAYBACKEVENTLIST_H
#define PLAYBACKEVENTLIST_H

#include "PlaybackList.h"
#include "networkcommond.h"

class PlaybackEventList : public PlaybackList {
    Q_OBJECT

public:
    explicit PlaybackEventList(QWidget *parent = nullptr);

    void closePlayback();

    NetworkResult dealNetworkCommond(const QString &commond);

    void dealMessage(MessageReceive *message) override;
    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(MessageReceive *message) override;

    QStringList treeHeaders() override;
    void updateTreeList() override;

private:
    void playItem(int row) override;

    void searchEventBackupPage(int page);

protected slots:
    void onItemClicked(const QModelIndex &index) override;
    void onItemDoubleClicked(const QModelIndex &index) override;

    void on_toolButton_firstPage_clicked() override;
    void on_toolButton_previousPage_clicked() override;
    void on_toolButton_nextPage_clicked() override;
    void on_toolButton_lastPage_clicked() override;
    void on_pushButton_go_clicked() override;

    void on_toolButton_close_clicked() override;

private:
    QList<resp_search_event_backup> m_backupList;
    QEventLoop m_eventLoop;
};

#endif // PLAYBACKEVENTLIST_H
