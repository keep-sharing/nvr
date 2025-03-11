#ifndef PLAYBACKEVENT_H
#define PLAYBACKEVENT_H

#include "AlarmKey.h"
#include "BasePlayback.h"
#include "progressdialog.h"
#include <QModelIndex>

extern "C" {
#include "msdb.h"
}

class PlaybackEventList;
class ItemCheckedButtonDelegate;

namespace Ui {
class PlaybackEvent;
}

class PlaybackEvent : public BasePlayback {
    Q_OBJECT

    enum TreeType {
        NoneTree,
        ChannelTree,
        AlarmTree
    };

public:
    explicit PlaybackEvent(QWidget *parent = 0);
    ~PlaybackEvent();

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
    void ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message);

    void keyPressEvent(QKeyEvent *event) override;
    void returnPressed();

private:
    void initializeChannelTree();
    void initializeAlarminTree();
    void updateCheckedChannel();

    void selectNextChannel();

    void updateSubType();

private slots:
    void onLanguageChanged();

    void onItemClicked(const QModelIndex &index);
    void onItemClicked(int row, int column);
    void onTreeViewEnterPressed();

    //progress
    void onSearchCanceled();

    //
    void on_comboBox_mainType_activated(int index);
    void on_comboBox_mainType_indexSet(int index);
    void on_comboBoxSubType_indexSet(int index);
    void on_comboBoxDetectionObject_indexSet(int index);
    void on_pushButton_search_clicked();

private:
    Ui::PlaybackEvent *ui;

    PlaybackEventList *m_playList = nullptr;
    TreeType m_treeType = NoneTree;
    ItemCheckedButtonDelegate *m_columnCheckDelegate = nullptr;

    //progress
    ProgressDialog *m_progress = nullptr;
    int m_searchSid = -1;
    bool m_isSearching = false;

    //alarmin
    QList<alarm_in> m_alarminList;

    int m_mainType = INFO_MAJOR_MOTION;
    QList<AlarmKey> m_alarmInputMask;
    QList<int> m_channelMask;
};

#endif // PLAYBACKEVENT_H
