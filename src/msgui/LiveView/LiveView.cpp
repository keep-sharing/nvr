#include "LiveView.h"
#include "ui_LiveView.h"
#include "BottomBar.h"
#include "ContentLiveView.h"
#include "CurrentVideo.h"
#include "CustomLayoutData.h"
#include "DynamicDisplayData.h"
#include "EventLoop.h"
#include "EventPopup.h"
#include "EventPopupSub.h"
#include "ImageConfiguration.h"
#include "LivePage.h"
#include "LiveVideo.h"
#include "LiveViewAlarmOut.h"
#include "LiveViewOccupancyManager.h"
#include "LiveViewSearch.h"
#include "LiveViewSub.h"
#include "LiveViewTargetPlay.h"
#include "LiveViewZoom.h"
#include "LogoutChannel.h"
#include "MainMenu.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MultiScreenControl.h"
#include "MyDebug.h"
#include "PlaybackWindow.h"
#include "PopupContent.h"
#include "SimpleToast.h"
#include "SubControl.h"
#include "VideoBar.h"
#include "VideoContainer.h"
#include "centralmessage.h"
#include "mainwindow.h"
#include "msuser.h"
#include "networkcommond.h"
#include "popupdialog.h"
#include "ptz3dcontrol.h"
#include "ptzcontrol.h"
#include "ptzdatamanager.h"
#include "runnablesavelayout.h"
#include "settingcontent.h"
#include "toast.h"
#include <QDesktopWidget>
#include <QPainter>
#include <QThreadPool>
#include <QWheelEvent>
#include "OptimalSplicingDistance.h"

extern "C" {

#include "ptz_public.h"
#include "recortsp.h"
}

LiveView *LiveView::s_liveView = nullptr;

LiveView::LiveView(QWidget *parent)
    : BaseWidget(parent)
{
    s_liveView = this;

    ui = new Ui::LiveView;
    ui->setupUi(this);

    //右键菜单
    initializeContextMenu();

    //
    ui->videoContainerManager->setLogicScreen(VideoContainerManager::LogicMain);
    LiveLayout::initializeCameraData();
    initializeVideo();
    initializeLayout();

    //
    initializePreviewMode();

    //
    m_bottomBar = new BottomBar(this);
    connect(m_bottomBar, SIGNAL(visibleChanged(bool)), this, SLOT(onBottomBarVisibleChanged(bool)));
    m_bottomBar->hide();

    //
    m_mainMenu = new MainMenu(this);
    connect(m_mainMenu, SIGNAL(itemClicked(const MainMenu::MenuItem &)), this, SIGNAL(menuItemClicked(const MainMenu::MenuItem &)));
    m_mainMenu->hide();

    //
    m_videoFrame = new CurrentVideo(this);
    m_videoFrame->hide();

    m_videoBar = new VideoBar(this);
    connect(m_videoBar, SIGNAL(sig_setEmergencyRecord(bool)), this, SLOT(onEmergencyRecord(bool)));
    connect(m_videoBar, SIGNAL(sig_imageConfiguration()), this, SLOT(onImageConfiguration()));
    connect(m_videoBar, SIGNAL(sig_ptz()), this, SLOT(onPtz()));
    connect(m_videoBar, SIGNAL(sig_fisheye(int)), this, SLOT(onFisheye(int)));
    connect(m_videoBar, SIGNAL(sig_videoRatio(int)), this, SLOT(onVideoRatio(int)));
    connect(m_videoBar, SIGNAL(sig_zoom(bool)), this, SLOT(onZoom(bool)));
    connect(m_videoBar, SIGNAL(sig_screenshot(int)), this, SLOT(onScreenshot(int)));
    connect(m_videoBar, SIGNAL(sig_playback(int)), this, SLOT(onPlayback(int)));
    connect(m_videoBar, SIGNAL(sig_hide()), this, SLOT(onVideoBarHide()));
    connect(m_videoBar, SIGNAL(sig_alarmout(int)), this, SLOT(onAlarmOutPut(int)));
    connect(m_videoBar, SIGNAL(sig_stream(int)), this, SLOT(onStream(int)));
    connect(m_videoBar, SIGNAL(sig_optimalSplicing()), this, SLOT(onOptimalSplicing()));
    m_videoBar->hide();

    m_page = new LivePage(this);
    LivePage::setMainPage(m_page);
    connect(m_page, SIGNAL(nextPage()), this, SLOT(nextPage()));
    connect(m_page, SIGNAL(previousPage()), this, SLOT(previousPage()));
    m_page->hide();

    //ptz
    m_ptz = new PtzControl(this);
    connect(m_ptz, SIGNAL(closed()), this, SLOT(onPtzClosed()));

    //
    connect(IrregularLayout::instance(), SIGNAL(customLayoutClicked(QString)), this, SLOT(onSetLayoutMode(QString)));

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    //
    connect(&gMsMessage, SIGNAL(sig_cameraStatus(QMap<int, resp_camera_status>)), this, SLOT(onCameraStatus(QMap<int, resp_camera_status>)));

    //
    connect(&gDynamicDisplayData, SIGNAL(dynamicDataChanged(int, int)), this, SLOT(updateDynamicDisplay(int, int)), Qt::QueuedConnection);

    //
    connect(&gCameraData, SIGNAL(cameraStateChanged(CameraData::State)), this, SLOT(onCameraStateChanged(CameraData::State)));

    //
    hide();

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_LIVE_SNAPSHOT_PHOTO, this);
}

LiveView::~LiveView()
{
    qDebug() << "LiveView::~LiveView()";

    s_liveView = nullptr;
    delete ui;
}

LiveView *LiveView::instance()
{
    return s_liveView;
}

void LiveView::updateDynamicDisplay(int type, int channel)
{
    Q_UNUSED(type)
    Q_UNUSED(channel)
}

void LiveView::setGeometry(const QRect &rc)
{
    BaseWidget::setGeometry(rc);
    //
    if (m_timeWidget) {
        resetTimeInfoMode();
    }
}

bool LiveView::canAutoLogout()
{
    return true;
}

void LiveView::showLiveView(ShowReason reason)
{
    gCameraData.getAllIpcData();
    m_showReason = reason;
    NetworkCommond::instance()->setPageMode(MOD_LIVEVIEW);

    //保存注销前的状态
    Modes modeBeforeLogout = ModeNone;
    if (ui->widget_target->isTargetEnable()) {
        modeBeforeLogout |= ModeTarget;
    } else {
        modeBeforeLogout |= ModeRegular;
    }
    //避免多次设置布局，这里先确定是不是TargetMode
    bool isTargetMode = false;
    if (m_showReason == LogoutShowReason) {
        //
        switch (LogoutChannel::instance()->logoutMode()) {
        case LogoutChannel::TargetMode:
            isTargetMode = true;
            ui->widget_target->clearTargetInfo();
            break;
        default:
            break;
        }
    } else if (m_showReason == LoginShowReason) {
        //恢复注销前的状态
        if (m_modeBeforeLogout & ModeTarget) {
            isTargetMode = true;
        }
    } else {
        isTargetMode = ui->widget_target->isTargetEnable();
    }
    if (isTargetMode) {
        ui->widget_target->setTargetEnable(true, m_showReason);
        ui->widget_target->show();
        if (qMsNvr->isSupportTargetMode()) {
            m_actionRegularMode->setChecked(false);
            m_actionTargetMode->setChecked(true);
            LiveviewBottomBar::instance()->updateTargetState();
        }
    } else {
        ui->widget_target->setTargetEnable(false, m_showReason);
        ui->widget_target->hide();
        if (qMsNvr->isSupportTargetMode()) {
            m_actionRegularMode->setChecked(true);
            m_actionTargetMode->setChecked(false);
            LiveviewBottomBar::instance()->updateTargetState();
        }
    }

    //
    if (g_isFirstLogin && SubControl::instance()->isSubEnable()) {
        //主屏控制时，第一次进来需要把辅屏清掉
        if (SubControl::instance()->isSubControl()) {
            SCREEN_E screen_e = SCREEN_MAIN;
            Q_UNUSED(screen_e)
        } else {
            SCREEN_E screen_e = SCREEN_SUB;
            Q_UNUSED(screen_e)
        }

        setLayoutMode(SCREEN_MAIN);
        setLayoutMode(SCREEN_SUB);
    } else {
        if (SubControl::instance()->isSubEnable()) {
            if (SubControl::instance()->isSubControl()) {
                setLayoutMode(SCREEN_SUB);
                if (qMsNvr->hasNoResource(SCREEN_MAIN) || reason == LogoutShowReason || reason == LoginShowReason) {
                    setLayoutMode(SCREEN_MAIN);
                }
            } else {
                setLayoutMode(SCREEN_MAIN);
                if (qMsNvr->hasNoResource(SCREEN_SUB) || reason == LogoutShowReason || reason == LoginShowReason) {
                    setLayoutMode(SCREEN_SUB);
                }
            }
        } else {
            if (SubControl::instance()->isSubControl()) {
                setLayoutMode();
            } else {
                setLayoutMode();
            }
        }
    }

    g_isFirstLogin = false;
    changeDisplayInfo();
    show();

    //
    if (LiveViewOccupancyManager::instance()->isOccupancyMode()) {
        modeBeforeLogout |= ModeOccupancy;
    }
    //工具栏，人数统计
    if (m_showReason == LogoutShowReason) {
        m_bottomBar->tempHide();
        m_mainMenu->animateHide();
        m_page->hide();
        //
        switch (LogoutChannel::instance()->logoutMode()) {
        case LogoutChannel::OccupancyMode:
            LiveViewOccupancyManager::instance()->showOccupancy(LogoutChannel::instance()->logoutGroup(), LiveViewOccupancyManager::LogoutReason);
            break;
        default:
            break;
        }
    } else {
        //
        m_bottomBar->setLocked(m_bottomBar->isLocked());
        //
        m_page->initializeData();
        adjustPagePos();
        //
        if (m_modeBeforeLogout & ModeOccupancy) {
            LiveViewOccupancyManager::instance()->resetOccupancy(LiveViewOccupancyManager::NormalReason);
        } else if (LiveViewOccupancyManager::instance()->isShowLater()) {
            //重启后显示
            LiveViewOccupancyManager::instance()->resetOccupancy(LiveViewOccupancyManager::NormalReason);
        } else {
            LiveViewOccupancyManager::instance()->closeOccupancy(LiveViewOccupancyManager::NormalReason);
        }
    }

    //
    m_modeBeforeLogout = modeBeforeLogout;
}

void LiveView::dealMessage(MessageReceive *message)
{
    if (m_videoBar) {
        m_videoBar->dealMessage(message);
    }
    //
    if (message->isAccepted()) {
        return;
    }
    //
    switch (message->type()) {
    case RESPONSE_FLAG_NOTIFYGUI:
        ON_RESPONSE_FLAG_NOTIFYGUI(message);
        break;
    case RESPONSE_FLAG_NOTIFY_POPUP_GUI:
        ON_RESPONSE_FLAG_NOTIFY_POPUP_GUI(message);
        break;
    case RESPONSE_FLAG_RET_RECSTATUS:
        ON_RESPONSE_FLAG_RET_RECSTATUS(message);
        break;
    default:
        break;
    }

    //
    if (m_bottomBar) {
        m_bottomBar->dealMessage(message);
    }
}

void LiveView::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_LIVE_SNAPSHOT_PHOTO:
        ON_RESPONSE_FLAG_LIVE_SNAPSHOT_PHOTO(message);
        break;
    case RESPONSE_FLAG_GET_IPC_STATUS:
        ON_RESPONSE_FLAG_GET_IPC_STATUS(message);
        break;
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_GET_AUTO_TRACKING(message);
        break;
    case RESPONSE_FLAG_SET_CAMAPARAM:
        ON_RESPONSE_FLAG_SET_CAMAPARAM(message);
        break;
    }
}

void LiveView::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_LIVE_SNAPSHOT_PHOTO:
        ON_RESPONSE_FLAG_LIVE_SNAPSHOT_PHOTO(message);
        break;
    }
}

void LiveView::ON_RESPONSE_FLAG_NOTIFYGUI(MessageReceive *message)
{
    resp_notify_gui *notify = (resp_notify_gui *)message->data;
    if (notify == nullptr) {
        qWarning() << QString("LiveView::ON_RESPONSE_FLAG_NOTIFYGUI, resp_notify_gui data is null.");
        return;
    }

    QMap<int, bool> channelStateMap;
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        if (notify->chans & ((quint64)1 << i)) {
            channelStateMap.insert(i, true);
        } else {
            channelStateMap.insert(i, false);
        }
    }
    switch (notify->type) {
    case MOTION_STATE:
        qMsCDebug("qt_motion_state") << QString("MOTION_STATE(64<==>1), %1").arg(notify->chans, 64, 2, QLatin1Char('0'));
        showMotion(channelStateMap);
        break;
    case VIDEOLOSS_STATE:
        qDebug() << QString("VIDEOLOSS_STATE, %1").arg(notify->chans, 64, 2, QLatin1Char('0'));
        showVideoLoss(channelStateMap);
        break;
    case POP_CHANS:
        break;
    case NO_RESOURCE: {
#ifdef VAPI2_0

#else
        qDebug() << QString("NO_RESOURCE, type: %1, channel: %2").arg(notify->livepb).arg(notify->chans, 64, 2, QLatin1Char('0'));
        switch (notify->livepb) {
        case DSP_MODE_LIVE:
            m_noresourceMap = channelStateMap;
            showNoResource(channelStateMap);
            //
            if (CommonVideo::instance()) {
                CommonVideo::instance()->showNoResource(channelStateMap);
            }
            if (LiveViewPlayback::instance()) {
                LiveViewPlayback::instance()->showNoResource(channelStateMap);
            }
            break;
        case DSP_MODE_PBK:
            //
            if (Playback::instance()) {
                Playback::instance()->showNoResource(channelStateMap);
            }
            break;
        case DSP_MODE_ZOOMIN:
            break;
        case DSP_MODE_ANPR:
            if (LiveViewTarget::instance()) {
                LiveViewTarget::instance()->showNoResource(channelStateMap);
            }
            break;
        default:
            break;
        }
#endif
        break;
    }
    case DISK_FAIL:
        break;
    case CODEC_CHANGE:
        break;
    case CODEC_NOMATCH:
        break;
    case POPUP_CHANS:
        //qDebug() << QString("POPUP_CHANS, %1").arg(notify->chans, 64, 2, QLatin1Char('0'));
        break;
    case SMART_EVENT_STATE: {
        showSmartEvent(channelStateMap, notify->chans);
        break;
    }
    case SMART_REGIONIN_STATE:
        break;
    case SMART_REGIONEXIT_STATE:
        break;
    case SMART_ADVANCED_MOTION_STATE:
        //MsDebug() << QString("SMART_EVENT_STATE, (n<-1)%1").arg(notify->chans, 64, 2, QLatin1Char('0'));
        break;
    case SMART_TAMPER_STATE:
        break;
    case SMART_LINRCORSS_STATE:
        break;
    case SMART_LOITER_STATE:
        break;
    case SMART_HUMAN_STATE:
        break;
    case ANPR_STATE: {
        //qDebug() << QString("ANPR_STATE, %1").arg(notify->chans, 64, 2, QLatin1Char('0'));
        showAnprEvent(channelStateMap);
        break;
    }
    case AUDIO_STATE: {
        showAudioAlarm(channelStateMap);
        break;
    }
    default:
        break;
    }
}

