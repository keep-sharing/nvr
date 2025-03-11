#ifndef LIVEVIDEO_H
#define LIVEVIDEO_H

#include "MsWidget.h"
#include "VideoScene.h"
#include "DrawView.h"
#include <PosData.h>
#include <QElapsedTimer>
#include "GraphicsItemPosText.h"

class VideoContainer;

extern "C" {
#include "msdb.h"
}

namespace Ui {
class LiveVideo;
}

class LiveVideo : public MsWidget {
    Q_OBJECT

public:
    struct State {
        bool isEmergencyRecording = false;
        bool isAudio = false;
        RATIO_E ratioMain = RATIO_VO_FULL; //原画或拉伸
        RATIO_E ratioSub = RATIO_VO_FULL; //原画或拉伸
    };

    explicit LiveVideo(QWidget *parent = 0);
    ~LiveVideo();

    static QColor s_infoColor;
    static bool s_showStreamInfo;
    static bool s_showChannelName;
    static bool s_showBorder;
    static int s_showChannelNameFontSize;

    static void setAnprState(int channel, bool state);
    static void setSmartEventState(int channel, bool state);
    static void setMotionState(int channel, bool state);
    static void setRecordState(int channel, bool state);
    static void setVideoLossState(int channel, bool state);
    static bool isVideoLoss(int channel);
    static void setNoResourceState(int winid, bool state);
    static void clearStreamInfo();
    static void setStreamInfo(int channel, const resp_camera_status &info);
    static resp_camera_status streamStatus(int channel);
    //主次码流
    int streamFormat();
    static bool isConnected(int channel);

    QRect globalGeometry() const;

    int channel() const;
    void setChannel(int channel);

    int screen() const;

    void setContainer(VideoContainer *container);
    void clearContainer();

    //每个page内唯一的，主要用于和底层通信
    int layoutIndex() const;
    int vapiWinId() const;
    bool isVapiWinIdValid() const;

    void setVideoDragEnable(bool enable);

    bool isChannelEnable() const;
    bool isChannelValid() const;

    bool isVideoLoss() const;
    bool isConnected() const;

    void updateStreamInfo();
    void updateAnprEvent();
    void updateSmartEvent();
    void updateMotion();
    void updateRecord();
    void updateVideoLoss();
    void showNoResource();
    void updateTalkState();
    void updateAudioState();
    void updateVideoScene();

    void showVideoAdd(bool show);

    void showChannelName();

    QSize mainStreamFrameSize() const;
    //视频画面实际显示区域，this坐标系
    QRect videoFrameRect() const;

    //pos
    void clearPos();

    //
    static LiveVideo::State state(int channel);
    static void setEmergencyRecord(int channel, bool enable);
    static void setAudio(int channel, bool enable);
    static bool audioState(int channel);
    static void setVideoRatio(int channel, int screen, const RATIO_E &ratio);
    static RATIO_E videoRatio(int channel, int screen);
    void setVideoRatio(int screen, const RATIO_E &ratio);

    static bool isDraging();

    static void setDisplayInfo(const display &display_info);
    void updateDisplayInfo();

    VideoScene *drawScene();

    //
    void dealMessage(MessageReceive *message);
    void dealEventDetectionRegionMessage(MessageReceive *message);

    void processMessage(MessageReceive *message) override;

    /**track**/
public:
    void initializeTrack(TrackMode mode, bool enable);
private:
    void checkHumanDetection();
    void closeHumanDetection();
    void checkPtzTracking();
    void closePtzTracking();
    void closeFisheyeTracking();
    void ON_RESPONSE_FLAG_GET_VCA_HUMANDETECTION(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message);
    QTimer *m_timerVcaTrackCheck = nullptr;
    QTimer *m_timerPtzTrackCheck = nullptr;
private slots:
    void onVcaTrackCheck();
    void onPtzTrackCheck();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool event(QEvent *) override;

private:
    void adjustAll();

    void adjustChannelName();
    void adjustAnprEvent();
    void adjustSmartEvent();
    void adjustMotion();
    void adjustRecord();
    void adjustVideoLoss();
    void adjustVideoAdd();
    void adjustStreamInfo();
    void adjustNoResource();
    void adjustDrawView();
    void adjustMessage();

    void adjustWidgetIcon();

signals:
    void mouseClicked(int channel);
    void mouseDoubleClicked(int channel);
    void showSwapVideo(int channel1, int channel2);
    void videoAddButtonClicked(int channel);

public slots:

private slots:
    void onLanguageChanged();

    void onToolButtonAddClicked();
    void onDoubleClickTimer();
    void onDragTimer();

    void onNoResourceChanged(int winid, int bNoResource);

    void onDrawViewShow();
    void onVcaAlarm(const MS_VCA_ALARM &alarm);

    //pos
    void onPosData(PosData data);

    //
    void onCameraFacePrivacyState(int chan, int state);

private:
    void resetAndCheckEventLoop();
    bool isEventLoopStillNormal(int level);

    Ui::LiveVideo *ui;

    static QMap<int, LiveVideo::State> s_mapVideoState;
    static QMap<int, bool> s_mapAnprState;
    static QMap<int, bool> s_mapSmartEventState;
    static QMap<int, bool> s_mapMotionState;
    static QMap<int, bool> s_mapRecordState;
    static QMap<int, bool> s_mapVideoLossState;
    static QMap<int, bool> s_mapNoResourceState;
    static QMap<int, resp_camera_status> s_mapStreamInfo;

    static bool s_isDragging;

    VideoContainer *m_container = nullptr;

    bool m_pressed = false;
    QPoint m_pressedPoint;

    State m_state;
    QSize m_iconSize;

    int m_channel = -1;

    int m_frameWidth = 0;
    int m_frameHeight = 0;

    QTimer *m_doubleClickTimer;
    bool m_isDoubleClicked = false;

    QElapsedTimer m_dragTimer;

    bool m_dragEnable = true;
    DrawView *m_drawView = nullptr;
    VideoScene *m_videoScene = nullptr;

    int eventLoopCheckCount = 0;

    //pos
    QMap<int, GraphicsItemPosText *> m_posTextItemMap;

    bool m_isFacePrivacyEnable = false;
    int m_codec;
};

QDebug operator<<(QDebug dbg, LiveVideo *video);

#endif // LIVEVIDEO_H
