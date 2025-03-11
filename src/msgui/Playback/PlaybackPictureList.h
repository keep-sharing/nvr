#ifndef PLAYBACKPICTURELIST_H
#define PLAYBACKPICTURELIST_H

#include "PlaybackList.h"
#include "networkcommond.h"

class PicturePlay;

class PlaybackPictureList : public PlaybackList {
    Q_OBJECT

protected:
    enum TreeColumn {
        ColumnCheck,
        ColumnChannel,
        ColumnTime,
        ColumnPlay
    };

public:
    explicit PlaybackPictureList(QWidget *parent = nullptr);
    ~PlaybackPictureList() override;

    static PlaybackPictureList *instance();

    void initializeData();
    void closePlayback();

    bool playPrevious();
    bool playNext();

    NetworkResult dealNetworkCommond(const QString &commond);

    void dealMessage(MessageReceive *message) override;
    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_SEARCH_PIC_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SEARCH_PIC_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_PIC_BACKUP(MessageReceive *message);

    QStringList treeHeaders() override;
    void initializeTreeView() override;
    void updateTreeList() override;

private:
    void playItem(int row);
    void searchPicturePage(int page);

    void setItemTextColor(int row, const QString &color);
    void clearAllItemTextColor();

protected slots:
    virtual void onItemClicked(const QModelIndex &index);
    virtual void onItemDoubleClicked(const QModelIndex &index);

    virtual void on_toolButton_firstPage_clicked();
    virtual void on_toolButton_previousPage_clicked();
    virtual void on_toolButton_nextPage_clicked();
    virtual void on_toolButton_lastPage_clicked();
    virtual void on_pushButton_go_clicked();

    virtual void on_pushButton_export_clicked();
    virtual void on_pushButton_exportAll_clicked();

    virtual void on_toolButton_close_clicked();

private:
    static PlaybackPictureList *s_playbackPictureList;

    QList<resp_search_picture_backup> m_backupList;
};

#endif // PLAYBACKPICTURELIST_H