void LiveView::ON_RESPONSE_FLAG_NOTIFY_POPUP_GUI(MessageReceive *message)
{
    resp_notify_popup_gui *popup = static_cast<resp_notify_popup_gui *>(message->data);
    if (popup) {
        QList<int> channels;
        switch (popup->layout) {
        case LAYOUTMODE_1:
            channels.append(popup->channels[0]);
            break;
        case LAYOUTMODE_4:
            channels.append(popup->channels[0]);
            channels.append(popup->channels[1]);
            channels.append(popup->channels[2]);
            channels.append(popup->channels[3]);
            break;
        default:
            break;
        }
        qDebug() << QString("Event Popup, state: %1, layout: %2").arg(popup->state).arg(popup->layout) << channels;

        EventPopup::instance()->setPopupChannel(popup->state, popup->layout, channels);
    } else {
        qWarning() << "LiveView::ON_RESPONSE_FLAG_NOTIFY_POPUP_GUI, data is nullptr.";
    }
}

void LiveView::ON_RESPONSE_FLAG_RET_RECSTATUS(MessageReceive *message)
{
    resp_ret_rectask *status = (resp_ret_rectask *)message->data;
    if (status == nullptr) {
        qWarning() << QString("LiveView::ON_RESPONSE_FLAG_RET_RECSTATUS: data is null.");
        return;
    }
    showRecord(status->chanid, status->enable);
}

void LiveView::nextPage()
{
    if (SubControl::instance()->isSubControl()) {
        if (m_currentSubPageMap.size() < 2) {
            return;
        }
        int page = m_subCurrentLayout.page;
        qMsDebug() << "SCREEN_SUB, old page:" << page;
        if (page == m_currentSubPageMap.size() - 1) {
            page = 0;
        } else {
            page++;
        }
        qMsDebug() << "SCREEN_SUB, new page:" << page;
        setLayoutPage(SCREEN_SUB, page);
    } else {
        if (m_currentMainPageMap.size() < 2) {
            return;
        }
        int page = m_mainCurrentLayout.page;
        qMsDebug() << "SCREEN_MAIN, old page:" << page;
        if (page == m_currentMainPageMap.size() - 1) {
            page = 0;
        } else {
            page++;
        }
        qMsDebug() << "SCREEN_MAIN, new page:" << page;
        setLayoutPage(SCREEN_MAIN, page);
    }

    //MSHN-7543
    resetAnotherScreenLayout();
}

void LiveView::previousPage()
{
    if (SubControl::instance()->isSubControl()) {
        if (m_currentSubPageMap.size() < 2) {
            return;
        }
        int page = m_subCurrentLayout.page;
        qMsDebug() << "SCREEN_SUB, old page:" << page;
        if (page == 0) {
            page = m_currentSubPageMap.size() - 1;
        } else {
            page--;
        }
        qMsDebug() << "SCREEN_SUB, new page:" << page;
        setLayoutPage(SCREEN_SUB, page);
    } else {
        if (m_currentMainPageMap.size() < 2) {
            return;
        }
        int page = m_mainCurrentLayout.page;
        qMsDebug() << "SCREEN_MAIN, old page:" << page;
        if (page == 0) {
            page = m_currentMainPageMap.size() - 1;
        } else {
            page--;
        }
        qMsDebug() << "SCREEN_MAIN, new page:" << page;
        setLayoutPage(SCREEN_MAIN, page);
    }

    //MSHN-7543
    resetAnotherScreenLayout();
}

void LiveView::onLanguageChanged()
{
    m_actionMenu->setText(GET_TEXT("LIVEVIEW/20070", "Menu"));
    m_singleScreenMenu->setTitle(GET_TEXT("LIVEVIEW/20000", "Single Screen"));
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        QAction *action = m_actionSingleScreenList.at(i);
        action->setText(GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(i + 1));
    }
    m_multiScreenMenu->setTitle(GET_TEXT("LIVEVIEW/20032", "Multi Screen"));
    m_actionInfo->setText(GET_TEXT("LIVEVIEW/20067", "Display Info"));
    m_actionPrevious->setText(GET_TEXT("LIVEVIEW/20011", "Previous Screen"));
    m_actionNext->setText(GET_TEXT("LIVEVIEW/20012", "Next Screen"));
    if (m_menuPreview) {
        m_menuPreview->setTitle(GET_TEXT("ANPR/103000", "Preview Mode"));
    }
    if (m_actionRegularMode) {
        m_actionRegularMode->setText(GET_TEXT("ANPR/103001", "Regular Mode"));
    }
    if (m_actionTargetMode) {
        m_actionTargetMode->setText(GET_TEXT("TARGETMODE/103206", "Target Mode"));
    }
    if (m_actionOccupancyMode) {
        m_actionOccupancyMode->setText(GET_TEXT("OCCUPANCY/74255", "Occupancy Mode"));
    }
}

void LiveView::adjustPagePos()
{
    if (m_page) {
        m_page->adjustPos(liveviewGeometry());
    }
}

void LiveView::onShowPopup(EventPopupInfo data)
{
    qMsDebug();

    //
    switch (data.screen) {
    case SCREEN_MAIN:
        memcpy(&m_mainLayoutBeforePopup, &m_mainCurrentLayout, sizeof(layout_custom));
        break;
    case SCREEN_SUB:
        memcpy(&m_subLayoutBeforePopup, &m_subCurrentLayout, sizeof(layout_custom));
        break;
    }

    //
    if (data.screen == SubControl::instance()->currentScreen()) {
        //
        m_bottomBar->tempHide();
        m_mainMenu->animateHide();
        m_page->hide();
        //
        m_currentVideo = nullptr;
        m_videoFrame->hide();
        m_videoBar->hide();

        //
        switch (data.layout) {
        case LAYOUTMODE_1: {
            CustomLayoutInfo info("LAYOUTMODE_1", data.screen, 1, 1);
            info.setType(CustomLayoutKey::DefaultType);
            for (int r = 0; r < 1; ++r) {
                for (int c = 0; c < 1; ++c) {
                    info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
                }
            }
            for (int i = 0; i < data.channels.size(); ++i) {
                info.insertChannel(i, data.channels.at(i));
            }
            ui->videoContainerManager->setLayoutMode(info, 0);
            break;
        }
        case LAYOUTMODE_4: {
            CustomLayoutInfo info("LAYOUTMODE_4", data.screen, 2, 2);
            info.setType(CustomLayoutKey::DefaultType);
            for (int r = 0; r < 2; ++r) {
                for (int c = 0; c < 2; ++c) {
                    info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
                }
            }
            for (int i = 0; i < data.channels.size(); ++i) {
                info.insertChannel(i, data.channels.at(i));
            }
            ui->videoContainerManager->setLayoutMode(info, 0);
            break;
        }
        default:
            qMsWarning() << QString("invalid layout: %1").arg(data.layout);
            //MsWaitting::closeGlobalWait();
            return;
        }
    } else {
        if (LiveViewSub::instance()) {
            LiveViewSub::instance()->setPopupVideo(data.screen, data.layout, data.channels);
        }
    }
    //MsWaitting::closeGlobalWait();
}

void LiveView::onEmergencyRecord(bool enable)
{
    if (!m_currentVideo) {
        return;
    }
    int channel = m_currentVideo->channel();
    LiveVideo::setEmergencyRecord(channel, enable);

    //
    struct req_record_task recordTask;
    memset(&recordTask, 0, sizeof(struct req_record_task));
    recordTask.chanid = channel;
    recordTask.recType = REC_EVENT_MANUAL;
    recordTask.enable = enable;
    sendMessage(REQUEST_FLAG_EMERGENCY_REC_TASK, (void *)&recordTask, sizeof(req_record_task));
}

void LiveView::onImageConfiguration()
{
    if (!m_currentVideo) {
        return;
    }
    hideToolBar();

    if (!m_image) {
        m_image = new ImageConfiguration(this);
        connect(m_image, SIGNAL(finished(int)), m_videoBar, SLOT(show()));
    }

    QRect videoRect = m_currentVideo->globalGeometry();
    m_image->showImageInfo(m_currentVideo->channel(), videoRect);
    m_image->show();
}

void LiveView::onOptimalSplicing()
{
    if (!m_currentVideo) {
        return;
    }
    hideToolBar();

    if (!m_optimal) {
        m_optimal = new OptimalSplicingDistance(this);
        connect(m_optimal, SIGNAL(finished(int)), m_videoBar, SLOT(show()));
    }

    QRect videoRect = m_currentVideo->globalGeometry();
    m_optimal->showImageInfo(m_currentVideo->channel(), videoRect);
    m_optimal->show();
}

void LiveView::onPtz()
{
    if (!m_currentVideo) {
        return;
    }
    bool isBottomBarLocked = m_bottomBar->isLocked();
    //
    int channel = m_currentVideo->channel();
    int result = m_ptz->waitForGetPTZSupport(channel, m_currentVideo->globalGeometry(), PtzControl::MODE_PTZ);
    switch (result) {
    case PtzControl::STATE_NONSUPPORT: {
        ShowMessageBox(GET_TEXT("PTZCONFIG/36013", "This channel does not support this function."));
        return;
    }
    case PtzControl::STATE_PTZ_NORMAL: {
        hideToolBar();
        int reason = m_ptz->execPtz();
        switch (reason) {
        case PtzControl::ExitForPopup:
            break;
        default:
            if (m_showReason != LogoutShowReason) {
                if (isBottomBarLocked) {
                    m_bottomBar->setLocked(true);
                    m_bottomBar->animateShow();
                } else {
                    m_bottomBar->setLocked(false);
                    m_bottomBar->animateHide();
                }
            } else {
                m_bottomBar->animateHide();
            }
            break;
        }
        break;
    }
    case PtzControl::STATE_PTZ_3D: {
        hideToolBar();
        int reason = execPtz3D(channel);
        switch (reason) {
        case PTZ3DControl::ExitForPopup:
            break;
        default:
            if (m_showReason != LogoutShowReason) {
                if (isBottomBarLocked) {
                    m_bottomBar->setLocked(true);
                    m_bottomBar->animateShow();
                } else {
                    m_bottomBar->setLocked(false);
                    m_bottomBar->animateHide();
                }
            } else {
                m_bottomBar->animateHide();
            }
            break;
        }
        break;
    }
    default:
        break;
    }
}

void LiveView::onPtzClosed()
{
    if (m_ptz3d && m_ptz3d->isVisible()) {
        PtzBottomBar::instance()->setPtzButtonChecked(false);
        return;
    }
#ifdef MS_FISHEYE_SOFT_DEWARP
#else
    if (m_fisheye && m_fisheye->isVisible()) {
        m_fisheye->showFisheyeBar();
        return;
    }
#endif
    if (m_currentVideo) {
        m_videoBar->showVideoBar(m_currentVideo->channel());
    }
}

int LiveView::execPtz3D(int channel)
{
    if (!m_ptz3d) {
        m_ptz3d = new PTZ3DControl(this);
    }
    m_ptz3d->setGeometry(ui->widget_liveviewContent->geometry());
    m_ptz3d->setPtzWidget(m_ptz);
    m_ptz3d->setChannel(channel);
    //
    bool isFullBeforePtz = false;
    if (SubControl::instance()->isSubControl()) {
        isFullBeforePtz = CustomLayoutData::instance()->isSingleLayout(CustomLayoutKey(m_subCurrentLayout));
    } else {
        isFullBeforePtz = CustomLayoutData::instance()->isSingleLayout(CustomLayoutKey(m_mainCurrentLayout));
    }
    if (!isFullBeforePtz) {
        showFullChannel(channel);
    }
    //
    m_bottomBar->setBottomMode(BottomBar::ModePTZ3D, channel);
    //
    const int result = m_ptz3d->exec();
    switch (result) {
    case 0: //常规右键退出
        if (!isFullBeforePtz) {
            closeFullChannel();
        }
        //
        m_bottomBar->setBottomMode(BottomBar::ModeNormal);
        break;
    case 1: //event popup时退出ptz 3d
    case 2: //切换主辅屏
        m_bottomBar->setBottomMode(BottomBar::ModeNormal);
        break;
    default:
        break;
    }
    return result;
}

void LiveView::setVideoRatio(int type, int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    int screen = SubControl::instance()->currentScreen();

    video->setVideoRatio(screen, static_cast<RATIO_E>(type));

    struct req_video_play_ratio2 ratio;
    memset(&ratio, 0, sizeof(ratio));
    ratio.ratioType = type;
    ratio.winid = video->vapiWinId();
    qMsDebug() << QString("REQUEST_FLAG_SET_WIN_RATIO, channel: %1, type: %2, winid: %3").arg(channel).arg(ratio.ratioType).arg(VapiWinIdString(ratio.winid));
    sendMessageOnly(REQUEST_FLAG_SET_WIN_RATIO, &ratio, sizeof(ratio));
}

