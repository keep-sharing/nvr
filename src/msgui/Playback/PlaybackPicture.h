#ifndef PLAYBACKPICTURE_H
#define PLAYBACKPICTURE_H

#include "BasePlayback.h"
#include "progressdialog.h"
#include <QModelIndex>

class PlaybackPictureList;
class ItemCheckedButtonDelegate;

namespace Ui {
class PlaybackPicture;
}

class PlaybackPicture : public BasePlayback {
    Q_OBJECT

public:
    explicit PlaybackPicture(QWidget *parent = 0);
    ~PlaybackPicture();

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
    void ON_RESPONSE_FLAG_SEARCH_PIC_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message);

    void keyPressEvent(QKeyEvent *event) override;
    void returnPressed();

private:
    void initializeChannelTree();
    //    void initializeAlarminTree();
    //    void updateChannelName();

    //    void updateCheckedChannel();
    void selectNextChannel();
    //    void tertiaryTypeInit(int index);
private slots:
    void onLanguageChanged();

    void onItemClicked(const QModelIndex &index);
    void onItemClicked(int row, int column);
    void onTreeViewEnterPressed();

    //progress
    void onSearchCanceled();

    //
    void on_pushButton_search_clicked();

    //    void on_comboBox_subType_activated(int index);
    void on_comboBox_mainType_activated(int index);

private:
    Ui::PlaybackPicture *ui;

    PlaybackPictureList *m_playList = nullptr;
    ItemCheckedButtonDelegate *m_columnCheckDelegate = nullptr;

    //progress
    ProgressDialog *m_progress = nullptr;
    int m_searchSid = -1;
    bool m_isSearching = false;

    //    struct osd m_osdList[MAX_CAMERA];
    //    TreeType m_treeType = NoneTree;
    //    int m_subType = -2;
    //    long long m_alarmInputMask = 0;
    //    long long m_channelMask = 0;
};

#endif // PLAYBACKPICTURE_H
