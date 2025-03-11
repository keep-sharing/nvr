#ifndef LIVEVIEW_H
#define LIVEVIEW_H

#include "CustomLayoutKey.h"
#include "EventPopupData.h"
#include "LiveViewPlayback.h"
#include "MainMenu.h"
#include "TimeWidget.h"
#include "BaseWidget.h"
#include "networkcommond.h"
#include <QMap>
#include <QMenu>
#include <QPointer>
#include "CameraData.h"

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "FisheyeDewarpControl.h"
#else
#include "FisheyeControl.h"
#endif

extern "C" {
#include "msdb.h"
}

class BottomBar;
class CurrentVideo;
class LiveVideo;
class VideoBar;
class LivePage;
class ImageConfiguration;
class PtzControl;
class PTZ3DControl;
class SwapIndicator;
class LiveViewSearch;
class LiveViewAlarmOut;
class OptimalSplicingDistance;
namespace Ui {
class LiveView;
}

#define gLiveView LiveView::instance()

class LiveView : public BaseWidget {
    Q_OBJECT

public:
    enum ShowReason {
        NormalShowReason,
        LogoutShowReason,
        LoginShowReason
    };

    enum Mode {
        ModeNone = 0x0000,
        ModeRegular = 0x0001,
        ModeTarget = 0x0002,
        ModeOccupancy = 0x0004
    };
    Q_DECLARE_FLAGS(Modes, Mode)

    explicit LiveView(QWidget *parent = nullptr);
    ~LiveView();

    static LiveView *instance();

    void setGeometry(const QRect &rc);

    bool canAutoLogout();
    void showLiveView(ShowReason reason = NormalShowReason);
    void stopLiveView();
    void stopLiveView(SCREEN_E screen);

    void resetSubLayoutMode();
    void resetMainScreenLayout();
    void resetSubScreenLayout();
    void resetCurrentScreenLayout();
    void resetAnotherScreenLayout();

    void initializeVideo();
    bool isChannelConnected(int channel) const;

    bool isFisheyeMode();

    //
    void initializeLayout();
    CustomLayoutKey currentLayoutMode();
    CustomLayoutKey currentLayoutMode(int screen);
    int currentLayoutWindowCount();
    bool isSingleLayout(int screen) const;

    int currentMainFullScreenChannel() const;
    void setCurrentMainFullScreenChannel(int channel);
    int currentSubFullScreenChannel() const;
    void setCurrentSubFullScreenChannel(int channel);

    //dynamic display
    bool checkDynamicDisplayChannel(int channel);

    //轮巡
    void nextMainPage();
    void nextSubPage();

    //subcontrol
    void initializeSubScreenLayout();
    void stopSubScreen();
    void setSubLayout();
    void setSubDisplayInfo();
    void setSubMotion(const QMap<int, bool> &channelStateMap);
    void switchScreen();

    //popup
    bool canShowPopup();
    void preparePopup(int screen, int layout, const QList<int> &channels);
    void recoverLayoutBeforePopup(int screen);
    void closeAllPopupWindow();
    void closeAllPopupWindowWithoutToolBar();

    //swap
    void swapFinished(int index1, int channel1, int index2, int channel2);

    //display
    void changeDisplayInfo();
    void setDisplayMenuChecked(bool checked);

    //
    void showOrHideWidget(const QPoint &point);

    void showMainMenu();
    void hideMainMenu();

    void showBottomBar();
    void hideBottomBar();

    //anpr mode
    void initializePreviewMode();
    void showRegularMode();
    void showTargetMode();
    bool isTargetMode() const;

    LiveVideo *liveVideo(int channel);
    QRect liveviewGeometry();

    //track
    void setVcaTrackEnabled(int channel, int enable);
    void setPtzTrackEnabled(int channel, int enable);
    void setFisheyeTrackEnabled(int channel, int enable);

    //
    void updateTalkState(int channel);
    void updateAudioState(int channel);

    //
    void setTimeInfoMode(int mode);
    void resetTimeInfoMode();