void LiveView::onFisheye(int channel)
{
    if (!m_currentVideo) {
        return;
    }

    bool isBottomBarLocked = m_bottomBar->isLocked();

#ifdef MS_FISHEYE_SOFT_DEWARP
    if (!m_fisheyeDewarpControl) {
        m_fisheyeDewarpControl = new FisheyeDewarpControl(this);
        FisheyeDewarpBottomBar *dewarpBar = FisheyeDewarpBottomBar::instance();
        if (dewarpBar) {
            connect(dewarpBar, SIGNAL(fisheyePanelButtonClicked(int, int)), m_fisheyeDewarpControl, SLOT(onLiveViewFisheyePanelButtonClicked(int, int)));
            connect(dewarpBar, SIGNAL(closeFisheyePanel()), m_fisheyeDewarpControl, SLOT(closeFisheyePanel()));
            connect(dewarpBar, SIGNAL(closeButtonClicked()), m_fisheyeDewarpControl, SLOT(closeFisheyeDewarp()));
            connect(m_fisheyeDewarpControl, SIGNAL(dewarpStateChanged(int)), dewarpBar, SLOT(onFisheyePanelButtonStateChanged(int)));
        } else {
            qWarning() << "FisheyeDewarpControl::FisheyeDewarpControl, FisheyeDewarpBottomBar::instance() is nullptr";
        }
    }
    m_fisheyeDewarpControl->setGeometry(ui->widget_liveviewContent->geometry());
    m_bottomBar->setBottomMode(BottomBar::ModeFisheye, channel);
    //
    layout_custom tempLayout;
    memset(&tempLayout, 0, sizeof(layout_custom));
    layout_custom *targetLayout = nullptr;
    if (SubControl::instance()->isSubControl()) {
        targetLayout = &m_subLayoutBeforeFull;
    } else {
        targetLayout = &m_mainLayoutBeforeFull;
    }
    memcpy(&tempLayout, targetLayout, sizeof(layout_custom));

    showFullChannel(channel);

    //
    hideToolBar();
    FisheyeKey fisheyeKey(FisheyeDewarpControl::ModeLiveview, channel, -1, SubControl::instance()->currentScreen());
    FisheyeDewarpControl::setCurrentChannel(fisheyeKey);
    FisheyeDewarpControl::setVapiWinId(m_currentVideo->vapiWinId());
    qDebug() << QString("Enter Fisheye Dewarp Mode, channel: %1, %2").arg(channel).arg(VapiWinIdString(FisheyePanel::s_vapiWinId));

    m_page->hide();
    int result = m_fisheyeDewarpControl->execFisheyeDewarpControl(fisheyeKey);
    Q_UNUSED(result)
    m_bottomBar->setBottomMode(BottomBar::ModeNormal);
    closeFullChannel();
    memcpy(targetLayout, &tempLayout, sizeof(layout_custom));
#else
    if (!m_fisheye) {
        m_fisheye = new FisheyeControl(this);
    }
    m_fisheye->setGeometry(ui->widget_liveviewContent->geometry());
    m_fisheye->setPtzControl(m_ptz);
    //
    bool isFullBeforeFisheye = false;
    if (SubControl::instance()->isSubControl()) {
        isFullBeforeFisheye = CustomLayoutData::instance()->isSingleLayout(CustomLayoutKey(m_subCurrentLayout));
    } else {
        isFullBeforeFisheye = CustomLayoutData::instance()->isSingleLayout(CustomLayoutKey(m_mainCurrentLayout));
    }
    if (!isFullBeforeFisheye) {
        showFullChannel(channel);
    }
    if (!m_currentVideo) {
        qWarning() << "LiveView::onFisheye, m_currentVideo is nullptr.";
        return;
    }
    //
    hideToolBar();
    qDebug() << QString("Enter Fisheye Mode, channel: %1").arg(channel);
    //
    m_bottomBar->setBottomMode(BottomBar::ModeFisheye);
    //
    m_ptz->setPtzMode(PtzControl::MODE_FISHEYE);
    //
    m_fisheye->showFisheyeControl(channel);
    m_page->hide();
    int result = m_fisheye->exec();
    //等待退出鱼眼
    switch (result) {
    case 0:
        break;
    case 1:
    case 2:
        break;
    }
    if (!isFullBeforeFisheye) {
        closeFullChannel();
    }
    //
    m_bottomBar->setBottomMode(BottomBar::ModeNormal);
    m_ptz->close();
#endif

    //
    if (m_showReason != LogoutShowReason) {
        if (isBottomBarLocked) {
            m_bottomBar->setLocked(true);
            m_bottomBar->animateShow();
        } else {
            m_bottomBar->setLocked(false);
            m_bottomBar->animateHide();
        }
    } else {
        m_bottomBar->animateHide();
    }

    //
    m_page->initializeData();
    adjustPagePos();
}

void LiveView::onVideoRatio(int type)
{
    if (m_currentVideo) {
        setVideoRatio(type, m_currentVideo->channel());
    }
}

void LiveView::onZoom(bool enable)
{
    Q_UNUSED(enable)

    if (!m_currentVideo) {
        return;
    }

    //
    m_videoFrame->hide();
    m_videoBar->hide();
    ui->videoContainerManager->hide();

    m_mainMenu->animateHide();

    bool isPageVisible = m_page->isVisible();
    m_page->setVisible(false);
    bool isBottomVarVisible = m_bottomBar->isVisible();
    m_bottomBar->setVisible(false);

    //放大前后layout状态不变
    layout_custom tempLayout;
    memset(&tempLayout, 0, sizeof(layout_custom));
    layout_custom *targetLayout = nullptr;
    if (SubControl::instance()->isSubControl()) {
        targetLayout = &m_subLayoutBeforeFull;
    } else {
        targetLayout = &m_mainLayoutBeforeFull;
    }
    memcpy(&tempLayout, targetLayout, sizeof(layout_custom));

    //
    showFullChannel(m_currentVideo->channel());
    //
    LiveViewZoom zoom(m_currentVideo->vapiWinId(), MainWindow::instance());
    zoom.showZoom(ui->widget_target->isTargetEnable() ? LiveViewZoom::Anpr : LiveViewZoom::Normal, ui->widget_liveviewContent->geometry());
    zoom.exec();
    //
    closeFullChannel();
    setLayoutMode();
    memcpy(targetLayout, &tempLayout, sizeof(layout_custom));

    ui->videoContainerManager->show();

    m_page->setVisible(isPageVisible);
    m_bottomBar->setVisible(isBottomVarVisible);
}

void LiveView::onScreenshot(int channel)
{
    if (!m_currentVideo) {
        return;
    }

    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        int vapi_id = video->vapiWinId();
        if (qMsNvr->isNoResource(vapi_id)) {
            Toast::showToast(this, GET_TEXT("LIVEVIEW/20057", "Snapshot Failed"));
            return;
        }
        //
        if (qMsNvr->is3798() || qMsNvr->is3536c()) {
            QSize mainFrameSize = video->mainStreamFrameSize();
            qMsDebug() << "main frame size:" << mainFrameSize;
            int size = qRound(mainFrameSize.width() * mainFrameSize.height() / 1024.0 / 1024.0);
            if (size > 8) {
                Toast::showToast(this, GET_TEXT("LIVEVIEW/20057", "Snapshot Failed"));
                return;
            }
        }

        qDebug() << "====LiveView::onScreenshot====";
        ms_live_snaphost snaphost;
        memset(&snaphost, 0, sizeof(ms_live_snaphost));
        snaphost.chnid = channel;
        snaphost.sid = video->layoutIndex();
        snaphost.screen = SubControl::instance()->currentScreen();
        snaphost.noresource = m_noresourceMap.value(channel, 0);
        qDebug() << "----chnid:" << snaphost.chnid;
        qDebug() << "----screen:" << snaphost.screen;
        qDebug() << "----sid:" << snaphost.sid;
        qDebug() << "----noresource:" << snaphost.noresource;
        //这个特殊，回复没有id
        sendMessageOnly(REQUEST_FLAG_LIVE_SNAPSHOT_PHOTO, (void *)&snaphost, sizeof(ms_live_snaphost));
    } else {
        qMsWarning() << "video is nullptr, channel:" << channel;
    }
}

void LiveView::onPlayback(int channel)
{
    if (!m_currentVideo) {
        return;
    }

    //
    m_currentVideo->clearPos();

    //
    if (!m_playback) {
        m_playback = new LiveViewPlayback(this);
    }
    m_videoBar->hide();
    int audioChannel = qMsNvr->liveviewAudioChannel();
    const int result = m_playback->execPlayback(channel, m_currentVideo->globalGeometry(), m_currentVideo->vapiWinId());
    if (result < 0) {
        ShowMessageBox(GET_TEXT("DISK/92033", "No record files currently."));
    }
    //恢复音频
    if (m_currentVideo) {
        if (audioChannel >= 0) {
            qMsNvr->openLiveviewAudio(m_currentVideo->channel());
        }
    }
    //event popup关掉playback
    if (result != 1) {
        m_videoBar->showVideoBar(channel);
    }

    //
    m_currentVideo->clearPos();
}

void LiveView::onVideoBarHide()
{
    m_videoFrame->hide();
    m_videoBar->hide();
}

void LiveView::onAlarmOutPut(int channel)
{
    if (!m_currentVideo) {
        return;
    }
    hideToolBar();
    Q_UNUSED(channel);
    if (!m_alarmout) {
        m_alarmout = new LiveViewAlarmOut(this);
        connect(m_alarmout, SIGNAL(finished(int)), m_videoBar, SLOT(show()));
    }
    QRect videoRect = m_currentVideo->globalGeometry();
    m_alarmout->moveAlarmOut(videoRect);
    m_alarmout->initializeData(channel);
}

void LiveView::onStream(int type)
{
    if (!m_currentVideo) {
        return;
    }
    MsWaittingContainer wait(m_currentVideo);
    m_isManualChangeStream = true;
    int channel = m_currentVideo->channel();
    if (!gCameraData.isCameraConnected(channel)) {
        qMsDebug() << "not connected";
        m_isManualChangeStream = false;
        return;
    }

    //
    const CustomLayoutInfo &info = CustomLayoutData::instance()->layoutInfo(currentLayoutMode());

    req_action_s action;
    memset(&action, 0, sizeof(req_action_s));

    action.chnid = channel;
    action.sid = m_currentVideo->layoutIndex();
    action.enScreen = static_cast<SCREEN_E>(info.screen());
    action.avFrom = AV_FROM_LIVE;
    if (type == STREAM_TYPE_MAINSTREAM) {
        action.avType = AV_TYPE_MAIN;
    } else {
        action.avType = AV_TYPE_SUB;
        if (!gCameraData.isCameraSubStreamConnected(action.chnid)) {
            SimpleToast::showToast(GET_TEXT("STREAM/115000", "Failed to switch the Stream Type."), m_currentVideo->globalGeometry());
            m_isManualChangeStream = false;
            return;
        }
    }

    qMsDebug() << QString("action:chid:%1,sid:%2,screen:%3,type:%4")
                      .arg(action.chnid)
                      .arg(action.sid)
                      .arg(action.enScreen)
                      .arg(action.avType);

    sendMessage(REQUEST_FLAG_SET_CAMAPARAM, &action, sizeof(req_action_s));
    gEventLoopExec();
    if (!m_currentVideo) {
        qWarning() << "invalid m_currentVideo";
        return;
    }
    if (STREAM_TYPE_MAINSTREAM != type) {
        SimpleToast::showToast(GET_TEXT("STREAM/115000", "Failed to switch the Stream Type."), m_currentVideo->globalGeometry());
        m_isManualChangeStream = false;
        return;
    }

    StreamKey key;
    key.currentChannel(action.chnid);
    key.setScreen(action.enScreen);
    CustomLayoutData::instance()->saveStreamType(key, type);
    m_videoBar->updateState(LiveVideo::state(m_currentVideo->channel()));

    //
    gMsMessage.refreshAllCameraStatus(100);
    m_isManualChangeStream = false;
}

void LiveView::initializeVideoBar()
{
    if (m_videoBar && m_currentVideo && !m_currentVideo->isConnected()) {
        closeAllPopupWindowWithoutToolBar();
        m_videoBar->updateState(LiveVideo::state(m_currentVideo->channel()));
    }
}

void LiveView::onMenuActionClicked(bool checked)
{
    Q_UNUSED(checked)
    m_mainMenu->setTempShow();
}

void LiveView::onSingleScreenActionClicked(bool checked)
{
    Q_UNUSED(checked)

    QAction *action = static_cast<QAction *>(sender());
    const int &channel = action->data().toInt();

    if (gCameraData.isCameraEnable(channel)) {
        showFullChannel(channel);
    }
}

void LiveView::onMultiScreenActionClicked(bool checked)
{
    Q_UNUSED(checked)

    QAction *action = static_cast<QAction *>(sender());
    const CustomLayoutKey &key = action->data().value<CustomLayoutKey>();
    onSetLayoutMode(key);
}

void LiveView::onInfoActionClicked(bool checked)
{
    struct display display_info = qMsNvr->displayInfo();
    if (checked) {
        display_info.camera_info = 1;
        display_info.show_channel_name = 1;
        display_info.border_line_on = 1;
        display_info.page_info = 2;
        m_page->setMode(LivePage::ModeAuto);
    } else {
        display_info.camera_info = 0;
        display_info.show_channel_name = 0;
        display_info.border_line_on = 0;
        display_info.page_info = 0;
        m_page->setMode(LivePage::ModeAlwaysHide);
    }
    qMsNvr->writeDisplayInfo(&display_info);

    if (DisplaySetting::instance()) {
        DisplaySetting::instance()->initializeData();
    }

    changeDisplayInfo();
}

void LiveView::onPreviousActionClicked(bool checked)
{
    Q_UNUSED(checked)
    previousPage();
}

void LiveView::onNextActionClicked(bool checked)
{
    Q_UNUSED(checked)
    nextPage();
}

void LiveView::onSubControlActionClicked(bool checked)
{
    Q_UNUSED(checked);

    if (SubControl::instance()->isSubControl()) {
        //当前在次屏，准备切换到主屏
        int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20028", "Do you want to enter main control?"));
        if (result == MessageBox::Cancel) {
            return;
        }
    } else {
        //当前在主屏，准备切换到次屏
        int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20027", "Do you want to enter sub control?"));
        if (result == MessageBox::Cancel) {
            return;
        }
    }

    SubControl::instance()->switchScreen();
}

void LiveView::onLogoutActionClicked(bool checked)
{
    Q_UNUSED(checked)
    m_mainMenu->showLogout();
}

void LiveView::showAnprEvent(const QMap<int, bool> &map)
{
    QMap<int, bool>::const_iterator iter = map.constBegin();
    for (; iter != map.constEnd(); ++iter) {
        int channel = iter.key();
        bool value = iter.value();
        LiveVideo::setAnprState(channel, value);
        //
        LiveVideo *video = videoFromChannel(channel);
        if (video) {
            video->updateAnprEvent();
        }

        //
        if (LiveViewSub::isSubEnable()) {
            LiveViewSub::instance()->updateAnprEvent(channel);
        }
    }
}

void LiveView::showSmartEvent(const QMap<int, bool> &map, quint64 chans)
{
    Q_UNUSED(chans);
    quint64 support = 0;
    //qMsDebug() << QString("SMART_EVENT_STATE, support\t, (n<->1)%1").arg(support, 64, 2, QLatin1Char('0'));
    //qMsDebug() << QString("SMART_EVENT_STATE, state\t, (n<->1)%1").arg(chans, 64, 2, QLatin1Char('0'));

    QMap<int, bool>::const_iterator iter = map.constBegin();
    for (; iter != map.constEnd(); ++iter) {
        int channel = iter.key();
        if (support & (static_cast<quint64>(1) << channel)) {

        } else {
            bool value = iter.value();
            LiveVideo::setSmartEventState(channel, value);
            //
            LiveVideo *video = videoFromChannel(channel);
            if (video) {
                video->updateSmartEvent();
            }

            //
            if (LiveViewSub::isSubEnable()) {
                LiveViewSub::instance()->updateSmartEvent(channel);
            }
        }
    }
}

void LiveView::showMotion(const QMap<int, bool> &map)
{
    QMap<int, bool>::const_iterator iter = map.constBegin();
    for (; iter != map.constEnd(); ++iter) {
        int channel = iter.key();
        bool value = iter.value();
        LiveVideo::setMotionState(channel, value);
        //
        LiveVideo *video = videoFromChannel(channel);
        if (video) {
            video->updateMotion();
        }

        //
        if (LiveViewSub::isSubEnable()) {
            LiveViewSub::instance()->updateMotion(channel);
        }
    }
}

void LiveView::showRecord(int channel, bool show)
{
    LiveVideo::setRecordState(channel, show);
    //
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->updateRecord();
    }

    //
    if (LiveViewSub::isSubEnable()) {
        LiveViewSub::instance()->updateRecord(channel);
    }
}

