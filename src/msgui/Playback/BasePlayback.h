#ifndef BASEPLAYBACK_H
#define BASEPLAYBACK_H

#include "MsWidget.h"
#include "PlaybackChannelInfo.h"
#include "PlaybackSplitInfo.h"
#include <QDateTime>
#include <QEventLoop>
#include <QMetaType>

extern "C" {
#include "msfs_pb.h"
#include "msg.h"
}

class PlaybackMode;
class PlaybackGeneral;
class PlaybackLayout;
class PlaybackBar;
class PlaybackCut;
class PlaybackTimeLine;
class ThumbWidget;
class FisheyeDewarpControl;
class SmartSpeedPanel;
class SmartSearchControl;
class MessageReceive;
class MsWaitting;
class FilterEventPanel;

Q_DECLARE_METATYPE(resp_snapshot_state)
Q_DECLARE_METATYPE(resp_search_common_backup)
Q_DECLARE_METATYPE(resp_search_tags)
Q_DECLARE_METATYPE(resp_search_picture_backup)

enum MsPlaybackError {
    PlaybackError_NoError,
    PlaybackError_NullBackup,
    PlaybackError_EmptyBackup
};

enum MsPlaybackType {
    GeneralPlayback,
    EventPlayback,
    TagPlayback,
    SplitPlayback,
    PicturePlayback
};

enum MsPlaybackState {
    PlaybackState_None,
    PlaybackState_Stop,
    PlaybackState_Play,
    PlaybackState_Pause,
    PlaybackState_Reverse,
    PlaybackState_ReverseStep,
    PlaybackState_Forward,
    PlaybackState_ForwardStep
};

enum MsPlayState {
    MsStopedState,
    MsPlayingState,
    MsStoppingState
};

enum MsSplitLayoutMode {
    SplitLayout_0 = 0,
    SplitLayout_4 = 4,
    SplitLayout_9 = 9,
    SplitLayout_16 = 16
};

class BasePlayback : public MsWidget {
    Q_OBJECT
public:
    explicit BasePlayback(QWidget *parent = nullptr);

    //
    static PlaybackMode *s_playbackMode;
    static PlaybackGeneral *s_playbackGeneral;
    static PlaybackLayout *s_playbackLayout;
    static PlaybackBar *s_playbackBar;
    static PlaybackTimeLine *s_playbackTimeLine;
    static ThumbWidget *s_thumbWidget;
    static SmartSpeedPanel *s_smartSpeed;
    static SmartSearchControl *s_smartSearch;
    static FilterEventPanel *s_filterEventPanel;

#ifdef MS_FISHEYE_SOFT_DEWARP
    static FisheyeDewarpControl *s_fisheyeControl;
#endif

    //
    static QString playbackStateString(const MsPlaybackState &state);
    static QString playbackSpeedString(PLAY_SPEED speed);

    //
    static void initializeBasePlayback(QWidget *parent);
    static void clearupBasePlayback();

    //ChannelList
    static void setSelectedChannel(int channel);

    //布局
    static void updateLayout(int channel);
    static void setCurrentVideoInLayout(int channel);

    //TimeLine
    static void initializeTimeLine();
    static void setCurrentTimeLine(int channel);
    static void setCurrentTimeLine();
    static void updateTimeLine();
    static void setTimeLineStartDateTime(const QDateTime &dateTime);

    //smart search
    static bool isSmartSearchMode();

    //获取录像日历
    static void getMonthEvent();

    //搜索录像
    static void searchCommonPlayback(int channel);
    static void searchCommonPlaybackPage(int channel);
    static void waitForSearchCommonPlayback(int channel);

    //关闭录像
    static void closeCommonPlayback(int channel);
    static void closeAllCommonPlayback();

    //搜索事件录像，当前只搜索移动监测事件
    static void searchEventPlayback(int channel);
    static void searchEventPlaybackPage(int channel);
    static void waitForSearchEventPlayback(int channel);

    //smart search
    static void searchEventPlayback_smartSearch(int channel, char *region);
    static void searchEventPlaybackPage_smartSearch(int channel);
    static void waitForSearchEventPlayback_smartSearch(int channel, char *region);

    //搜索tag，用于在录像信息上绘制
    static void searchTagPlayback(int channel);

    //关闭事件录像
    static void closeEventPlayback(int channel);
    static void closeAllEventPlayback();

    //开始播放
    static void startPlayback(int channel);
    static void startAllPlayback();
    static void waitForStartPlayback(int channel);
    static void waitForStartAllPlayback();

    //跳转
    static void seekPlayback();
    static void seekPlayback(qint64 secs);

    //停止播放
    static void stopPlayback(int channel);
    static void stopAllPlayback();
    static void waitForStopPlayback(int channel);
    static void waitForStopAllPlayback();

    //恢复
    static void resumeAllPlayback();
    static void waitForResumeAllPlayback();
    static void restartAllPlayback();
    static void waitForRestartAllPlayback();

    //暂停
    static void pauseAllPlayback();

    //倒放
    static void backwardAllPlayback();

