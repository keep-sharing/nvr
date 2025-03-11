#include "VideoContainerManager.h"
#include "CustomLayoutData.h"
#include "LiveVideo.h"
#include "LiveView.h"
#include "LiveViewSub.h"
#include "LogoutChannel.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include "SubControl.h"
#include "VideoContainer.h"
#include "centralmessage.h"
#include "msuser.h"
#include <QDesktopWidget>
#include <QGridLayout>

VideoContainerManager::VideoContainerManager(QWidget *parent)
    : MsWidget(parent)
{
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        VideoContainer *c = new VideoContainer(i, this);
        connect(c, SIGNAL(clicked(int)), LiveView::instance(), SLOT(onVideoContainerClicked(int)));
        m_containers.append(c);
    }

    m_layout = new QGridLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);
}

QList<VideoContainer *> VideoContainerManager::containers() const
{
    return m_containers;
}

void VideoContainerManager::setLogicScreen(VideoContainerManager::LogicScreen screen)
{
    m_logicScreen = screen;
}

int VideoContainerManager::physicalScreen()
{
    int screen = 0;
    switch (m_logicScreen) {
    case LogicMain:
        if (SubControl::instance()->isSubControl()) {
            screen = SCREEN_SUB;
        } else {
            screen = SCREEN_MAIN;
        }
        break;
    case LogicSub:
        if (SubControl::instance()->isSubControl()) {
            screen = SCREEN_MAIN;
        } else {
            screen = SCREEN_SUB;
        }
        break;
    default:
        qMsWarning() << "invalid logic screen:" << m_logicScreen;
        break;
    }
    return screen;
}

void VideoContainerManager::setLayoutMode(const CustomLayoutInfo &info, int page)
{
    dealDesktopSize();

    //先清除旧布局
    for (int i = 0; i < m_containers.size(); ++i) {
        auto *c = m_containers.at(i);
        m_layout->removeWidget(c);
        c->clearVideo();
        c->hide();
    }
    for (int r = 0; r < m_rowCount; ++r) {
        m_layout->setRowStretch(r, 0);
    }
    for (int c = 0; c < m_columnCount; ++c) {
        m_layout->setColumnStretch(c, 0);
    }

    //
    m_rowCount = info.baseRow();
    m_columnCount = info.baseColumn();
    for (int r = 0; r < m_rowCount; ++r) {
        m_layout->setRowStretch(r, 1);
    }
    for (int c = 0; c < m_columnCount; ++c) {
        m_layout->setColumnStretch(c, 1);
    }

    //
    const bool isSingleLayout = info.isSingleLayout();

    //
    const auto &positions = info.positions();
    int index = 0;
    int globalIndex = positions.size() * page;

    //
    struct req_layout2_s reqLayout;
    memset(&reqLayout, 0, sizeof(reqLayout));
    reqLayout.enScreen = static_cast<SCREEN_E>(info.screen());
    reqLayout.enMode = DSP_MODE_LIVE;
    if (reqLayout.enScreen == SubControl::instance()->currentScreen()) {
        reqLayout.enView = LiveView::instance()->isTargetMode() ? VIEW_LPR : VIEW_NORMAL;
    } else {
        reqLayout.enView = VIEW_NORMAL;
    }
    reqLayout.enLayout = static_cast<LAYOUT_E>(0);
    reqLayout.winNum = qMin(positions.size(), qMsNvr->maxChannel());

    //
    if (reqLayout.enView == VIEW_LPR) {
        m_dw = m_nvrScreenSize.width() / 4 * 3 / m_columnCount;
        m_dh = m_nvrScreenSize.height() / m_rowCount;
    } else {
        m_dw = m_nvrScreenSize.width() / m_columnCount;
        m_dh = m_nvrScreenSize.height() / m_rowCount;
    }

    //
    for (auto iter = positions.constBegin(); iter != positions.constEnd(); ++iter) {
        const auto &position = iter.key();
        auto *container = m_containers.at(index);
        m_layout->addWidget(container, position.row, position.column, position.rowSpan, position.columnSpan);
        int channel = info.channel(globalIndex);
        container->setGlobalIndex(globalIndex);
        container->setScreen(info.screen());
        if (channel >= 0) {
            auto *video = liveVideo(channel);
            if (!video) {
                qMsWarning() << QString("video is nullptr, channel: %1").arg(channel);
            } else {
                if (!isSingleLayout) {
                    video->drawScene()->hideVcaRects();
                }
            }
            container->setVideo(video);
        }
        container->hideLayoutButton();
        container->show();

        QRect rc = nvrVideoGeometry(reqLayout.enView, position);

        //
        if (index < qMsNvr->maxChannel()) {
            CHNWIN_S &win = reqLayout.astcw[index];
            if (!gMsUser.hasLiveViewChannelPermission(channel)) {
                win.bshow = 0;
            } else if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel)) {
                win.bshow = 0;
            } else {
                win.bshow = channel < 0 ? 0 : 1;
            }
            win.chnid = channel;
            win.sid = index;
            win.enFrom = AV_FROM_LIVE;
            win.enRatio = LiveVideo::videoRatio(channel, info.screen());
            StreamKey key;
            key.currentChannel(channel);
            key.setScreen(info.screen());
            int result = CustomLayoutData::instance()->streamType(key);
            switch (result) {
            case STREAM_TYPE_MAINSTREAM:
                win.appointid = VAPI_STREAM_ID(channel, AV_FROM_LIVE, AV_TYPE_MAIN, 0);
                break;
            case STREAM_TYPE_SUBSTREAM:
                win.appointid = VAPI_STREAM_ID(channel, AV_FROM_LIVE, AV_TYPE_SUB, 0);
                break;
            default:
                break;
            }
            win.stZone.enMode = ZONE_MODE_USER;
            win.stZone.x = rc.x();
            win.stZone.y = rc.y();
            win.stZone.w = rc.width();
            win.stZone.h = rc.height();
            qMsCDebug("qt_layout") << QString("index: %1, channel: %2").arg(index).arg(channel) << rc;

#ifdef MS_FISHEYE_SOFT_DEWARP
            FisheyeKey fisheyeKey(FisheyeDewarpControl::ModeLiveview, win.chnid, -1, info.screen());
            if (fisheyeKey == FisheyeDewarpControl::fisheyeChannel(FisheyeDewarpControl::ModeLiveview)) {
                win.enFish = static_cast<FISH_E>(FisheyeDewarpControl::fisheyeDisplayMode(fisheyeKey));
            }
#endif
        }

        //
        ++index;
        ++globalIndex;
        if (index >= qMsNvr->maxChannel()) {
            break;
        }
    }

    //
    sendMessageOnly(REQUEST_FLAG_SET_DISLAYOUT, (void *)&reqLayout, sizeof(reqLayout));
}