void LiveView::showVideoLoss(const QMap<int, bool> &map)
{
    QMap<int, bool>::const_iterator iter = map.constBegin();
    for (; iter != map.constEnd(); ++iter) {
        int channel = iter.key();
        bool value = iter.value();
        LiveVideo::setVideoLossState(channel, value);
        //
        LiveVideo *video = videoFromChannel(channel);
        if (video) {
            video->updateVideoLoss();
        }

        //
        if (LiveViewSub::isSubEnable()) {
            LiveViewSub::instance()->updateVideoLoss(channel);
        }
    }
}

void LiveView::showAudioAlarm(const QMap<int, bool> &map)
{
    QMap<int, bool>::const_iterator iter = map.constBegin();
    for (; iter != map.constEnd(); ++iter) {
        int channel = iter.key();
        bool value = iter.value();
        LiveVideo::setSmartEventState(channel, value);
        //
        LiveVideo *video = videoFromChannel(channel);
        if (video) {
            video->updateSmartEvent();
        }

        //
        if (LiveViewSub::isSubEnable()) {
            LiveViewSub::instance()->updateSmartEvent(channel);
        }
    }
}

void LiveView::updateTalkState(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->updateTalkState();
    }
}

void LiveView::updateAudioState(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->updateAudioState();
    }
}

LiveVideo *LiveView::makeNewVideo(int channel)
{
    LiveVideo *video = new LiveVideo(this);
    connect(video, SIGNAL(mouseClicked(int)), this, SLOT(videoClicked(int)));
    connect(video, SIGNAL(mouseDoubleClicked(int)), this, SLOT(videoDoubleClicked(int)));
    connect(video, SIGNAL(videoAddButtonClicked(int)), this, SLOT(onVideoAddButtonClicked(int)));
    video->setChannel(channel);
    video->hide();
    m_mapChannelVideo.insert(channel, video);
    return video;
}

LiveVideo *LiveView::videoFromChannel(int channel)
{
    LiveVideo *video = m_mapChannelVideo.value(channel, nullptr);
    return video;
}

LiveVideo *LiveView::popupVideoFromChannel(int channel)
{
    if (channel < 0) {
        return nullptr;
    }
    for (auto iter = m_mapChannelVideo.constBegin(); iter != m_mapChannelVideo.constEnd(); ++iter) {
        LiveVideo *video = iter.value();
        if (video->channel() == channel) {
            return video;
        }
    }
    return nullptr;
}

void LiveView::stopLiveView()
{
    stopLiveView(SubControl::instance()->currentScreen());
}

void LiveView::stopLiveView(SCREEN_E screen)
{
    qMsDebug() << screen;
    //停止预览
    struct req_layout2_s reqLayout;
    memset(&reqLayout, 0, sizeof(reqLayout));
    reqLayout.enScreen = screen;
    reqLayout.enMode = DSP_MODE_LIVE;
    sendMessageOnly(REQUEST_FLAG_SET_DISLAYOUT, &reqLayout, sizeof(reqLayout));
}

void LiveView::resetSubLayoutMode()
{
    if (SubControl::instance()->isSubEnable()) {
        if (SubControl::instance()->isSubControl()) {
            setLayoutMode(SCREEN_MAIN);
        } else {
            setLayoutMode(SCREEN_SUB);
        }
    }
}

void LiveView::resetMainScreenLayout()
{
    setLayoutMode(SCREEN_MAIN);
}

void LiveView::resetSubScreenLayout()
{
    setLayoutMode(SCREEN_SUB);
}

void LiveView::resetCurrentScreenLayout()
{
    if (SubControl::instance()->isSubControl()) {
        setLayoutMode(SCREEN_SUB);
    } else {
        setLayoutMode(SCREEN_MAIN);
    }
}

void LiveView::resetAnotherScreenLayout()
{
    if (SubControl::isSubEnable()) {
        if (SubControl::instance()->isSubControl()) {
            if (qMsNvr->hasNoResource(SCREEN_MAIN)) {
                setLayoutMode(SCREEN_MAIN);
            }
        } else {
            if (qMsNvr->hasNoResource(SCREEN_SUB)) {
                setLayoutMode(SCREEN_SUB);
            }
        }
    }
}

void LiveView::initializeVideo()
{
    int channelCount = qMsNvr->maxChannel();
    for (int i = 0; i < channelCount; ++i) {
        makeNewVideo(i);
    }
}

bool LiveView::isChannelConnected(int channel) const
{
    return gCameraData.isCameraConnected(channel);
}

bool LiveView::isFisheyeMode()
{
    bool result = false;
#ifdef MS_FISHEYE_SOFT_DEWARP
    if (m_fisheyeDewarpControl && m_fisheyeDewarpControl->isVisible()) {
        result = true;
    }
#endif
    return result;
}

void LiveView::initializeLayout()
{
    memset(&m_mainCurrentLayout, 0, sizeof(layout_custom));
    memset(&m_subCurrentLayout, 0, sizeof(layout_custom));
    memset(&m_mainLayoutBeforeFull, 0, sizeof(layout_custom));
    memset(&m_subLayoutBeforeFull, 0, sizeof(layout_custom));
    memset(&m_mainLayoutBeforePopup, 0, sizeof(layout_custom));
    memset(&m_subLayoutBeforePopup, 0, sizeof(layout_custom));

    layout_custom layouts[2];
    int count = 0;
    read_layout_custom_64Screen(SQLITE_FILE_NAME, layouts, &count);

    for (int i = 0; i < count; ++i) {
        const layout_custom &layout = layouts[i];
        switch (layout.screen) {
        case SCREEN_MAIN:
            memcpy(&m_mainCurrentLayout, &layout, sizeof(m_mainCurrentLayout));
            break;
        case SCREEN_SUB:
            memcpy(&m_subCurrentLayout, &layout, sizeof(m_subCurrentLayout));
            break;
        }
    }

    if (QString(m_mainCurrentLayout.name).isEmpty()) {
        m_mainCurrentLayout.screen = SCREEN_MAIN;
        m_mainCurrentLayout.page = 0;
        m_mainCurrentLayout.type = CustomLayoutKey::DefaultType;
        snprintf(m_mainCurrentLayout.name, sizeof(m_mainCurrentLayout.name), "%s", "LAYOUTMODE_4");
    }
    if (QString(m_subCurrentLayout.name).isEmpty()) {
        m_subCurrentLayout.screen = SCREEN_SUB;
        m_subCurrentLayout.page = 0;
        m_subCurrentLayout.type = CustomLayoutKey::DefaultType;
        snprintf(m_subCurrentLayout.name, sizeof(m_subCurrentLayout.name), "%s", "LAYOUTMODE_4");
    }
}

void LiveView::swapFinished(int index1, int channel1, int index2, int channel2)
{
    //交换
    ui->videoContainerManager->swapChannel(index1, channel1, index2, channel2);
    //存储
    CustomLayoutData::instance()->swapChannel(currentLayoutMode(), index1, channel1, index2, channel2);
    //
    videoClicked(channel1);
}

void LiveView::showOrHideWidget(const QPoint &point)
{
    if (!isVisible()) {
        return;
    }
    if (m_contextMenu->isVisible()) {
        return;
    }
    if (IrregularLayout::instance()->isVisible()) {
        return;
    }
    if (PopupContent::instance() && PopupContent::instance()->isVisible()) {
        return;
    }
    if (PopupDialog::hasPopupWindow()) {
        return;
    }
    if (EventPopup::instance()->isVisible()) {
        return;
    }
    if (m_image && m_image->isVisible()) {
        return;
    }
    if (m_alarmout && m_alarmout->isVisible()) {
        return;
    }
    if (m_playback && m_playback->isVisible()) {
        return;
    }
    if (m_optimal && m_optimal -> isVisible()) {
        return;
    }
    if (LiveViewTargetPlay::instance() && LiveViewTargetPlay::instance()->isVisible()) {
        return;
    }
    if (LiveViewZoom::instance() && LiveViewZoom::instance()->isVisible()) {
        return;
    }
    //ptz 3d不显示main menu和page
    if (m_ptz3d && m_ptz3d->isVisible()) {
        m_page->hide();
        m_mainMenu->animateHide();
        m_bottomBar->showOrHide(point);
        return;
    }
    //fisheye不显示main menu和page
#ifdef MS_FISHEYE_SOFT_DEWARP
    if (m_fisheyeDewarpControl && m_fisheyeDewarpControl->isVisible()) {
        m_page->hide();
        m_mainMenu->animateHide();
        m_bottomBar->showOrHide(point);
        return;
    }
#else
    if (m_fisheye && m_fisheye->isVisible()) {
        m_page->hide();
        m_mainMenu->animateHide();
        m_bottomBar->showOrHide(point);
        return;
    }
#endif

    //
    m_bottomBar->showOrHide(point);
    m_mainMenu->showOrHide(point);
    m_page->showOrHide(point);
}

void LiveView::showMainMenu()
{
    if (PopupDialog::hasPopupWindow()) {
        return;
    }
    if (EventPopup::instance()->isVisible()) {
        return;
    }
    if (m_image && m_image->isVisible()) {
        return;
    }
    if (m_alarmout && m_alarmout->isVisible()) {
        return;
    }
    if (m_playback && m_playback->isVisible()) {
        return;
    }
    if (m_optimal && m_optimal -> isVisible()) {
        return;
    }
    //ptz 3d不显示main menu和page
    if (m_ptz3d && m_ptz3d->isVisible()) {
        return;
    }
    //fisheye不显示main menu和page
#ifdef MS_FISHEYE_SOFT_DEWARP

#else
    if (m_fisheye && m_fisheye->isVisible()) {
        return;
    }
#endif

    //
    m_mainMenu->animateShow();
    m_mainMenu->setFocus();
}

void LiveView::hideMainMenu()
{
    m_mainMenu->animateHide();
}

void LiveView::showBottomBar()
{
    if (PopupDialog::hasPopupWindow()) {
        return;
    }
    if (EventPopup::instance()->isVisible()) {
        return;
    }
    if (m_image && m_image->isVisible()) {
        return;
    }
    if (m_alarmout && m_alarmout->isVisible()) {
        return;
    }
    if (m_playback && m_playback->isVisible()) {
        return;
    }
    if (m_optimal && m_optimal -> isVisible()) {
        return;
    }
    //
    m_bottomBar->animateShow();
}

void LiveView::hideBottomBar()
{
    m_bottomBar->animateHide();
}

void LiveView::initializePreviewMode()
{
    if (qMsNvr->isSupportTargetMode()) {
        if (qMsNvr->isTargetMode()) {
            m_actionRegularMode->setChecked(false);
            m_actionTargetMode->setChecked(true);
            showTargetMode();
        } else {
            m_actionRegularMode->setChecked(true);
            m_actionTargetMode->setChecked(false);
            showRegularMode();
        }
    } else {
        showRegularMode();
    }

    //
    if (qMsNvr->isOccupancyMode()) {
        m_actionOccupancyMode->setChecked(true);
        //这里可能在登录界面，等到预览的时候再显示
        LiveViewOccupancyManager::instance()->setShowLater();
    } else {
        m_actionOccupancyMode->setChecked(false);
    }
}

void LiveView::showRegularMode()
{
    qDebug() << QString("LiveView::showRegularMode");
    ui->widget_target->setTargetEnable(false, LiveView::NormalShowReason);
    ui->widget_target->hide();

    QTimer::singleShot(100, this, SLOT(adjustPagePos()));
}

void LiveView::showTargetMode()
{
    qDebug() << QString("LiveView::showAnprMode");
    ui->widget_target->setTargetEnable(true, LiveView::NormalShowReason);
    ui->widget_target->show();

    QTimer::singleShot(100, this, SLOT(adjustPagePos()));
}

bool LiveView::isTargetMode() const
{
    return ui->widget_target->isTargetEnable();
}

LiveVideo *LiveView::liveVideo(int channel)
{
    return m_mapChannelVideo.value(channel);
}

QRect LiveView::liveviewGeometry()
{
    QRect rc = ui->widget_liveviewContent->geometry();
    rc.setTopLeft(mapToGlobal(rc.topLeft()));
    return rc;
}

NetworkResult LiveView::dealNetworkCommond(const QString &commond)
{
    qDebug() << "LiveView::dealNetworkCommond," << commond;

    NetworkResult result = BaseWidget::dealNetworkCommond(commond);
    if (result == NetworkAccept) {
        return result;
    }

    //
    if (commond.startsWith("Video_Play")) {
        if (m_videoBar->isVisible()) {
            result = m_videoBar->dealNetworkCommond(commond);
        } else if (LiveViewTarget::instance()->isNetworkFocus()) {
            LiveViewTarget::instance()->networkPlay();
            result = NetworkAccept;
        }
    } else if (commond.startsWith("ChangeFocus_")) {
        result = networkTab() ? NetworkAccept : NetworkReject;
    } else if (commond.startsWith("ChangeFocus_Prev")) {
        result = networkTab_Prev() ? NetworkAccept : NetworkReject;
    } else if (commond.startsWith("Dial_Insid_Add")) {
        result = networkTab() ? NetworkAccept : NetworkReject;
    } else if (commond.startsWith("Dial_Insid_Sub")) {
        result = networkTab_Prev() ? NetworkAccept : NetworkReject;
    } else if (commond.startsWith("Screen_Prev")) {
        networkPreviousPage();
        result = NetworkAccept;
    } else if (commond.startsWith("Screen_Next")) {
        networkNextPage();
        result = NetworkAccept;
    } else if (commond.startsWith("SelWnd_")) {
        //SelWnd_3
        QStringList strList = commond.split("_");
        if (strList.size() == 2) {
            int channel = strList.at(1).toInt();
            if (channel >= 0 && channel < qMsNvr->maxChannel()) {
                networkSelectChannel(channel);
            }
        }
        result = NetworkAccept;
    } else if (commond.startsWith("Wnd_")) {
        int mode = -1;
        if (commond == "Wnd_1") {
            mode = LAYOUTMODE_1;
        } else if (commond == "Wnd_4") {
            mode = LAYOUTMODE_4;
        } else if (commond == "Wnd_8") {
            mode = LAYOUTMODE_8;
        } else if (commond == "Wnd_8_1") {
            mode = LAYOUTMODE_8_1;
        } else if (commond == "Wnd_9") {
            mode = LAYOUTMODE_9;
        } else if (commond == "Wnd_12") {
            mode = LAYOUTMODE_12;
        } else if (commond == "Wnd_12_1") {
            mode = LAYOUTMODE_12_1;
        } else if (commond == "Wnd_14") {
            mode = LAYOUTMODE_14;
        } else if (commond == "Wnd_16") {
            mode = LAYOUTMODE_16;
        } else if (commond == "Wnd_25") {
            mode = LAYOUTMODE_25;
        } else if (commond == "Wnd_32") {
            mode = LAYOUTMODE_32;
        } else if (commond == "Wnd_32_2") {
            mode = LAYOUTMODE_32_2;
        }
        if (mode > 0) {
            networkLayout(mode);
        }
        result = NetworkAccept;
    } else if (commond.startsWith("SingleChannel_")) {
        //SingleChannel_1
        QStringList strList = commond.split("_");
        if (strList.size() == 2) {
            int channel = strList.at(1).toInt();
            if (channel >= 0 && channel < qMsNvr->maxChannel()) {
                networkFullscreenChannel(channel);
            }
        }
        result = NetworkAccept;
    } else if (commond.startsWith("FullScreen")) {
        networkFullscreen();
        result = NetworkAccept;
    } else if (commond.startsWith("Audio")) {
        networkAudio();
        result = NetworkAccept;
    } else if (commond.startsWith("Rec")) {
        networkRecord();
        result = NetworkAccept;
    } else if (commond.startsWith("Snap")) {
        networkSnapshot();
        result = NetworkAccept;
    } else if (commond.startsWith("Toolbar")) {
        //
        if (m_bottomBar->isVisible()) {
            hideBottomBar();
        } else {
            showBottomBar();
        }
        result = NetworkAccept;
    } else if (commond.startsWith("Menu")) {
        networkMenu();
        result = NetworkAccept;
    } else if (commond.startsWith("Seq_")) {
        if (Sequence::instance()) {
            Sequence::instance()->networkSequence();
        }
        result = NetworkAccept;
    }

    return result;
}

