#ifndef PLAYBACKTAG_H
#define PLAYBACKTAG_H

#include "BasePlayback.h"
#include "progressdialog.h"
#include <QModelIndex>

class PlaybackTagList;
class ItemCheckedButtonDelegate;

namespace Ui {
class PlaybackTag;
}

class PlaybackTag : public BasePlayback {
    Q_OBJECT

public:
    explicit PlaybackTag(QWidget *parent = 0);
    ~PlaybackTag();

    void initializeData();
    void closePlayback();

    void setFocus();
    bool hasFocus();
    bool focusPrevious();
    bool focusNext();

    NetworkResult dealNetworkCommond(const QString &commond);

    void dealMessage(MessageReceive *message) override;
    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message);

    void keyPressEvent(QKeyEvent *event) override;
    void returnPressed();

private:
    void initializeChannelTree();
    void updateChannelName();

    void selectNextChannel();

private slots:
    void onLanguageChanged();

    void onItemClicked(const QModelIndex &index);
    void onItemClicked(int row, int column);
    void onTreeViewEnterPressed();

    //progress
    void onSearchCanceled();

    //
    void on_pushButton_search_clicked();

private:
    Ui::PlaybackTag *ui;

    PlaybackTagList *m_playList = nullptr;
    ItemCheckedButtonDelegate *m_columnCheckDelegate = nullptr;

    //progress
    ProgressDialog *m_progress = nullptr;
    int m_searchSid = -1;
    bool m_isSearching = false;
};

#endif // PLAYBACKTAG_H
