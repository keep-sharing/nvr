#include "PlaybackLayoutSplit.h"
#include "ui_PlaybackLayoutSplit.h"
#include "MyDebug.h"
#include "PlaybackData.h"
#include "PlaybackVideoBar.h"
#include "centralmessage.h"
#include "PlaybackBar.h"
#include "PlaybackSplit.h"
#include "PlaybackTimeLine.h"
#include "SmartSearchControl.h"
#include "SubControl.h"
#include <QPainter>

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "FisheyeDewarpControl.h"
#endif

PlaybackLayoutSplit *PlaybackLayoutSplit::s_splitLayout = nullptr;

PlaybackLayoutSplit::PlaybackLayoutSplit(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackLayout_Split)
{
    ui->setupUi(this);

    s_splitLayout = this;
}

PlaybackLayoutSplit::~PlaybackLayoutSplit()
{
    s_splitLayout = nullptr;
    delete ui;
}

PlaybackLayoutSplit *PlaybackLayoutSplit::instance()
{
    return s_splitLayout;
}

void PlaybackLayoutSplit::setSplitLayout(int mode, int selectedSid)
{
    PlaybackSplit *playbackSplit = PlaybackSplit::instance();
    Q_ASSERT(playbackSplit);

    int channel = playbackSplit->channel();

    clearGridLayout();

    int rowCount = 0;
    int columnCount = 0;
    LAYOUT_E layout_e;
    switch (mode) {
    case SplitLayout_0:
        rowCount = 0;
        columnCount = 0;
        layout_e = LAYOUT_NUM;
        break;
    case SplitLayout_4:
        rowCount = 2;
        columnCount = 2;
        layout_e = LAYOUT_2X2;
        break;
    case SplitLayout_9:
        rowCount = 3;
        columnCount = 3;
        layout_e = LAYOUT_3X3;
        break;
    case SplitLayout_16:
        rowCount = 4;
        columnCount = 4;
        layout_e = LAYOUT_4X4;
        break;
    default:
        rowCount = 2;
        columnCount = 2;
        layout_e = LAYOUT_2X2;
        break;
    }
    for (int row = 0; row < rowCount; ++row) {
        ui->gridLayout->setRowStretch(row, 1);
    }
    for (int column = 0; column < columnCount; ++column) {
        ui->gridLayout->setColumnStretch(column, 1);
    }
    int index = 0;
    for (int row = 0; row < rowCount; ++row) {
        for (int column = 0; column < columnCount; ++column) {
            PlaybackVideo *video = m_videoMap.value(index, nullptr);
            if (!video) {
                video = new PlaybackVideo(this);
                connect(video, SIGNAL(mouseClicked(int)), this, SLOT(onVideoClicked(int)));
                connect(video, SIGNAL(mouseDoubleClicked(int)), this, SLOT(onVideoDoubleClicked(int)));
                m_videoMap.insert(index, video);
            }
            video->setSplitChannel(channel, index);
            video->setIndexInPage(index);
            video->setSid(index);
            video->showNoResource(false);
            ui->gridLayout->addWidget(video, row, column);
            video->show();

            index++;
        }
    }

    req_layout2_s layout2_s;
    memset(&layout2_s, 0, sizeof(layout2_s));
    layout2_s.enScreen = SubControl::instance()->currentScreen();
    layout2_s.enMode = DSP_MODE_PBK;
    layout2_s.winNum = mode;
    layout2_s.enLayout = layout_e;
    for (int i = 0; i < mode; ++i) {
        CHNWIN_S &win = layout2_s.astcw[i];
        win.bshow = 1;
        win.chnid = channel;
        win.appointid = VAPI_STREAM_ID(channel, AV_FROM_PB, AV_TYPE_MAIN, i);
        win.sid = i;
        win.enFrom = AV_FROM_PB;
        win.enRatio = RATIO_VO_FULL;
#ifdef MS_FISHEYE_SOFT_DEWARP
        FisheyeKey fKey(FisheyeDewarpControl::ModePlaybackSplit, win.chnid, win.sid, SubControl::instance()->currentScreen());
        if (fKey == FisheyeDewarpControl::fisheyeChannel(FisheyeDewarpControl::ModePlaybackSplit)) {
            win.enFish = static_cast<FISH_E>(FisheyeDewarpControl::fisheyeDisplayMode(fKey));
        }
#endif
    }
    qMsDebug() << "\n----REQUEST_FLAG_SET_DISLAYOUT----"
               << "\n----channel:" << channel
               << "\n----enLayout:" << layout2_s.enLayout;
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_DISLAYOUT, (void *)&layout2_s, sizeof(layout2_s));

    if (mode == SplitLayout_0) {
        m_currentVideo = nullptr;
    } else {
        if (m_videoMap.contains(selectedSid) && selectedSid <= mode) {
            m_currentVideo = m_videoMap.value(selectedSid);
            PlaybackSplit::instance()->setSelectedSid(selectedSid);
        } else {
            m_currentVideo = m_videoMap.value(0);
            PlaybackSplit::instance()->setSelectedSid(m_currentVideo->indexInPage());
        }
    }
    s_playbackBar->updateSplitChannel();
    update();

    QMetaObject::invokeMethod(this, "dealVideoBar", Qt::QueuedConnection);
}

void PlaybackLayoutSplit::resetSplitLayout()
{
    PlaybackSplit *split = PlaybackSplit::instance();
    setSplitLayout(split->layoutMode(), split->selectedSid());
}