void LiveView::setVcaTrackEnabled(int channel, int enable)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->initializeTrack(TrackModeVca, enable);
    }
}

void LiveView::setPtzTrackEnabled(int channel, int enable)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->initializeTrack(TrackModePtz, enable);
    }
}

void LiveView::setFisheyeTrackEnabled(int channel, int enable)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->initializeTrack(TrackModeFisheye, enable);
    }
}

/**
 * @brief LiveView::networkMenu
 * 网络键盘：MENU
 */
void LiveView::networkMenu()
{
    if (m_mainMenu->isShow()) {
        hideMainMenu();
    } else {
        showMainMenu();
    }
}

NetworkResult LiveView::dealRockerNvr(const RockerDirection &direction)
{
    //anpr
    if (LiveViewTarget::instance()->isNetworkFocus()) {
        if (LiveViewTarget::instance()->isPlaying()) {
            return NetworkAccept;
        }

        switch (direction) {
        case RockerRight:
            LiveViewTarget::instance()->setNetworkFocus(false);
            break;
        case RockerUp:
            LiveViewTarget::instance()->networkFocusPrevious();
            return NetworkAccept;
        case RockerDown:
            LiveViewTarget::instance()->networkFocusNext();
            return NetworkAccept;
        default:
            return NetworkReject;
        }
    } else if (m_currentVideo && m_currentVideo->layoutIndex() == 0 && LiveViewTarget::instance()->isVisible()) {
        if (direction == RockerLeft) {
            LiveViewTarget::instance()->setNetworkFocus(true);
            if (m_videoFrame->isVisible()) {
                m_videoFrame->hide();
            }
            if (m_videoBar->isVisible()) {
                m_videoBar->closeVideoBar();
            }
            if (m_currentVideo) {
                m_currentVideo->showVideoAdd(false);
                m_currentVideo = nullptr;
            }
            return NetworkAccept;
        }
    }

    //video
    if (PTZ3DControl::instance() && PTZ3DControl::instance()->isVisible()) {
        return NetworkReject;
    }
#ifdef MS_FISHEYE_SOFT_DEWARP
#else
    if (FisheyeControl::instance() && FisheyeControl::instance()->isVisible()) {
        return NetworkReject;
    }
#endif
    int index;
    if (!m_currentVideo) {
        index = 0;
    } else {
        index = m_currentVideo->layoutIndex();
        switch (direction) {
        case RockerUp:
        case RockerLeft:
        case RockerUpLeft:
        case RockerUpRight:
            index--;
            break;
        case RockerDown:
        case RockerRight:
        case RockerDownLeft:
        case RockerDownRight:
            index++;
            break;
        default:
            break;
        }
    }
    const CustomLayoutKey &key = currentLayoutMode();
    const CustomLayoutInfo &info = CustomLayoutData::instance()->layoutInfo(currentLayoutMode());
    if (!info.isValid()) {
        qMsWarning() << "invalid key:" << key;
        return NetworkAccept;
    }
    int maxIndex = qMin(info.positionCount(), qMsNvr->maxChannel()) - 1;
    if (index < 0) {
        previousPage();
        index = maxIndex;
        m_page->showPage(2000);
    }
    if (index > maxIndex) {
        nextPage();
        index = 0;
        m_page->showPage(2000);
    }

    LiveVideo *video = ui->videoContainerManager->videoFromIndex(index);
    if (!video) {
        qMsWarning() << QString("video is null.");
        return NetworkReject;
    }
    videoClicked(video->channel());

    return NetworkAccept;
}

NetworkResult LiveView::dealRockerPtz(const RockerDirection &direction, int hRate, int vRate)
{
    NetworkResult result = NetworkReject;

    int action = -1;
    switch (direction) {
    case RockerUp:
        action = PTZ_UP;
        break;
    case RockerDown:
        action = PTZ_DOWN;
        break;
    case RockerLeft:
        action = PTZ_LEFT;
        break;
    case RockerRight:
        action = PTZ_RIGHT;
        break;
    case RockerUpLeft:
        action = PTZ_LEFT_UP;
        break;
    case RockerUpRight:
        action = PTZ_RIGHT_UP;
        break;
    case RockerDownLeft:
        action = PTZ_LEFT_DOWN;
        break;
    case RockerDownRight:
        action = PTZ_RIGHT_DOWN;
        break;
    case RockerStop:
        action = PTZ_STOP_ALL;
        break;
    default:
        break;
    }
    if (action >= 0) {
        if (m_ptz) {
            const int channel = networkCurrentChannel();
            gPtzDataManager->setChannel(channel);
            m_ptz->setChannel(channel);
            m_ptz->sendPtzControl(action, hRate, vRate);
            result = NetworkAccept;
        }
    }

    return result;
}

/**
 * @brief LiveView::networkPtzControl
 * 网络键盘：ptz
 * @param action
 */
void LiveView::networkPtzControl(int action, int Hrate, int Vrate)
{
    if (m_ptz) {
        const int channel = networkCurrentChannel();
        gPtzDataManager->setChannel(channel);
        m_ptz->setChannel(channel);
        m_ptz->sendPtzControl(action, Hrate, Vrate);
    }
}

void LiveView::networkPresetControl(int action, int index)
{
    if (m_ptz) {
        const int channel = networkCurrentChannel();
        gPtzDataManager->setChannel(channel);
        m_ptz->setChannel(channel);
        m_ptz->sendPresetControl(action, index);
    }
}

void LiveView::networkPatrolControl(int action, int index)
{
    if (m_ptz) {
        const int channel = networkCurrentChannel();
        gPtzDataManager->setChannel(channel);
        m_ptz->setChannel(channel);
        m_ptz->sendPatrolControl(action, index);
    }
}

void LiveView::networkPatternControl(int action, int index)
{
    if (m_ptz) {
        const int channel = networkCurrentChannel();
        gPtzDataManager->setChannel(channel);
        m_ptz->setChannel(channel);
        m_ptz->sendPatternControl(action, index);
    }
}

/**
 * @brief LiveView::networkTab
 * 网络键盘：T2
 * @return
 */
bool LiveView::networkTab()
{
    if (!isVisible()) {
        return false;
    }
    //
    if (m_videoBar->isVisible()) {
        m_videoBar->focusNextChild();
        return true;
    }
    //
    focusNextChild();
    return true;
}

bool LiveView::networkTab_Prev()
{
    if (!isVisible()) {
        return false;
    }
    //
    if (m_videoBar->isVisible()) {
        m_videoBar->focusPreviousChild();
        return true;
    }
    //
    focusPreviousChild();
    return true;
}

/**
 * @brief LiveView::networkLayout
 * 网络键盘：MULT
 * @param mode
 */
void LiveView::networkLayout(int mode)
{
    onSetLayoutMode(mode);
}

/**
 * @brief LiveView::networkNextPage
 * 网络键盘：NEXT
 */
void LiveView::networkNextPage()
{
    nextPage();
    m_page->showPage(2000);
}

/**
 * @brief LiveView::networkPreviousPage
 * 网络键盘：PREV
 */
void LiveView::networkPreviousPage()
{
    previousPage();
    m_page->showPage(2000);
}

/**
 * @brief LiveView::networkRecord
 * 网络键盘：REC
 * @return
 */
bool LiveView::networkRecord()
{
    if (!isVisible()) {
        return false;
    }
    if (!m_videoBar->isVisible()) {
        return false;
    }
    m_videoBar->recordClicked();
    return true;
}

/**
 * @brief LiveView::networkAudio
 * 网络键盘：AUDIO
 * @return
 */
bool LiveView::networkAudio()
{
    if (!isVisible()) {
        return false;
    }
    if (!m_videoBar->isVisible()) {
        return false;
    }
    m_videoBar->audioClicked();
    return true;
}

/**
 * @brief LiveView::networkSnapshot
 * 网络键盘：SNAP
 * @return
 */
bool LiveView::networkSnapshot()
{
    if (!isVisible()) {
        return false;
    }
    if (!m_videoBar->isVisible()) {
        return false;
    }
    m_videoBar->snapshotClicked();
    return true;
}

/**
 * @brief LiveView::networkSelectChannel
 * 网络键盘：WIN
 * @param channel
 * @return
 */
bool LiveView::networkSelectChannel(int channel)
{
    if (!isVisible()) {
        return false;
    }
    const CustomLayoutKey &key = currentLayoutMode();
    const CustomLayoutInfo &info = CustomLayoutData::instance()->layoutInfo(key);
    if (!info.isValid()) {
        qMsWarning() << "invalid key:" << key;
        return false;
    }
    int page = info.pageOfChannel(channel);
    if (page != currentPage()) {
        setLayoutMode(currentLayoutMode(), page);
    }
    LiveVideo *video = videoFromChannel(channel);
    if (!video) {
        return false;
    }
    videoClicked(channel);

    return true;
}

/**
 * @brief LiveView::networkFullscreenChannel
 * 网络键盘：CAM
 * @param channel
 * @return
 */
bool LiveView::networkFullscreenChannel(int channel)
{
    if (!isVisible()) {
        return false;
    }
    if (PTZ3DControl::instance() && PTZ3DControl::instance()->isVisible()) {
        return false;
    }
#ifdef MS_FISHEYE_SOFT_DEWARP
#else
    if (FisheyeControl::instance() && FisheyeControl::instance()->isVisible()) {
        return false;
    }
#endif
    if (gCameraData.isCameraEnable(channel)) {
        showFullChannel(channel);
    }
    return true;
}

/**
 * @brief LiveView::networkFullscreenChannel
 * @return
 */
bool LiveView::networkFullscreen()
{
    if (!isVisible()) {
        return false;
    }
    if (isSingleLayout(SubControl::instance()->currentScreen())) {
        closeFullChannel();
    } else {
        if (m_currentVideo) {
            int channel = m_currentVideo->channel();
            videoDoubleClicked(channel);
        }
    }

    return true;
}

void LiveView::focusNextChild()
{
    qDebug() << "LiveView::focusNextChild";
    QWidget::focusNextChild();
}

void LiveView::focusPreviousChild()
{
    qDebug() << "LiveView::focusPreviousChild";
    QWidget::focusPreviousChild();
}

void LiveView::ON_RESPONSE_FLAG_LIVE_SNAPSHOT_PHOTO(MessageReceive *message)
{
    struct resp_snapshot_state *snapshot_state = (struct resp_snapshot_state *)message->data;
    if (!snapshot_state) {
        qWarning() << "LiveView::ON_RESPONSE_FLAG_LIVE_SNAPSHOT_PHOTO, data is null.";
        return;
    }
    if (snapshot_state->status == 0) {
        //成功
        Toast::showToast(this, GET_TEXT("LIVEVIEW/20055", "Snapshot Successfully"));
    } else {
        Toast::showToast(this, GET_TEXT("LIVEVIEW/20057", "Snapshot Failed"));
    }
}

void LiveView::ON_RESPONSE_FLAG_GET_IPC_STATUS(MessageReceive *message)
{
    struct resp_camera_status *camera_status_array = (struct resp_camera_status *)message->data;
    int count = message->header.size / sizeof(struct resp_camera_status);

    LiveVideo::clearStreamInfo();
    for (int i = 0; i < count; ++i) {
        const resp_camera_status &camera_status = camera_status_array[i];
        LiveVideo::setStreamInfo(camera_status.chnid, camera_status);
    }
    //
    for (auto iter = m_mapChannelVideo.constBegin(); iter != m_mapChannelVideo.constEnd(); ++iter) {
        LiveVideo *video = iter.value();
        if (video->isChannelValid()) {
            video->updateStreamInfo();
        }
        //
        int channel = video->channel();
        const resp_camera_status &camera_status = video->streamStatus(channel);
        if (camera_status.chnid < 0) {
            continue;
        }
    }
    initializeVideoBar();
}

void LiveView::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
#ifdef MS_FISHEYE_SOFT_DEWARP
    Q_UNUSED(message)
#else
    if (m_fisheye) {
        m_fisheye->processMessage(message);
    }
#endif
}

void LiveView::ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message)
{
    if (!isVisible()) {
        return;
    }

    ms_auto_tracking *info = (ms_auto_tracking *)message->data;
    if (!info) {
        return;
    }
    //
#ifdef MS_FISHEYE_SOFT_DEWARP

#else
    if (m_fisheye) {
        m_fisheye->processMessage(message);
    }
#endif
}

void LiveView::ON_RESPONSE_FLAG_SET_CAMAPARAM(MessageReceive *message)
{
    int *result = (int *)message->data;
    if (gEventLoop.isRunning()) {
        gEventLoopExit(*result);
    }
}

void LiveView::wheelEvent(QWheelEvent *event)
{
    if (LiveViewZoom::instance() && LiveViewZoom::instance()->isVisible()) {
        return;
    }
    if (m_ptz3d && m_ptz3d->isVisible()) {
        return;
    }
#ifdef MS_FISHEYE_SOFT_DEWARP

#else
    if (m_fisheye && m_fisheye->isVisible()) {
        return;
    }
#endif

    if (LiveVideo::isDraging()) {
        return;
    }

    //限制翻页频率
    if (!m_pageChangeLimitTimer) {
        m_pageChangeLimitTimer = new QTimer(this);
        m_pageChangeLimitTimer->setSingleShot(true);
    }
    if (m_pageChangeLimitTimer->isActive()) {
        return;
    }
    m_pageChangeLimitTimer->start(1000);
    //
    int numDegrees = event->delta();
    if (numDegrees < 0) {
        nextPage();
    } else if (numDegrees > 0) {
        previousPage();
    }
    m_page->showPage(2000);
}

void LiveView::escapePressed()
{
    if (m_videoBar->isVisible()) {
        m_videoBar->closeVideoBar();
    }
}

bool LiveView::isAddToVisibleList()
{
    return true;
}

void LiveView::showEvent(QShowEvent *event)
{
    if (m_timeWidget) {
        resetTimeInfoMode();
    }
    m_streamStateMap.clear();
    BaseWidget::showEvent(event);
}

void LiveView::hideEvent(QHideEvent *event)
{
    if (m_timeWidget) {
        m_timeWidget->hide();
    }
    BaseWidget::hideEvent(event);
}

