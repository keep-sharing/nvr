#ifndef PLAYBACKBAR_H
#define PLAYBACKBAR_H

#include "AnimateToast.h"
#include "BasePlayback.h"
#include "PlaybackCut.h"
#include "PlaybackFileManagement.h"
#include "PlaybackZoom.h"
#include <QDate>
#include <QTimer>

namespace Ui {
class PlaybackBar;
}

class PlaybackBar : public BasePlayback {
    Q_OBJECT

public:
    enum EventSearchType {
        EventNone,
        EventAll,
        EventAudioAlarm,
        EventAlarm,
        EventVca,
        EventSmart
    };
    explicit PlaybackBar(QWidget *parent = 0);
    ~PlaybackBar();

    PlaybackTimeLine *timeline();

    void initializeData();
    void initializeTimeLine();

    void setCurrentTimeLine(int channel);
    void setCurrentTimeLine();

    void setCurrentDateTime(const QDateTime &dateTime);

    void updatePlaybackButtonState();
    void setPlaybackButtonState(MsPlaybackState state);

    void updateTimeLine();
    void updateSplitChannel();
    void updatePlaybackSpeedString();

    void closePlaybackCut();

    void switchScreen();

    bool isBestDecodeMode() const;
    bool isSeeking() const;

    bool isRealtimeActive();

    //是否显示POS信息
    bool isShowPosData() const;

    //
    QMap<int, QDateTime> splitDateTimeMap();
    QDateTime splitDateTime();
    void clearSplitDateTimeMap();

    //
    void waitForSearchGeneralEventPlayBack();

    //
    NetworkResult dealNetworkCommond(const QString &commond);

    void dealMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_START_ALL_PLAYBACK(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_GET_PLAYBACK_REALTIME(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SPLIT_PLAYBACK_TIME(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEEK_ALL_PLAYBACK(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEEK_SPLIT_PLAYBACK(MessageReceive *message);
    void ON_RESPONSE_FLAG_CHECK_AUDIOTALK(MessageReceive *message);

private:
    void setSnapshotButtonEnable(bool enable);
    void setLockButtonEnable(bool enable);
    void setQuickTagButtonEnable(bool enable);
    void setCustomTagButtonEnable(bool enable);
    void setDateTimeText(const QDateTime &dateTime);
    void setDateTimeText(const QString &text);

    void updateSmartSearchButtonState();
    void updateFisheyeButtonState();
    void updateZoomButtonState();
    void updateFilterButtonState();

    void waitForCheckNvrTalkSupport();

signals:
    void fisheyePanelButtonClicked(int x, int y);
    void smartSpeedPanelButtonClicked(int x, int y);
    void posClicked(bool show);
    void pauseClicked(bool pause);
    void filterEventPanelButtonClicked(int x, int y);

public slots:
    void onPlaybackModeChanged(MsPlaybackType mode);
    void onSelectedDateChanged(const QDate &date);
    void onFisheyePanelButtonStateChanged(int state);
    void onFisheyeDewarpControlClosed();
    void onSmartSpeedPanelButtonStateChanged(int state);
    void onZoomStateChanged(int state);

    void onChannelStarted(int channel);
    void onChannelStoped(int channel);

private slots:
    void onLanguageChanged();
    //
    void onPlaybackTimer();
    void onPlaybackRealTime(QDateTime dateTime);
    void onSplitPlaybackRealTime(int channel, QMap<int, QDateTime> dateTimeMap);
    //
    void onSetCurrentTimeLine(int channel);
    //录像跳转
    void onTimeLineClicked(const QDateTime &dateTime);

    //speed
    void onPlaybackSpeedChanged(int speed);
    //smart search
    void onSmartSearchModeChanged(int mode);
    //event
    void onEventSearchFinished(int channel);
    //tag
    void onTagSearchFinished(int channel);
    void onTagChanged(int channel);

    //
    void on_toolButton_stop_clicked();
    //
    void on_toolButton_play_clicked(bool checked);
    //
    void on_toolButton_rewind_clicked(bool checked);
    //
    void on_toolButton_stepForward_clicked();
    void on_toolButton_stepReverse_clicked();
    //
    void on_toolButton_speedUp_clicked();
    void on_toolButton_speedDown_clicked();

    void onSmartSearchClicked();
    void onFisheyeClicked();
    void showFisheyePanel();
    void onZoomClicked();
    void onSnapshotClicked();
    void onAudioClicked(bool checked);
    void setAudioEnable(bool checked);
    void onLockClicked();
    void onQuickTagClicked();
    void onCustomTagClicked();

    void on_toolButton_cut_clicked();
    void on_toolButtonLockAll_clicked();
    void on_toolButtonQuickTagAll_clicked();
    void on_toolButtonCustomTagAll_clicked();
    void on_toolButton_fileManagement_clicked();
    void on_toolButton_smartSpeed_clicked();
    void on_toolButton_bestDecoding_clicked(bool checked);
    void on_toolButtonPos_clicked(bool checked);

    void onSplitButtonGroupClicked(int id);

    void onRockerTimer();

    void on_toolButtonFilterEvent_clicked();
    void onToolButtionFilterEventSearch();

private:
    Ui::PlaybackBar *ui;

    QTimer *m_playbackTimer;
    bool m_isSeeking = false;

    AnimateToast *m_animateToast = nullptr;

    PlaybackFileManagement *m_fileManagement = nullptr;
    PlaybackCut *m_cut = nullptr;

    //
    QTimer *m_rockerTimer = nullptr;
    int m_rockerMode = 0;

    //
    QMap<int, QDateTime> m_splitDateTimeMap;

    //
    QEventLoop m_eventLoop;
    int m_nvrTalkState;
    EventSearchType m_eventSearchType;
};

#endif // PLAYBACKBAR_H