void PlaybackLayoutSplit::dealZoom()
{
    //zoom
    if (PlaybackZoom::instance()->isZooming()) {
        PlaybackZoom::instance()->closeZoom();
    } else {
        if (m_currentVideo) {
            hide();
            //
            int channel = m_currentVideo->channel();
            int vapiWinId = m_currentVideo->vapiWinId();
            PlaybackZoom::instance()->showZoom(channel, vapiWinId);
        }
    }
}

int PlaybackLayoutSplit::currentWinId()
{
    if (m_currentVideo) {
        return m_currentVideo->vapiWinId();
    } else {
        return 0;
    }
}

void PlaybackLayoutSplit::paintEvent(QPaintEvent *)
{
    if (m_currentVideo) {
        QRect rc = m_currentVideo->geometry();

        QPainter painter(this);
        painter.setPen(QPen(QColor(255, 255, 255), 1));
        painter.setBrush(Qt::NoBrush);
        rc.setRight(rc.right() - 1);
        rc.setBottom(rc.bottom() - 1);
        painter.drawRect(rc);
    }
}

void PlaybackLayoutSplit::showFullPlayback()
{
    if (m_isFullPlayback) {
        return;
    }

    m_isFullPlayback = true;
    Q_ASSERT(m_currentVideo);

    clearGridLayout();
    ui->gridLayout->addWidget(m_currentVideo, 0, 0);
    m_currentVideo->show();

    req_layout2_s layout2_s;
    memset(&layout2_s, 0, sizeof(layout2_s));
    layout2_s.enScreen = SubControl::instance()->currentScreen();
    layout2_s.enMode = DSP_MODE_PBK;
    layout2_s.winNum = 1;
    layout2_s.enLayout = LAYOUT_1;

    int channel = m_currentVideo->channel();
    int sid = m_currentVideo->indexInPage();

    CHNWIN_S &win = layout2_s.astcw[0];
    win.bshow = 1;
    win.chnid = channel;
    win.appointid = VAPI_STREAM_ID(channel, AV_FROM_PB, AV_TYPE_MAIN, sid);
    win.sid = 0;
    win.enFrom = AV_FROM_PB;
    win.enRatio = RATIO_VO_FULL;
#ifdef MS_FISHEYE_SOFT_DEWARP
    FisheyeKey fKey(FisheyeDewarpControl::ModePlaybackSplit, win.chnid, sid, SubControl::instance()->currentScreen());
    if (fKey == FisheyeDewarpControl::fisheyeChannel(FisheyeDewarpControl::ModePlaybackSplit)) {
        win.enFish = static_cast<FISH_E>(FisheyeDewarpControl::fisheyeDisplayMode(fKey));
    }
#endif

    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_DISLAYOUT, (void *)&layout2_s, sizeof(layout2_s));

    //全屏后winid改变
    m_currentVideo->setIndexInPage(0);

    //
    if (playbackState() == PlaybackState_Play) {
        PlaybackSplit::instance()->resumeSplitPlayback();
    }

    QMetaObject::invokeMethod(this, "dealVideoBar", Qt::QueuedConnection);
}

void PlaybackLayoutSplit::closeFullPlayback()
{
    if (!m_isFullPlayback) {
        return;
    }

    m_isFullPlayback = false;

    PlaybackSplit *split = PlaybackSplit::instance();
    setSplitLayout(split->layoutMode(), split->selectedSid());

    //
    if (playbackState() == PlaybackState_Play) {
        PlaybackSplit::instance()->resumeSplitPlayback();
    }

    QMetaObject::invokeMethod(this, "dealVideoBar", Qt::QueuedConnection);
}

void PlaybackLayoutSplit::clearGridLayout()
{
    for (auto iter = m_videoMap.constBegin(); iter != m_videoMap.constEnd(); ++iter) {
        PlaybackVideo *video = iter.value();
        ui->gridLayout->removeWidget(video);
        video->hide();
    }
    for (int row = 0; row < ui->gridLayout->rowCount(); ++row) {
        ui->gridLayout->setRowStretch(row, 0);
    }
    for (int row = 0; row < ui->gridLayout->columnCount(); ++row) {
        ui->gridLayout->setColumnStretch(row, 0);
    }
}

void PlaybackLayoutSplit::onVideoClicked(int channel)
{
    Q_UNUSED(channel)
    m_currentVideo = static_cast<PlaybackVideo *>(sender());
    update();

    PlaybackSplit::instance()->setSelectedSid(m_currentVideo->sid());
    s_playbackBar->updateSplitChannel();
    updateTimeLine();

    //zoom
    if (PlaybackZoom::instance()->isZoomMode()) {
        hide();
        //
        int vapiWinId = m_currentVideo->vapiWinId();
        PlaybackZoom::instance()->showZoom(channel, vapiWinId);
    }

    QMetaObject::invokeMethod(this, "dealVideoBar", Qt::QueuedConnection);
}

void PlaybackLayoutSplit::onVideoDoubleClicked(int channel)
{
    Q_UNUSED(channel)
    m_currentVideo = static_cast<PlaybackVideo *>(sender());

    if (m_isFullPlayback) {
        closeFullPlayback();
    } else {
        showFullPlayback();
    }

    update();

    QMetaObject::invokeMethod(this, "dealVideoBar", Qt::QueuedConnection);
}

void PlaybackLayoutSplit::dealVideoBar()
{
    if (SmartSearchControl::instance()->isSmartSearchMode()) {
        return;
    }
    if (gPlaybackData.fisheyeMode()) {
        return;
    }
    if (gPlaybackData.zoomMode()) {
        return;
    }

    if (m_currentVideo && m_currentVideo->isVisible()) {
        QRect rc = m_currentVideo->globalGeomery();
        gPlaybackVideoBar->show(-1, rc);
    } else {
        gPlaybackVideoBar->hide();
    }
}