void LiveView::initializeContextMenu()
{
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested(QPoint)));
    m_contextMenu = new QMenu(this);
    m_actionMenu = new QAction(GET_TEXT("LIVEVIEW/20070", "Menu"), m_contextMenu);
    connect(m_actionMenu, SIGNAL(triggered(bool)), this, SLOT(onMenuActionClicked(bool)));
    m_actionInfo = new QAction(GET_TEXT("LIVEVIEW/20067", "Display Info"), m_contextMenu);
    m_actionInfo->setCheckable(true);
    connect(m_actionInfo, SIGNAL(triggered(bool)), this, SLOT(onInfoActionClicked(bool)));
    m_actionPrevious = new QAction(GET_TEXT("LIVEVIEW/20011", "Previous Screen"), m_contextMenu);
    connect(m_actionPrevious, SIGNAL(triggered(bool)), this, SLOT(onPreviousActionClicked(bool)));
    m_actionNext = new QAction(GET_TEXT("LIVEVIEW/20012", "Next Screen"), m_contextMenu);
    connect(m_actionNext, SIGNAL(triggered(bool)), this, SLOT(onNextActionClicked(bool)));
    m_actionSubControl = new QAction(GET_TEXT("LIVEVIEW/20015", "Sub Screen Ctrl"), m_contextMenu);
    connect(m_actionSubControl, SIGNAL(triggered(bool)), this, SLOT(onSubControlActionClicked(bool)));
    m_actionLogout = new QAction("Logout(admin)", m_contextMenu);
    connect(m_actionLogout, SIGNAL(triggered(bool)), this, SLOT(onLogoutActionClicked(bool)));

    //Single Screen
    m_singleScreenMenu = new QMenu(GET_TEXT("LIVEVIEW/20000", "Single Screen"), m_contextMenu);
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        QAction *action = new QAction(m_singleScreenMenu);
        action->setData(i);
        action->setText(GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(i + 1));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onSingleScreenActionClicked(bool)));
        m_actionSingleScreenList.append(action);
        m_singleScreenMenu->addAction(action);
    }

    //Multi Screen
    m_multiScreenMenu = new QMenu(GET_TEXT("LIVEVIEW/20032", "Multi Screen"), m_contextMenu);
    updateMultiScreenMenu();

    //Preview Mode
    m_menuPreview = new QMenu(GET_TEXT("ANPR/103000", "Preview Mode"), this);
    if (qMsNvr->isSupportTargetMode()) {
        m_actionRegularMode = new QAction(GET_TEXT("ANPR/103001", "Regular Mode"), this);
        m_actionRegularMode->setCheckable(true);
        connect(m_actionRegularMode, SIGNAL(triggered(bool)), this, SLOT(onActionRegularModeClicked(bool)));
        m_menuPreview->addAction(m_actionRegularMode);

        m_actionTargetMode = new QAction(GET_TEXT("TARGETMODE/103206", "Target Mode"), this);
        m_actionTargetMode->setCheckable(true);
        connect(m_actionTargetMode, SIGNAL(triggered(bool)), this, SLOT(onActionTargetModeClicked(bool)));
        m_menuPreview->addAction(m_actionTargetMode);
    }
    m_actionOccupancyMode = new QAction(GET_TEXT("OCCUPANCY/74255", "Occupancy Mode"), this);
    m_actionOccupancyMode->setCheckable(true);
    connect(m_actionOccupancyMode, SIGNAL(triggered(bool)), this, SLOT(onActionOccupancyModeClicked(bool)));
    m_menuPreview->addAction(m_actionOccupancyMode);

    //
    m_contextMenu->addAction(m_actionMenu);
    m_contextMenu->addMenu(m_singleScreenMenu);
    m_contextMenu->addMenu(m_multiScreenMenu);
    if (m_menuPreview) {
        m_contextMenu->addMenu(m_menuPreview);
    }
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionInfo);
    m_contextMenu->addAction(m_actionPrevious);
    m_contextMenu->addAction(m_actionNext);
    m_contextMenu->addAction(m_actionLogout);
}

void LiveView::updateMultiScreenMenu()
{
    m_multiScreenMenu->clear();

    QList<QPair<QString, QString>> multiActionPairs;
    multiActionPairs.append(qMakePair(QString("LAYOUTMODE_4"), GET_TEXT("LIVEVIEW/20002", "4 Screen")));
    //根据机型通道数定制右键菜单
    int channelCount = qMsNvr->maxChannel();
    if (channelCount >= 8) {
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_8"), GET_TEXT("LIVEVIEW/20003", "8 Screen")));
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_8_1"), GET_TEXT("LIVEVIEW/20004", "1+7 Screen")));
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_9"), GET_TEXT("LIVEVIEW/20005", "9 Screen")));
    }
    if (channelCount >= 12) {
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_12"), GET_TEXT("LIVEVIEW/20006", "12 Screen")));
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_12_1"), GET_TEXT("LIVEVIEW/20007", "1+11 Screen")));
    }
    if (channelCount >= 14) {
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_14"), GET_TEXT("LIVEVIEW/20008", "14 Screen")));
    }
    if (channelCount >= 16) {
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_16"), GET_TEXT("LIVEVIEW/20009", "16 Screen")));
    }
    if (channelCount >= 25) {
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_25"), GET_TEXT("LIVEVIEW/20029", "25 Screen")));
    }
    if (channelCount >= 32) {
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_32"), GET_TEXT("LIVEVIEW/20010", "32 Screen")));
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_32_2"), GET_TEXT("LIVEVIEW/20030", "1+31 Screen")));
    }
    //3536g辅屏不支持64分屏
    bool hide64Layout = channelCount < 64 || (qMsNvr->is3536g() && SCREEN_MAIN != SubControl::instance()->currentScreen());
    if (!hide64Layout) {
        multiActionPairs.append(qMakePair(QString("LAYOUTMODE_64"), GET_TEXT("LIVEVIEW/168004", "64 Screen")));
    }

    //
    if (qMsNvr->is5016()) {
        if (LiveViewTarget::instance() && LiveViewTarget::instance()->isTargetEnable()) {
            multiActionPairs.removeAll(qMakePair(QString("LAYOUTMODE_14"), GET_TEXT("LIVEVIEW/20008", "14 Screen")));
            multiActionPairs.removeAll(qMakePair(QString("LAYOUTMODE_16"), GET_TEXT("LIVEVIEW/20009", "16 Screen")));
        }
    }

    //
    int screen = SubControl::instance()->currentScreen();
    for (int i = 0; i < multiActionPairs.size(); ++i) {
        const QPair<QString, QString> &pair = multiActionPairs.at(i);

        QAction *action = new QAction(pair.second, m_multiScreenMenu);
        action->setData(QVariant::fromValue(CustomLayoutKey(pair.first, screen, CustomLayoutKey::DefaultType)));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onMultiScreenActionClicked(bool)));
        m_multiScreenMenu->addAction(action);
    }

    //
    QStringList names = CustomLayoutData::instance()->customLayoutNames(screen);
    //5016机型，target模式下屏蔽通道数大于12的布局
    QStringList realNames = names;
    if (qMsNvr->is5016()) {
        if (LiveView::instance()->isTargetMode()) {
            for (int i = 0; i < names.size(); ++i) {
                const QString &name = names.at(i);
                CustomLayoutKey key(name, screen, CustomLayoutKey::CustomType);
                const CustomLayoutInfo &info = CustomLayoutData::instance()->layoutInfo(key);
                if (info.isValid()) {
                    if (info.positionCount() > 12) {
                        realNames.removeAll(name);
                    }
                } else {
                    qMsWarning() << "invalid key:" << key;
                }
            }
        }
    }
    QFontMetrics fm(m_multiScreenMenu->font());
    for (int i = 0; i < realNames.size(); ++i) {
        const QString &name = realNames.at(i);
        QAction *action = new QAction(name, m_multiScreenMenu);
        action->setToolTip(name);
        action->setData(QVariant::fromValue(CustomLayoutKey(name, screen, CustomLayoutKey::CustomType)));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onMultiScreenActionClicked(bool)));
        m_multiScreenMenu->addAction(action);
    }
}

void LiveView::setLayoutPage(int screen, int page)
{
    QString name;
    int type = 0;
    switch (screen) {
    case SCREEN_MAIN:
        name = QString(m_mainCurrentLayout.name);
        type = m_mainCurrentLayout.type;
        break;
    case SCREEN_SUB:
        name = QString(m_subCurrentLayout.name);
        type = m_subCurrentLayout.type;
        break;
    default:
        qMsWarning() << QString("invalid screen: %1").arg(screen);
        return;
    }
    setLayoutMode(name, screen, type, page);
}

void LiveView::hideToolBar()
{
    if (m_videoBar) {
        m_videoBar->hide();
    }
    if (m_mainMenu->isVisible()) {
        m_mainMenu->animateHide();
    }
}

void LiveView::closeAllPopupWindow()
{
    closeAllPopupWindowWithoutToolBar();
    //硬解鱼眼模式只有通道删除太退出，断流不退出
#ifdef MS_FISHEYE_SOFT_DEWARP
#else
    if (m_fisheye) {
        m_fisheye->close();
    }
#endif
    //video bar
    if (m_videoBar && m_videoBar->isVisible()) {
        m_videoBar->close();
    }
    //
    m_contextMenu->close();
    //
    PopupContent::instance()->closePopupWidget(BasePopup::CloseNormal);
}

void LiveView::closeAllPopupWindowWithoutToolBar()
{
    //image
    if (m_image && m_image->isVisible()) {
        m_image->close();
    }
    //alarm output
    if (m_alarmout && m_alarmout->isVisible()) {
        m_alarmout->close();
    }
    //liveview playback
    if (m_playback && m_playback->isVisible()) {
        m_playback->closePlaybackForPopup();
    }
    //popup时退出zoom
    if (LiveViewZoom::instance() && LiveViewZoom::instance()->isVisible()) {
        LiveViewZoom::instance()->closeZoom();
    }
    //ptz
    if (m_ptz && m_ptz->isVisible()) {
        m_ptz->exitPtz(PtzControl::ExitForPopup);
    }
    //popup时退出ptz 3d
    if (PTZ3DControl::instance()) {
        PTZ3DControl::instance()->closePtz3D(PTZ3DControl::ExitForRightButton);
    }
    //fisheye
#ifdef MS_FISHEYE_SOFT_DEWARP
    if (m_fisheyeDewarpControl) {
        m_fisheyeDewarpControl->closeFisheyeDewarp();
    }
#endif
    //
    if (m_optimal && m_optimal -> isVisible()) {
        m_optimal->close();
    }
}

void LiveView::hideAllVideoAdd()
{
    for (auto iter = m_mapChannelVideo.constBegin(); iter != m_mapChannelVideo.constEnd(); ++iter) {
        LiveVideo *tempVideo = iter.value();
        tempVideo->showVideoAdd(false);
    }
}

int LiveView::streamInfo()
{
    LiveVideo *video = videoFromChannel(m_currentVideo->channel());
    return video->streamFormat();
}

bool LiveView::isManualChangeStream() const
{
    return m_isManualChangeStream;
}

int LiveView::BottomBarMode() const
{
    return m_bottomBar->mode();
}

void LiveView::onSetLayoutMode(int mode)
{
    CustomLayoutData::instance()->clearStreamType();
    QString name = CustomLayoutData::instance()->nameFromDefaultLayoutMode(mode);
    SCREEN_E screen = SubControl::instance()->currentScreen();
    setLayoutMode(name, screen, CustomLayoutKey::DefaultType, 0);
}

void LiveView::onSetLayoutMode(const QString &name)
{
    CustomLayoutData::instance()->clearStreamType();
    SCREEN_E screen = SubControl::instance()->currentScreen();
    setLayoutMode(name, screen, CustomLayoutKey::CustomType, 0);
}

void LiveView::onSetLayoutMode(const CustomLayoutKey &key)
{
    CustomLayoutData::instance()->clearStreamType();
    setLayoutMode(key, 0);
}

void LiveView::setLayoutMode()
{
    setLayoutMode(SubControl::instance()->currentScreen());
}

void LiveView::setLayoutMode(SCREEN_E screen)
{
    QString name;
    int type = 0;
    int page = 0;
    switch (screen) {
    case SCREEN_MAIN:
        name = QString(m_mainCurrentLayout.name);
        type = m_mainCurrentLayout.type;
        page = m_mainCurrentLayout.page;
        break;
    case SCREEN_SUB:
        name = QString(m_subCurrentLayout.name);
        type = m_subCurrentLayout.type;
        page = m_subCurrentLayout.page;
        break;
    default:
        qMsWarning() << QString("invalid screen: %1").arg(screen);
        return;
    }
    setLayoutMode(name, screen, type, page);
}

void LiveView::setLayoutMode(const QString &name, int screen, int type, int page)
{
    setLayoutMode(CustomLayoutKey(name, screen, type), page);
}

void LiveView::setLayoutMode(CustomLayoutKey key, int page)
{
    CustomLayoutInfo info = CustomLayoutData::instance()->layoutInfo(key);
    if (!info.isValid()) {
        qMsWarning() << "not found layout:" << key << ", use LAYOUTMODE_4";
        key = CustomLayoutKey("LAYOUTMODE_4", key.screen(), CustomLayoutKey::DefaultType);
        info = CustomLayoutData::instance()->layoutInfo(key);
    }

    /********
     * 5016系列只有16个解码器，如果在ANPR模式下预览16屏，播放ANPR小视频时一定会存在解码器不足的情况；
     * 为了保证用户的使用体验，在5016系列里进入ANPR模式时，窗格最高保留到12个窗格（横3*竖4），
     * 默认直接释放4个解码器的资源备用，此时14/16窗格的图标去掉：
     ********/
    if (qMsNvr->is5016()) {
        if (ui->widget_target->isTargetEnable()) {
            if (info.positionCount() > 12) {
                key = CustomLayoutKey("LAYOUTMODE_12", key.screen(), CustomLayoutKey::DefaultType);
                info = CustomLayoutData::instance()->layoutInfo(key);
            }
        }
    }

    //
    int realPage = 0;
    int pageCount = 0;

    //
    switch (key.screen()) {
    case SCREEN_MAIN:
        //
        m_currentMainPageMap = info.pageMap();
        if (page >= m_currentMainPageMap.size()) {
            page = 0;
        }
        realPage = m_currentMainPageMap.value(page);
        pageCount = m_currentMainPageMap.size();
        //
        snprintf(m_mainCurrentLayout.name, sizeof(m_mainCurrentLayout.name), "%s", key.name().toStdString().c_str());
        m_mainCurrentLayout.page = page;
        m_mainCurrentLayout.type = key.type();
        //
        QThreadPool::globalInstance()->start(new RunnableSaveLayout(&m_mainCurrentLayout));
        break;
    case SCREEN_SUB:
        //
        m_currentSubPageMap = info.pageMap();
        if (page >= m_currentSubPageMap.size()) {
            page = 0;
        }
        realPage = m_currentSubPageMap.value(page);
        pageCount = m_currentSubPageMap.size();
        //
        snprintf(m_subCurrentLayout.name, sizeof(m_mainCurrentLayout.name), "%s", key.name().toStdString().c_str());
        m_subCurrentLayout.page = page;
        m_subCurrentLayout.type = key.type();
        //
        QThreadPool::globalInstance()->start(new RunnableSaveLayout(&m_subCurrentLayout));
        break;
    }

    if (pageCount < 1) {
        pageCount = 1;
    }

    //
    if (key.screen() == SubControl::instance()->currentScreen()) {
        //
        ui->videoContainerManager->setLayoutMode(info, realPage);
        //
        m_page->setPage(page, pageCount);
        //
        m_videoFrame->hide();
        m_videoBar->hide();
        m_currentVideo = nullptr;
        //
        emit layoutChanged(key);
    } else {
        if (LiveViewSub::isSubEnable()) {
            //
            LiveViewSub::instance()->setLayoutMode(info, realPage);
            //
            LiveViewSub::instance()->showPage(page, pageCount);
        }
    }

    //
    if (info.isSingleLayout()) {
        int channel = info.channel(realPage);
        switch (key.screen()) {
        case SCREEN_MAIN:
            setCurrentMainFullScreenChannel(channel);
            break;
        case SCREEN_SUB:
            setCurrentSubFullScreenChannel(channel);
            break;
        }
    } else {
        switch (key.screen()) {
        case SCREEN_MAIN:
            setCurrentMainFullScreenChannel(-1);
            break;
        case SCREEN_SUB:
            setCurrentSubFullScreenChannel(-1);
            break;
        }
    }
}