    //快放、慢放
    static void speedUpAllPlayback();
    static void speedDownAllPlayback();

    //单帧
    static void stepForwardAllPlayback();
    static void stepBackwardAllPlayback();

    //音频
    static void openPlaybackAudio(int channel);
    static void closePlaybackAudio();
    static int audioChannel();

    //
    static void makeChannelMask(QList<int> list, char *mask, int size);
    static void makeChannelMask(int channel, char *mask, int size);
    static void makeChannelMask(char *mask, int size);

    //wait dialog
    static bool isWaitting();
    static void showWait();
    static void closeWait();
    //wait
    static void execWait();
    static void exitWait();

    //常规回放，事件回放，标签回放，图片回放
    static MsPlaybackType playbackType();
    static void setPlaybackType(MsPlaybackType mode);

    //主码流、次码流
    static FILE_TYPE_EN playbackStream();
    static void setPlaybackStream(FILE_TYPE_EN stream);

    //回放速度
    static PLAY_SPEED playbackSpeed();
    static void setPlaybackSpeed(PLAY_SPEED speed);
    static QString playbackSpeedString();

    //
    static void seekPlayback(const QDateTime &dateTime);

    //回放方向
    static PB_ACTION_EN playbackDirection();
    static void setPlaybackDirection(PB_ACTION_EN direction);

    //回放日期、时间
    static QDate playbackDate();
    static void setPlaybackDate(const QDate &date);
    static QTime playbackTime();
    static void setPlaybackTime(const QTime &time);
    static QDateTime playbackDateTime();

    //
    static QList<resp_search_common_backup> commonBackupList(int channel);
    static QDateTime commonBackupEndDateTime(int channel);
    static QList<resp_search_event_backup> eventBackupList(int channel);
    static QList<resp_search_event_backup> eventBackupList_smartSearch(int channel);
    static bool isChannelHasCommonBackup(int channel);
    static resp_search_common_backup findCommonBackup(int channel, const QDateTime &dateTime);
    static void setCommonBackupLock(const resp_search_common_backup &backup, int isLock);
    int currentSid();

    //tag
    static void addDrawTag(int channel, const QDateTime &dateTime);
    static void removeDrawTag(int channel, const QDateTime &dateTime);
    static void clearDrawTag();
    static QMap<QDateTime, int> drawTagMap(int channel);

    //当前通道
    static void setCurrentChannel(int channel);
    static int currentChannel();

    static QList<int> playingChannels();

    //所有选中的通道
    static QList<int> channelCheckedList();
    static void setChannelChecked(int channel, bool checked);
    static void clearChannelChecked();
    static bool isChannelChecked(int channel);
    static int channelCheckedCount();

    //回放状态
    static MsPlaybackState playbackState();
    static void setPlaybackState(MsPlaybackState state);
    static MsPlayState channelPlayState(int channel);
    static void setChannelPlayState(int channel, MsPlayState state);

    //鱼眼软解
    static bool isFisheyeDewarpEnable();
    static void setFisheyeDewarpState(int state);

    //事件过滤
    static void setFilterEvent(int filetEvent);
    static void setIsEventOnly(bool isEventOnly);
    static int getFilterEvent();
    static bool getIsEventOnly();
    static void waitForSearchGeneralEventPlayBack();
    static int getRECFilterEvent();
    //
    virtual void dealMessage(MessageReceive *message);
    virtual void ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(MessageReceive *message);
    virtual void ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE(MessageReceive *message);
    virtual void ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message);
    virtual void ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(MessageReceive *message);
    virtual void ON_RESPONSE_FLAG_START_ALL_PLAYBACK(MessageReceive *message);
    virtual void ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message);
    virtual void ON_RESPONSE_FLAG_RESTART_ALL_PLAYBACK(MessageReceive *message);

signals:

public slots:

protected:
    static QEventLoop *s_eventLoop;
    static QMap<int, PlaybackChannelInfo> s_channelInfoMap;
    //兼容跨天
    static QDateTime s_timelineBeginDateTime;
    static QDateTime s_timelineEndDateTime;

private:
    static bool s_isWaitting;
    static MsWaitting *s_waitting;
    //
    static MsPlaybackType s_playbackType;
    //
    static FILE_TYPE_EN s_playbackStream;
    //回放速度
    static PLAY_SPEED s_playbackSpeed;
    //回放方向
    static PB_ACTION_EN s_playbackDirection;
    //
    static QDate s_playbackDate;
    static QTime s_playbackTime;
    //
    static QList<int> s_checkedChannelList;
    //当前选中的通道
    static int s_currentChannel;
    //播放状态
    static MsPlaybackState s_playbackState;
    static QMap<int, MsPlayState> s_channelPlayStateMap;
    //音频
    static int s_audioChannel;
    //tag
    static QMap<int, QMap<QDateTime, int>> s_drawTagMap;
    //鱼眼软解
    static int s_fisheyeDewarpState;
    //事件过滤
    static int s_filterEvent;
    static bool s_isEventOnly;
};

#endif // BASEPLAYBACK_H