    //
    void updateOccupancyMenu();

    //网络键盘
    int networkCurrentChannel();
    void networkMenu();
    void networkPtzControl(int action, int Hrate = 0, int Vrate = 0);
    void networkPresetControl(int action, int index);
    void networkPatrolControl(int action, int index);
    void networkPatternControl(int action, int index);
    bool networkTab();
    bool networkTab_Prev();
    void networkLayout(int mode);
    void networkNextPage();
    void networkPreviousPage();
    bool networkRecord();
    bool networkAudio();
    bool networkSnapshot();
    bool networkSelectChannel(int channel);
    bool networkFullscreenChannel(int channel);
    bool networkFullscreen();

    void focusNextChild();
    void focusPreviousChild();

    //Stream
    int streamInfo();
    bool isManualChangeStream() const;

    //
    int BottomBarMode() const;

    //message
    void dealMessage(MessageReceive *message);
    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_NOTIFYGUI(MessageReceive *message);
    void ON_RESPONSE_FLAG_NOTIFY_POPUP_GUI(MessageReceive *message);
    void ON_RESPONSE_FLAG_RET_RECSTATUS(MessageReceive *message);
    void ON_RESPONSE_FLAG_LIVE_SNAPSHOT_PHOTO(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_STATUS(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_CAMAPARAM(MessageReceive *message);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void escapePressed() override;
    bool isAddToVisibleList() override;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    //网络键盘
    NetworkResult dealNetworkCommond(const QString &commond) override;
    NetworkResult dealRockerNvr(const RockerDirection &direction) override;
    NetworkResult dealRockerPtz(const RockerDirection &direction, int hRate, int vRate) override;

private:
    void showAnprEvent(const QMap<int, bool> &map);
    void showSmartEvent(const QMap<int, bool> &map, quint64 chans);
    void showMotion(const QMap<int, bool> &map);
    void showRecord(int channel, bool show);
    void showVideoLoss(const QMap<int, bool> &map);
    void showAudioAlarm(const QMap<int, bool> &map);

    //video
    LiveVideo *makeNewVideo(int channel);
    LiveVideo *videoFromChannel(int channel);
    LiveVideo *popupVideoFromChannel(int channel);

    //
    void initializeContextMenu();
    void updateMultiScreenMenu();

    //layout, page: 0-n
    void setLayoutPage(int screen, int page);
    void setLayoutMode();
    void setLayoutMode(SCREEN_E screen);
    void setLayoutMode(const QString &name, int screen, int type, int page);
    void setLayoutMode(CustomLayoutKey key, int page);

    int currentPage() const;

    //单通道全屏
    void showFullChannel(int channel);
    void closeFullChannel();

    void hideToolBar();

    void hideAllVideoAdd();

    //ptz
    int execPtz3D(int channel);

    void setVideoRatio(int type, int channel);

signals:
    void menuItemClicked(const MainMenu::MenuItem &item);
    void layoutChanged(const CustomLayoutKey &key);

public slots:
    void onSetLayoutMode(int mode);
    void onSetLayoutMode(const QString &name);
    void onSetLayoutMode(const CustomLayoutKey &key);

    void onSetVideoRatio(int type);

    //
    void onCameraStateChanged(CameraData::State state);

    //page change
    void nextPage();
    void previousPage();

    //menu
    void onActionRegularModeClicked(bool checked);
    void onActionTargetModeClicked(bool checked);
    void onActionOccupancyModeClicked(bool checked);

    //video container
    void onVideoContainerClicked(int channel);

    //dynamic display
    void updateDynamicDisplay(int type, int channel);

private slots:
    void onLanguageChanged();

    //
    void adjustPagePos();

    //popup
    void onShowPopup(EventPopupInfo data);

    //video
    void videoClicked(int channel);
    void videoDoubleClicked(int channel);
    void onVideoAddButtonClicked(int channel);

    //定时获取REQUEST_FLAG_GET_IPC_STATUS
    void onCameraStatus(QMap<int, resp_camera_status> map);

    /****video bar****/
    void onEmergencyRecord(bool enable);
    void onImageConfiguration();
    void onPtz();
    void onPtzClosed();
    void onFisheye(int channel);
    void onVideoRatio(int type);
    void onZoom(bool enable);
    void onScreenshot(int channel);
    void onPlayback(int channel);
    void onVideoBarHide();
    void onAlarmOutPut(int channel);
    void onStream(int type);
    void onOptimalSplicing();
    void initializeVideoBar();

    //menu
    void onMenuActionClicked(bool checked);
    void onSingleScreenActionClicked(bool checked);
    void onMultiScreenActionClicked(bool checked);
    void onInfoActionClicked(bool checked);
    void onPreviousActionClicked(bool checked);
    void onNextActionClicked(bool checked);
    void onSubControlActionClicked(bool checked);
    void onLogoutActionClicked(bool checked);
    void onCustomContextMenuRequested(const QPoint &pos);

    void onBottomBarVisibleChanged(bool visible);

private:
    Ui::LiveView *ui;
    static LiveView *s_liveView;

    BottomBar *m_bottomBar = nullptr;
    MainMenu *m_mainMenu = nullptr;
    VideoBar *m_videoBar = nullptr;
    LivePage *m_page = nullptr;

    //display page, real page
    QMap<int, int> m_currentMainPageMap;
    QMap<int, int> m_currentSubPageMap;

    //当前布局
    layout_custom m_mainCurrentLayout;
    layout_custom m_subCurrentLayout;

    //保存全屏前的布局
    layout_custom m_mainLayoutBeforeFull;
    layout_custom m_subLayoutBeforeFull;

    //保存EventPopup前的布局
    layout_custom m_mainLayoutBeforePopup;
    layout_custom m_subLayoutBeforePopup;

    int m_currentMainFullScreenChannel = -1;
    int m_currentSubFullScreenChannel = -1;

    //
    LiveVideo *m_currentVideo = nullptr;
    CurrentVideo *m_videoFrame = nullptr;

    //channel, index
    QMap<int, LiveVideo *> m_mapChannelVideo;

    //
    ImageConfiguration *m_image = nullptr;
    PtzControl *m_ptz = nullptr;
    LiveViewAlarmOut *m_alarmout = nullptr;
    OptimalSplicingDistance *m_optimal = nullptr;

    //ptz3d
    PTZ3DControl *m_ptz3d = nullptr;

    //fisheye
#ifdef MS_FISHEYE_SOFT_DEWARP
    FisheyeDewarpControl *m_fisheyeDewarpControl = nullptr;
#else
    FisheyeControl *m_fisheye = nullptr;
#endif

    //右键菜单
    QMenu *m_contextMenu = nullptr;
    QMenu *m_singleScreenMenu = nullptr;
    QMenu *m_multiScreenMenu = nullptr;
    QAction *m_actionMenu = nullptr;
    QList<QAction *> m_actionSingleScreenList;
    QAction *m_actionInfo = nullptr;
    QAction *m_actionPrevious = nullptr;
    QAction *m_actionNext = nullptr;
    QAction *m_actionSubControl = nullptr;
    QAction *m_actionLogout = nullptr;
    //ANPR
    QMenu *m_menuPreview = nullptr;
    QAction *m_actionRegularMode = nullptr;
    QAction *m_actionTargetMode = nullptr;
    QAction *m_actionOccupancyMode = nullptr;

    //playback
    LiveViewPlayback *m_playback = nullptr;

    //search
    //LiveViewSearch *m_liveViewSearch = nullptr;

    //
    QMap<int, bool> m_noresourceMap;

    //
    QTimer *m_pageChangeLimitTimer = nullptr;

    //
    TimeWidget *m_timeWidget = nullptr;

    //
    ShowReason m_showReason = NormalShowReason;
    Modes m_modeBeforeLogout = ModeNone;

    //正在手动切换码流
    bool m_isManualChangeStream = false;
    //
    QMap<int, CameraData::State> m_streamStateMap;
};

#endif // LIVEVIEW_H