int LiveView::currentPage() const
{
    if (SubControl::instance()->isSubControl()) {
        return m_subCurrentLayout.page;
    } else {
        return m_mainCurrentLayout.page;
    }
}

bool LiveView::isSingleLayout(int screen) const
{
    switch (screen) {
    case SCREEN_MAIN:
        return CustomLayoutData::instance()->isSingleLayout(CustomLayoutKey(m_mainCurrentLayout.name, m_mainCurrentLayout.screen, m_mainCurrentLayout.type));
    case SCREEN_SUB:
        return CustomLayoutData::instance()->isSingleLayout(CustomLayoutKey(m_subCurrentLayout.name, m_subCurrentLayout.screen, m_subCurrentLayout.type));
    default:
        qMsWarning() << "inalid screen:" << screen;
        return false;
    }
}

CustomLayoutKey LiveView::currentLayoutMode()
{
    if (SubControl::instance()->isSubControl()) {
        return CustomLayoutKey(m_subCurrentLayout.name, m_subCurrentLayout.screen, m_subCurrentLayout.type);
    } else {
        return CustomLayoutKey(m_mainCurrentLayout.name, m_mainCurrentLayout.screen, m_mainCurrentLayout.type);
    }
}

CustomLayoutKey LiveView::currentLayoutMode(int screen)
{
    switch (screen) {
    case SCREEN_MAIN:
        return CustomLayoutKey(m_mainCurrentLayout.name, m_mainCurrentLayout.screen, m_mainCurrentLayout.type);
    case SCREEN_SUB:
        return CustomLayoutKey(m_subCurrentLayout.name, m_subCurrentLayout.screen, m_subCurrentLayout.type);
    default:
        qMsWarning() << QString("invalid screen: %1").arg(screen);
        return CustomLayoutKey();
    }
}

int LiveView::currentLayoutWindowCount()
{
    const CustomLayoutKey &key = currentLayoutMode();
    const CustomLayoutInfo &info = CustomLayoutData::instance()->layoutInfo(key);
    return info.positionCount();
}

int LiveView::currentMainFullScreenChannel() const
{
    QMutexLocker locker(&s_mutex);
    return m_currentMainFullScreenChannel;
}

void LiveView::setCurrentMainFullScreenChannel(int channel)
{
    QMutexLocker locker(&s_mutex);
    m_currentMainFullScreenChannel = channel;
}

int LiveView::currentSubFullScreenChannel() const
{
    QMutexLocker locker(&s_mutex);
    return m_currentSubFullScreenChannel;
}

void LiveView::setCurrentSubFullScreenChannel(int channel)
{
    QMutexLocker locker(&s_mutex);
    m_currentSubFullScreenChannel = channel;
}

bool LiveView::checkDynamicDisplayChannel(int channel)
{
    QMutexLocker locker(&s_mutex);
    if (m_currentMainFullScreenChannel == channel || m_currentSubFullScreenChannel == channel) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief LiveView::nextMainPage
 * 物理意义的主屏
 */
void LiveView::nextMainPage()
{
    if (SCREEN_MAIN == SubControl::instance()->currentScreen()) {
        //控制在物理主屏
        if (!isVisible()) {
            return;
        }
        if (m_image && m_image->isVisible()) {
            return;
        }
        if (m_alarmout && m_alarmout->isVisible()) {
            return;
        }
        if (m_ptz && m_ptz->isVisible()) {
            return;
        }
        if (m_ptz3d && m_ptz3d->isVisible()) {
            return;
        }
        if (EventPopup::instance() && EventPopup::instance()->isVisible()) {
            return;
        }
        if (m_optimal && m_optimal -> isVisible()) {
            return;
        }
#ifdef MS_FISHEYE_SOFT_DEWARP
#else
        if (m_fisheye && m_fisheye->isVisible()) {
            return;
        }
#endif
    } else {
        //控制在物理辅屏
        if (EventPopupSub::instance() && EventPopupSub::instance()->isVisible()) {
            return;
        }
    }

    //
    CustomLayoutKey key(m_mainCurrentLayout.name, m_mainCurrentLayout.screen, m_mainCurrentLayout.type);
    CustomLayoutInfo info = CustomLayoutData::instance()->layoutInfo(key);
    if (info.pageMap().count() < 2) {
        return;
    }
    int page = m_mainCurrentLayout.page;
    if (page == m_currentMainPageMap.size() - 1) {
        page = 0;
    } else {
        page++;
    }
    setLayoutPage(SCREEN_MAIN, page);

    //
    if (SCREEN_MAIN == SubControl::instance()->currentScreen()) {
        m_page->showPage(2000);
    }
}

/**
 * @brief LiveView::nextSubPage
 * 物理意义的辅屏
 */
void LiveView::nextSubPage()
{
    if (SCREEN_SUB == SubControl::instance()->currentScreen()) {
        //控制在物理辅屏
        if (!isVisible()) {
            return;
        }
        if (m_image && m_image->isVisible()) {
            return;
        }
        if (m_alarmout && m_alarmout->isVisible()) {
            return;
        }
        if (m_ptz && m_ptz->isVisible()) {
            return;
        }
        if (m_ptz3d && m_ptz3d->isVisible()) {
            return;
        }
        if (EventPopup::instance() && EventPopup::instance()->isVisible()) {
            return;
        }
        if (m_optimal && m_optimal -> isVisible()) {
            return;
        }
#ifdef MS_FISHEYE_SOFT_DEWARP
#else
        if (m_fisheye && m_fisheye->isVisible()) {
            return;
        }
#endif
    } else {
        //控制在物理主屏
        if (EventPopupSub::instance() && EventPopupSub::instance()->isVisible()) {
            return;
        }
    }

    //
    CustomLayoutKey key(m_subCurrentLayout.name, m_subCurrentLayout.screen, m_subCurrentLayout.type);
    CustomLayoutInfo info = CustomLayoutData::instance()->layoutInfo(key);
    if (info.pageMap().count() < 2) {
        return;
    }
    int page = m_subCurrentLayout.page;
    if (page == m_currentSubPageMap.size() - 1) {
        page = 0;
    } else {
        page++;
    }
    setLayoutPage(SCREEN_SUB, page);

    //
    if (SCREEN_SUB == SubControl::instance()->currentScreen()) {
        m_page->showPage(2000);
    }
}

/**
 * @brief LiveView::initializeSubScreenLayout
 * 开启辅屏时先设置辅屏布局
 */
void LiveView::initializeSubScreenLayout()
{
    if (SubControl::instance()->isSubControl()) {
        setLayoutMode(SCREEN_MAIN);
    } else {
        setLayoutMode(SCREEN_SUB);
    }
}

void LiveView::stopSubScreen()
{
    // if (SubControl::instance()->isSubControl()) {
    //     ms_vapi_clear_screen(SCREEN_MAIN, 0);
    //     qDebug() << QString("LiveView::stopSubScreen, SCREEN_MAIN");
    // } else {
    //     ms_vapi_clear_screen(SCREEN_SUB, 0);
    //     qDebug() << QString("LiveView::stopSubScreen, SCREEN_SUB");
    // }
}

void LiveView::switchScreen()
{
    if (PopupContent::instance() && PopupContent::instance()->isVisible()) {
        PopupContent::instance()->closePopupWidget(BasePopup::CloseWithRightButton);
    }
    if (PTZ3DControl::instance() && PTZ3DControl::instance()->isVisible()) {
        PTZ3DControl::instance()->closePtz3DForSwitchScreen();
    }
#ifdef MS_FISHEYE_SOFT_DEWARP
    if (m_fisheyeDewarpControl) {
        m_fisheyeDewarpControl->closeFisheyeDewarp();
    }
#else
    if (FisheyeControl::instance() && FisheyeControl::instance()->isVisible()) {
        FisheyeControl::instance()->closePtz3DForSwitchScreen();
    }
#endif
    if (m_playback && m_playback->isVisible()) {
        m_playback->closePlaybackForPopup();
    }
    if (LiveViewZoom::instance() && LiveViewZoom::instance()->isVisible()) {
        LiveViewZoom::instance()->closeZoom();
    }
    if (LiveViewTargetPlay::instance() && LiveViewTargetPlay::instance()->isVisible()) {
        LiveViewTargetPlay::instance()->closeAnprPlayback();
    }

    //
    if (SubControl::instance()->isSubControl()) {
        //当前在次屏，准备切换到主屏
        SubControl::instance()->setSubControl(false);
        SubControl::instance()->switchFrameBuffer("/dev/fb0");
    } else {
        //当前在主屏，准备切换到次屏
        SubControl::instance()->setSubControl(true);
        SubControl::instance()->switchFrameBuffer("/dev/fb1");
    }

    SCREEN_E screen = SubControl::instance()->currentScreen();

    //TODO 改成Runnable
    struct display db_display = qMsNvr->displayInfo();
    db_display.start_screen = screen;
    qMsNvr->writeDisplayInfo(&db_display);

    //
    if (isVisible()) {
        setLayoutMode();
    } else {
        stopLiveView(screen);
    }
    if (SubControl::instance()->isSubEnable()) {
        if (SubControl::instance()->isSubControl()) {
            setLayoutMode(SCREEN_MAIN);
        } else {
            setLayoutMode(SCREEN_SUB);
        }
    }

    //
    sendMessageOnly(REQUEST_FLAG_SET_REFRESH_LAYOUT, (void *)NULL, 0);
}

/**
 * @brief LiveView::canShowPopup
 * 是否可以显示报警弹框
 * @return
 */
bool LiveView::canShowPopup()
{
    if (isVisible()) {
        return true;
    }
    if (LiveViewZoom::instance() && LiveViewZoom::instance()->isVisible()) {
        return true;
    }
    return false;
}

void LiveView::preparePopup(int screen, int layout, const QList<int> &channels)
{
    qMsDebug() << "screen:" << screen << "layout:" << layout << "channels:" << channels;
    //MsWaitting::showGlobalWait(this);
    //一些窗口要先关闭
    if (screen == SubControl::instance()->currentScreen()) {
        closeAllPopupWindow();
    }

    //
    EventPopupInfo info;
    info.screen = screen;
    info.layout = layout;
    info.channels = channels;
    QMetaObject::invokeMethod(this, "onShowPopup", Qt::QueuedConnection, Q_ARG(EventPopupInfo, info));
}

void LiveView::recoverLayoutBeforePopup(int screen)
{
    if (SubControl::instance()->mainLiveViewScreen() == screen && !isVisible()) {
        return;
    }
    if (SubControl::instance()->subLiveViewScreen() == screen && LiveViewSub::instance() && !LiveViewSub::instance()->isVisible()) {
        return;
    }

    switch (screen) {
    case SCREEN_MAIN:
        if (QString(m_mainLayoutBeforePopup.name).isEmpty()) {
            return;
        }
        setLayoutMode(CustomLayoutKey(m_mainLayoutBeforePopup.name, m_mainLayoutBeforePopup.screen, m_mainLayoutBeforePopup.type), m_mainLayoutBeforePopup.page);
        memset(&m_mainLayoutBeforePopup, 0, sizeof(layout_custom));
        break;
    case SCREEN_SUB:
        if (QString(m_subLayoutBeforePopup.name).isEmpty()) {
            return;
        }
        setLayoutMode(CustomLayoutKey(m_subLayoutBeforePopup.name, m_subLayoutBeforePopup.screen, m_subLayoutBeforePopup.type), m_subLayoutBeforePopup.page);
        memset(&m_subLayoutBeforePopup, 0, sizeof(layout_custom));
        break;
    default:
        break;
    }

    //
    m_bottomBar->resume();
}

void LiveView::showFullChannel(int channel)
{
    CustomLayoutData::instance()->clearStreamType();

    //
    CustomLayoutKey key("LAYOUTMODE_1", SCREEN_MAIN, CustomLayoutKey::DefaultType);

    //
    if (SubControl::instance()->isSubControl()) {
        key.setScreen(SCREEN_SUB);
        memcpy(&m_subLayoutBeforeFull, &m_subCurrentLayout, sizeof(layout_custom));
        if (QString(m_subLayoutBeforeFull.name).isEmpty()) {
            m_subLayoutBeforeFull.screen = SCREEN_SUB;
            m_subLayoutBeforeFull.page = 0;
            m_subLayoutBeforeFull.type = CustomLayoutKey::DefaultType;
            snprintf(m_subLayoutBeforeFull.name, sizeof(m_subLayoutBeforeFull.name), "%s", "LAYOUTMODE_4");
        }
    } else {
        key.setScreen(SCREEN_MAIN);
        memcpy(&m_mainLayoutBeforeFull, &m_mainCurrentLayout, sizeof(layout_custom));
        if (QString(m_mainLayoutBeforeFull.name).isEmpty()) {
            m_mainLayoutBeforeFull.screen = SCREEN_SUB;
            m_mainLayoutBeforeFull.page = 0;
            m_mainLayoutBeforeFull.type = CustomLayoutKey::DefaultType;
            snprintf(m_mainLayoutBeforeFull.name, sizeof(m_mainLayoutBeforeFull.name), "%s", "LAYOUTMODE_4");
        }
    }

    //
    const CustomLayoutInfo &info = CustomLayoutData::instance()->layoutInfo(key);
    int page = info.pageOfChannel(channel);
    if (page < 0) {
        qMsWarning() << QString("invalid page: %1, channel: %2").arg(page).arg(channel);
        return;
    }
    setLayoutMode(key, page);

    //全屏后默认当前video
    m_currentVideo = videoFromChannel(channel);
}

void LiveView::closeFullChannel()
{
    CustomLayoutData::instance()->clearStreamType();

    //
    if (SubControl::instance()->isSubControl()) {
        setLayoutMode(QString(m_subLayoutBeforeFull.name), m_subLayoutBeforeFull.screen, m_subLayoutBeforeFull.type, m_subLayoutBeforeFull.page);
    } else {
        setLayoutMode(QString(m_mainLayoutBeforeFull.name), m_mainLayoutBeforeFull.screen, m_mainLayoutBeforeFull.type, m_mainLayoutBeforeFull.page);
    }
}

void LiveView::videoClicked(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (!video) {
        qMsWarning() << QString("video is nullptr, channel: %1").arg(channel);
        return;
    }

    //
#ifdef MS_FISHEYE_SOFT_DEWARP

#else
    if (qMsNvr->isFisheye(channel)) {
        ////MsWaitting::showGlobalWait(video);
        gPtzDataManager->beginGetData(channel);
        gPtzDataManager->waitForGetFisheyeInfo();
        ////MsWaitting::closeGlobalWait();
        if (gPtzDataManager->isBundleFisheye() && isChannelConnected(channel)) {
            m_videoBar->setFisheyeEnable(true);
        } else {
            m_videoBar->setFisheyeEnable(false);
        }
    } else {
        m_videoBar->setFisheyeEnable(false);
    }
#endif

    //右侧主菜单隐藏
    if (m_mainMenu->isVisible()) {
        m_mainMenu->animateHide();
    }
    //底部菜单栏隐藏
    if (m_bottomBar->isVisible()) {
        if (!m_bottomBar->isLocked()) {
            m_bottomBar->animateHide();
        }
    }

    //
    m_currentVideo = video;

    hideAllVideoAdd();
    ui->videoContainerManager->hideAllLayoutButton();

    QRect rc = m_currentVideo->globalGeometry();
    m_videoFrame->setGeometry(rc);
    m_videoFrame->show();

    if (!m_currentVideo->isChannelEnable()) {
        if (!qMsNvr->isSlaveMode()) {
            m_currentVideo->showVideoAdd(true);
        }
        m_videoBar->hide();
        return;
    }

    //先show才能正确获取到大小
    m_videoBar->showVideoBar(m_currentVideo->channel());
    QRect screenRect = SubControl::instance()->logicalMainScreenGeometry();
    QPoint p(rc.right() - rc.width() / 2 - m_videoBar->realWidth() / 2, rc.bottom() + 1);
    if (p.y() + m_videoBar->height() > screenRect.bottom()) {
        p.setY(rc.top() - m_videoBar->height());
    }
    if (p.x() < screenRect.left()) {
        p.setX(screenRect.left());
    }
    if (p.x() + m_videoBar->realWidth() > screenRect.right()) {
        p.setX(screenRect.right() - m_videoBar->realWidth());
    }
    //
    if (p.y() < screenRect.top()) {
        p.setY(screenRect.top() + 1);
    }
    //
    m_videoBar->move(p);
    m_videoBar->updateState(LiveVideo::state(channel));
    m_videoBar->setFocus();
    m_videoBar->raise();
}

void LiveView::videoDoubleClicked(int channel)
{
    if (CustomLayoutData::instance()->isSingleLayout(currentLayoutMode())) {
        qMsDebug() << QString("close full channel: %1").arg(channel);
        closeFullChannel();
    } else {
        qMsDebug() << QString("show full channel: %1").arg(channel);
        LiveVideo *video = videoFromChannel(channel);
        if (!video) {
            qMsWarning() << QString("video is nullptr, channel: %1").arg(channel);
            return;
        }
        if (!video->isChannelValid()) {
            return;
        }
        if (!video->isChannelEnable()) {
            return;
        }
        //
        showFullChannel(channel);
    }
}

void LiveView::onVideoAddButtonClicked(int channel)
{
    Q_UNUSED(channel)
    LiveViewSearch *m_liveViewSearch = nullptr;
    m_liveViewSearch = new LiveViewSearch(SettingContent::instance());
    m_liveViewSearch->initializeData();
    m_liveViewSearch->exec();
    //
    showLiveView();
}

void LiveView::onVideoContainerClicked(int channel)
{
    VideoContainer *container = static_cast<VideoContainer *>(sender());

    m_currentVideo = nullptr;
    QRect rc = container->globalGeometry();
    m_videoFrame->setGeometry(rc);
    m_videoFrame->show();

    hideAllVideoAdd();
    ui->videoContainerManager->hideAllLayoutButton();

    if (channel < 0) {
        container->showLayoutButton();
    }

    //
    m_videoBar->hide();
    //右侧主菜单隐藏
    if (m_mainMenu->isVisible()) {
        m_mainMenu->animateHide();
    }
    //底部菜单栏隐藏
    if (m_bottomBar->isVisible()) {
        if (!m_bottomBar->isLocked()) {
            m_bottomBar->animateHide();
        }
    }
}

void LiveView::onCameraStatus(QMap<int, resp_camera_status> map)
{
    LiveVideo::clearStreamInfo();
    for (auto iter = map.constBegin(); iter != map.constEnd(); ++iter) {
        int channel = iter.key();
        const resp_camera_status &camera_status = iter.value();
        LiveVideo::setStreamInfo(channel, camera_status);

        LiveVideo *video = videoFromChannel(channel);
        if (video) {
            if (camera_status.chnid < 0) {
                if (video == m_currentVideo) {
                    video->showVideoAdd(true);
                    m_videoBar->hide();
#ifdef MS_FISHEYE_SOFT_DEWARP
#else
                    if (m_fisheye) {
                        m_fisheye->close();
                    }
#endif
                }
            } else {
                if (video == m_currentVideo) {
                    video->showVideoAdd(false);
                }
            }
            video->updateStreamInfo();
        }

        //
        if (LiveViewSub::isSubEnable()) {
            LiveViewSub::instance()->updateStreamInfo(channel);
        }
    }
}

void LiveView::onCustomContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos)
    //qDebug() << "LiveView::onCustomContextMenuRequested," << pos;

    switch (m_showReason) {
    case LogoutShowReason:
        return;
    default:
        break;
    }

    if (LiveVideo::isDraging()) {
        return;
    }
    //
    if (m_videoFrame->isVisible()) {
        m_videoFrame->hide();
    }
    if (m_videoBar->isVisible()) {
        m_videoBar->hide();
    }

    if (SubControl::instance()->isSubEnable()) {
        if (SubControl::instance()->isSubControl()) {
            m_actionSubControl->setText(GET_TEXT("LIVEVIEW/20016", "Main Screen Ctrl"));
        } else {
            m_actionSubControl->setText(GET_TEXT("LIVEVIEW/20015", "Sub Screen Ctrl"));
        }
        m_contextMenu->insertAction(m_actionLogout, m_actionSubControl);
    } else {
        m_contextMenu->removeAction(m_actionSubControl);
    }

    m_actionLogout->setText(GET_TEXT("LIVEVIEW/20017", "Logout(%1)").arg(gMsUser.userName()));

    //
    updateMultiScreenMenu();

    //
    m_contextMenu->exec(QCursor::pos());
}