void VideoContainerManager::swapChannel(int index1, int channel1, int index2, int channel2)
{
    LiveVideo *video1 = LiveView::instance()->liveVideo(channel1);
    LiveVideo *video2 = LiveView::instance()->liveVideo(channel2);
    auto *container1 = m_containers.at(index1);
    auto *container2 = m_containers.at(index2);
    container1->clearVideo();
    container2->clearVideo();
    container1->setVideo(video2);
    container2->setVideo(video1);
}

LiveVideo *VideoContainerManager::videoFromIndex(int index)
{
    if (index >= 0 && index < m_containers.size()) {
        return m_containers.value(index)->video();
    }
    return nullptr;
}

void VideoContainerManager::hideAllLayoutButton()
{
    for (int i = 0; i < m_containers.size(); ++i) {
        VideoContainer *tempContainer = m_containers.at(i);
        tempContainer->hideLayoutButton();
    }
}

void VideoContainerManager::dealDesktopSize()
{
    int nvrWidth = 0;
    int nvrHeight = 0;
    vapi_get_screen_res((SCREEN_E)physicalScreen(), &nvrWidth, &nvrHeight);
    m_nvrScreenSize = QSize(nvrWidth, nvrHeight);
    //
    QRect screenRect = qMsApp->desktop()->geometry();
    m_qtScreenSize = screenRect.size();

    qMsCDebug("qt_layout") << "nvr size:" << m_nvrScreenSize << "qt size:" << m_qtScreenSize;
}

QRect VideoContainerManager::nvrVideoGeometry(int enView, const VideoPosition &p)
{
    QRect rc(m_dw * p.column, m_dh * p.row, m_dw * p.columnSpan, m_dh * p.rowSpan);
    if (enView == VIEW_LPR) {
        rc.moveLeft(m_nvrScreenSize.width() / 4 + rc.left());
    }
    return rc;
}

LiveVideo *VideoContainerManager::liveVideo(int channel)
{
    switch (m_logicScreen) {
    case LogicMain:
        return LiveView::instance()->liveVideo(channel);
    case LogicSub:
        return LiveViewSub::instance()->liveVideo(channel);
    default:
        return nullptr;
    }
}
