#ifndef PLAYBACKSPLIT_H
#define PLAYBACKSPLIT_H

#include "BasePlayback.h"
#include "networkcommond.h"
#include "PlaybackSplitInfo.h"
#include <QEventLoop>

namespace Ui {
class PlaybackSplit;
}

class PlaybackSplit : public BasePlayback {
    Q_OBJECT

    enum ColumnType {
        ColumnChannel,
        ColumnName
    };

public:
    explicit PlaybackSplit(QWidget *parent = nullptr);
    ~PlaybackSplit();

    static PlaybackSplit *instance();

    void initializeData();
    void closePlayback();

    void setFocus();
    bool hasFocus();
    bool focusPrevious();
    bool focusNext();
    NetworkResult dealNetworkCommond(const QString &commond);

    //
    MsSplitLayoutMode layoutMode();
    void setLayoutMode(const MsSplitLayoutMode &mode);
    int channel();
    bool isPlaying();
    bool hasCommonBackup();
    bool hasCommonBackup(int sid);
    int playingChannel();
    void setSelectedSid(int sid);
    int selectedSid();
    void makeSelectedSidMask(char *mask, int size);
    void makeSidMask(char *mask, int size);

    resp_search_common_backup findCommonBackup(int sid, const QDateTime &dateTime);
    QList<resp_search_common_backup> splitCommonBackupList(int sid);
    QList<resp_search_common_backup> splitCommonBackupList();

    void startSplitPlayback();
    void stopSplitPlayback();
    void pauseSplitPlayback();
    void resumeSplitPlayback();
    void speedSplitPlayback(int speed);
    void backwardSplitPlayback();
    void forwardSplitPlayback();
    void stepSplitPlayback();
    void clearSplitCommonBackup();

    //audio
    void openAudio();
    void closeAudio();
    bool isSelectedAudioOpen();
    bool isAudioOpen();

    void dealMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_RANGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_OPEN(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_START_SPLIT_PLAYBACK(MessageReceive *message);
    void ON_RESPONSE_FLAG_STOP_SPLIT_PLAYBACK(MessageReceive *message);

    void selectNextChannel();

    QString splitSidMask();

    void searchSplitCommonPlaybackPage(int sid);

    void searchSplitPlayback(int channel);

private slots:
    void onLanguageChanged();
    void onChannelClicked(int row, int column);
    void onTreeViewEnterPressed();

    void on_pushButton_search_clicked();

private:
    static PlaybackSplit *s_self;

    Ui::PlaybackSplit *ui;

    QEventLoop m_eventloop;

    MsSplitLayoutMode m_layoutMode;
    int m_channel = 0;

    int m_tempSearchSid = 0;
    QDateTime m_searchStartDateTime;
    QDateTime m_searchEndDateTime;
    bool m_isPlaying = false;

    QDateTime m_ShowStartDateTime;
    QDateTime m_ShowEndDateTime;
    int m_selectedSid = 0;
    QMap<int, PlaybackSplitInfo> m_splitInfoMap;

    int m_audioSid = -1;
};

#endif // PLAYBACKSPLIT_H