void LiveView::onBottomBarVisibleChanged(bool visible)
{
    if (m_timeWidget) {
        if (m_timeWidget->mode() == 0 || !isVisible()) {
            m_timeWidget->hide();
            return;
        }
        m_timeWidget->setVisible(!visible);
        m_timeWidget->raise();
    }
}

void LiveView::setTimeInfoMode(int mode)
{
    qMsDebug() << "mode:" << mode;
    switch (mode) {
    case 0: //Auto
    {
        if (m_timeWidget) {
            m_timeWidget->hide();
        }
        break;
    }
    case 1: //Always
    {
        if (!m_timeWidget) {
            m_timeWidget = new TimeWidget(ContentLiveView::instance());
        }
        QRect rc = geometry();
        m_timeWidget->move(rc.right() - m_timeWidget->width(), rc.bottom() - m_timeWidget->height() + 2);
        if (BottomBar::instance() && !BottomBar::instance()->isVisible()) {
            m_timeWidget->show();
            m_timeWidget->raise();
        }
        break;
    }
    case 2: //Off
    {
        if (m_timeWidget) {
            m_timeWidget->hide();
        }
        break;
    }
    default:
        break;
    }
    //
    if (m_timeWidget) {
        m_timeWidget->setMode(mode);
    }
}

void LiveView::resetTimeInfoMode()
{
    int mode = qMsNvr->displayInfo().time_info;
    setTimeInfoMode(mode);
}

void LiveView::updateOccupancyMenu()
{
    if (LiveViewOccupancyManager::instance()->isOccupancyMode()) {
        m_actionOccupancyMode->setChecked(true);
    } else {
        m_actionOccupancyMode->setChecked(false);
    }
    //
    if (qMsNvr->isSupportTargetMode()) {
        if (qMsNvr->isTargetMode()) {
            m_actionRegularMode->setChecked(false);
            m_actionTargetMode->setChecked(true);
        } else {
            m_actionRegularMode->setChecked(true);
            m_actionTargetMode->setChecked(false);
        }
    }
}

int LiveView::networkCurrentChannel()
{
    int channel = -1;

    if (m_currentVideo) {
        channel = m_currentVideo->channel();
    } else {
        switch (SubControl::instance()->currentScreen()) {
        case SCREEN_MAIN:
            channel = currentMainFullScreenChannel();
            break;
        case SCREEN_SUB:
            channel = currentSubFullScreenChannel();
            break;
        default:
            break;
        }
    }
    return channel;
}

void LiveView::onActionRegularModeClicked(bool checked)
{
    if (!qMsNvr->isSupportTargetMode()) {
        return;
    }

    if (checked) {
        m_actionRegularMode->setChecked(true);
        m_actionTargetMode->setChecked(false);
        showRegularMode();
        resetCurrentScreenLayout();
        resetAnotherScreenLayout();
    } else {
        m_actionRegularMode->setChecked(true);
        m_actionTargetMode->setChecked(false);
    }
    LiveviewBottomBar::instance()->updateTargetState();
}

void LiveView::onActionTargetModeClicked(bool checked)
{
    if (!qMsNvr->isSupportTargetMode()) {
        return;
    }

    if (checked) {
        m_actionRegularMode->setChecked(false);
        m_actionTargetMode->setChecked(true);
        showTargetMode();
        resetCurrentScreenLayout();
        resetAnotherScreenLayout();
    } else {
        m_actionRegularMode->setChecked(false);
        m_actionTargetMode->setChecked(true);
    }
    LiveviewBottomBar::instance()->updateTargetState();
}

void LiveView::onActionOccupancyModeClicked(bool checked)
{
    if (checked) {
        LiveViewOccupancyManager::instance()->showOccupancy(LiveViewOccupancyManager::NormalReason);
    } else {
        LiveViewOccupancyManager::instance()->closeOccupancy(LiveViewOccupancyManager::NormalReason);
    }
}

void LiveView::onSetVideoRatio(int type)
{
    struct req_all_video_play_ratio ratio;
    memset(&ratio, 0, sizeof(ratio));
    ratio.ratioType = type;
    ratio.enScreen = SubControl::instance()->currentScreen();
    ratio.winNum = currentLayoutWindowCount();
    qDebug() << QString("REQUEST_FLAG_SET_ALLWIN_RATIO, type: %1, screen: %2, winNum: %3").arg(ratio.ratioType).arg(ratio.enScreen).arg(ratio.winNum);
    sendMessage(REQUEST_FLAG_SET_ALLWIN_RATIO, (void *)&ratio, sizeof(ratio));

    //
    int screen = SubControl::instance()->currentScreen();
    for (auto iter = m_mapChannelVideo.constBegin(); iter != m_mapChannelVideo.constEnd(); ++iter) {
        LiveVideo *video = iter.value();
        video->setVideoRatio(screen, (RATIO_E)type);
    }

    if (m_currentVideo) {
        int channel = m_currentVideo->channel();
        m_videoBar->updateState(LiveVideo::state(channel));
    }
}

void LiveView::onCameraStateChanged(CameraData::State state)
{
    if (state.livepb == DSP_MODE_LIVE) {
        LiveVideo *video = videoFromChannel(state.chanid);
        if (!video) {
            return;
        }
        if (!video->isVisible()) {
            return;
        }
        if (video->layoutIndex() < 0) {
            qMsCritical() << "invalid sid:" << video->layoutIndex();
            return;
        }
        //radio
        if (video->videoRatio(video->channel(), video->screen()) != RATIO_VO_FULL && !video->isConnected()) {
            setVideoRatio(RATIO_VO_FULL, state.chanid);
        }
        //更新VideoBar状态
        if (m_videoBar && m_videoBar->isVisible() && m_currentVideo) {
            if (state.state == RTSP_CLIENT_CONNECT) {
                m_videoBar->updataStream();
            }
        }
        initializeVideoBar();
        video->updateVideoScene();

        //手动切换到次码流时，关闭次码流则切换到主码流，次码流打开时再恢复到次码流
        if (state.mainsub == 0) {
            int stream = CustomLayoutData::instance()->customStream(state.chanid);
            if (stream != STREAM_TYPE_SUBSTREAM) {
                return;
            }
            //不重复设置
            if (m_streamStateMap.contains(state.chanid) && state == m_streamStateMap.value(state.chanid)) {
              return;
            } else {
              m_streamStateMap.insert(state.chanid, state);
            }

            SCREEN_E screen = SubControl::instance()->currentScreen();
            req_action_s action;
            memset(&action, 0, sizeof(req_action_s));
            action.chnid = state.chanid;
            action.sid = video->layoutIndex();
            action.enScreen = screen;
            action.avFrom = AV_FROM_LIVE;
            action.avType = AV_TYPE_MAIN;
            if (state.state == RTSP_CLIENT_DISCONNECT) {
                action.avType = AV_TYPE_MAIN;
            } else if (state.state == RTSP_CLIENT_CONNECT) {
                action.avType = AV_TYPE_SUB;
            } else if (state.state == RTSP_CLIENT_RESOLUTION_CHANGE) {
                if (video->streamFormat() == STREAM_TYPE_MAINSTREAM) {
                    action.avType = AV_TYPE_MAIN;
                } else if (video->streamFormat() == STREAM_TYPE_SUBSTREAM) {
                    action.avType = AV_TYPE_SUB;
                }
            }

            qMsWarning() << QString("chnid:%1, mainsub:%2, state:%3, livepb:%4").arg(state.chanid).arg(state.mainsub).arg(state.state).arg(state.livepb);
            qMsWarning() << QString("REQUEST_FLAG_SET_CAMAPARAM, chid:%1, sid:%2, screen:%3, type:%4")
                                .arg(action.chnid)
                                .arg(action.sid)
                                .arg(action.enScreen)
                                .arg(action.avType);
            sendMessage(REQUEST_FLAG_SET_CAMAPARAM, &action, sizeof(req_action_s));
        }
    }
}

void LiveView::changeDisplayInfo()
{
    struct display display_info = qMsNvr->displayInfo();
    LiveVideo::setDisplayInfo(display_info);

    for (auto iter = m_mapChannelVideo.constBegin(); iter != m_mapChannelVideo.constEnd(); ++iter) {
        LiveVideo *video = iter.value();
        video->updateDisplayInfo();
    }

    //
    if (LiveViewSub::isSubEnable()) {
        LiveViewSub::instance()->updateDisplayInfo();
    }
}

void LiveView::setDisplayMenuChecked(bool checked)
{
    m_actionInfo->setChecked(checked);
}
