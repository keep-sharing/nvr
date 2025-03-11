#include "PlaybackLayout.h"
#include "ui_PlaybackLayout.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include "PlaybackData.h"
#include "PlaybackVideo.h"
#include "PlaybackVideoBar.h"
#include "PlaybackZoom.h"
#include "SmartSearchControl.h"
#include "SmartSpeedPanel.h"
#include "SubControl.h"
#include <QElapsedTimer>
#include <QPainter>

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "FisheyeDewarpControl.h"
#endif

PlaybackLayout::PlaybackLayout(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackLayout)
{
    ui->setupUi(this);

    BasePlayback::s_playbackLayout = this;
}

PlaybackLayout::~PlaybackLayout()
{
    delete ui;
}

PlaybackLayout *PlaybackLayout::instance()
{
    return s_playbackLayout;
}

void PlaybackLayout::adjustLayout(int channel)
{
    m_currentChannel = channel;
    QMetaObject::invokeMethod(this, "dealVideoBar", Qt::QueuedConnection);

    if (m_channelList == channelCheckedList()) {
        update();
        return;
    }
    m_channelList = channelCheckedList();
    setPlaybackLayout(m_channelList);

    m_isSingleLayout = false;
    //
    update();
}

void PlaybackLayout::resetLayout()
{
    m_channelList.clear();
    adjustLayout(m_currentChannel);
}

void PlaybackLayout::refreshLayout()
{
    if (m_channelList.isEmpty()) {
        return;
    }
    qDebug() << QString("PlaybackLayout::refreshLayout, %1").arg(m_layout_e);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_DISLAYOUT, (void *)&m_layout_s, sizeof(m_layout_s));
}

int PlaybackLayout::vapiWinId(int channel)
{
    int win_id = 0;
    if (m_videoMap.contains(channel)) {
        win_id = m_videoMap.value(channel)->vapiWinId();
    }
    return win_id;
}

void PlaybackLayout::dealZoom(int channel)
{
    if (PlaybackZoom::instance()->isZooming()) {
        PlaybackZoom::instance()->closeZoom();
    } else {
        PlaybackVideo *video = m_videoMap.value(channel);
        if (video) {
            hide();

            int vapiWinId = video->vapiWinId();
            PlaybackZoom::instance()->showZoom(channel, vapiWinId);
        }
    }
}

bool PlaybackLayout::isSingleLayout() const
{
    if (m_isSingleLayout || m_layout_e == LAYOUT_1) {
        return true;
    } else {
        return false;
    }
}

void PlaybackLayout::showNoResource(const QMap<int, bool> &map)
{
    QMap<int, bool>::const_iterator iter = map.constBegin();
    for (; iter != map.constEnd(); ++iter) {
        int channel = iter.key();
        bool value = iter.value();
        if (m_videoMap.contains(channel)) {
            PlaybackVideo *video = m_videoMap.value(channel);
            video->showNoResource(value);
        }
    }
}

void PlaybackLayout::clearNoResource()
{
    for (auto iter = m_videoMap.constBegin(); iter != m_videoMap.constEnd(); ++iter) {
        PlaybackVideo *video = iter.value();
        video->showNoResource(false);
    }
}

void PlaybackLayout::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_STOP_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(message);
        break;
    }
}

void PlaybackLayout::setCurrentVideo(int channel)
{
    if (!m_isSingleLayout) {
        m_currentChannel = channel;
    }
    QMetaObject::invokeMethod(this, "dealVideoBar", Qt::QueuedConnection);

    update();
}

void PlaybackLayout::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (m_videoMap.contains(m_currentChannel) && m_channelList.contains(m_currentChannel)) {
        QRect rc = m_videoMap.value(m_currentChannel)->geometry();
        painter.setPen(QPen(QColor(255, 255, 255), 1));
        painter.setBrush(Qt::NoBrush);
        rc.setRight(rc.right() - 1);
        rc.setBottom(rc.bottom() - 1);
        painter.drawRect(rc);
    }
}

void PlaybackLayout::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PlaybackLayout::setPlaybackLayout(const QList<int> &channelList)
{
#ifdef MS_FISHEYE_SOFT_DEWARP
    s_fisheyeControl->hideFisheyeDewarpControl();
#endif
    //
    QMap<int, PlaybackVideo *>::iterator iter = m_videoMap.begin();
    for (; iter != m_videoMap.end(); ++iter) {
        PlaybackVideo *video = iter.value();
        ui->gridLayout->removeWidget(video);
        video->hide();
    }

    for (int row = 0; row < ui->gridLayout->rowCount(); ++row) {
        ui->gridLayout->setRowStretch(row, 0);
    }
    for (int column = 0; column < ui->gridLayout->columnCount(); ++column) {
        ui->gridLayout->setColumnStretch(column, 0);
    }

    int count = channelList.size();
    if (count < 2) {
        m_rowCount = 1;
        m_columnCount = 1;
        m_layout_e = LAYOUT_1;
    } else if (count < 5) {
        m_rowCount = 2;
        m_columnCount = 2;
        m_layout_e = LAYOUT_2X2;
    } else if (count < 10) {
        m_rowCount = 3;
        m_columnCount = 3;
        m_layout_e = LAYOUT_3X3;
    } else if (count < 17) {
        m_rowCount = 4;
        m_columnCount = 4;
        m_layout_e = LAYOUT_4X4;
    } else {
        m_layout_e = LAYOUT_1;
    }

    for (int row = 0; row < m_rowCount; ++row) {
        ui->gridLayout->setRowStretch(row, 1);
    }
    for (int column = 0; column < m_columnCount; ++column) {
        ui->gridLayout->setColumnStretch(column, 1);
    }

    int index = 0;
    for (int row = 0; row < m_rowCount; ++row) {
        for (int column = 0; column < m_columnCount; ++column) {
            if (index < count) {
                int channel = channelList.at(index);
                PlaybackVideo *video = nullptr;
                if (!m_videoMap.contains(channel)) {
                    video = new PlaybackVideo(this);
                    connect(video, SIGNAL(mouseClicked(int)), this, SLOT(onVideoClicked(int)));
                    connect(video, SIGNAL(mouseDoubleClicked(int)), this, SLOT(onVideoDoubleClicked(int)));
                    video->setChannel(channel);
                    video->setIndexInPage(index);
                    m_videoMap.insert(channel, video);
                } else {
                    video = m_videoMap.value(channel);
                    video->setChannel(channel);
                    video->setIndexInPage(index);
                    video->showNoResource(false);
                }
                ui->gridLayout->addWidget(video, row, column);
                video->show();

                index++;
            }
        }
    }

    //
    memset(&m_layout_s, 0, sizeof(m_layout_s));
#ifdef VAPI2_0
    m_layout_s.enScreen = SubControl::instance()->currentScreen();
    m_layout_s.enMode = DSP_MODE_PBK;
    m_layout_s.winNum = count;
    m_layout_s.enLayout = m_layout_e;
    for (int i = 0; i < channelList.size(); ++i) {
        int channel = channelList.at(i);

        CHNWIN_S &win = m_layout_s.astcw[i];
        win.bshow = 1;
        win.chnid = channel;
        win.sid = i;
        win.enFrom = AV_FROM_PB;
        win.enRatio = RATIO_VO_FULL;
#ifdef MS_FISHEYE_SOFT_DEWARP
        FisheyeKey fKey(FisheyeDewarpControl::ModePlayback, win.chnid, -1, SubControl::instance()->currentScreen());
        if (fKey == FisheyeDewarpControl::fisheyeChannel(FisheyeDewarpControl::ModePlayback)) {
            win.enFish = static_cast<FISH_E>(FisheyeDewarpControl::fisheyeDisplayMode(fKey));
        }
#endif
    }
#else
    m_layout_s.enScreen = SubControl::instance()->currentScreen();
    m_layout_s.enMode = DSP_MODE_PBK;
    m_layout_s.devNum = count;
    m_layout_s.enLayout = m_layout_e;
    for (int i = 0; i < channelList.size(); ++i) {
        m_layout_s.stDevInfo[i].devID = channelList.at(i);
        m_layout_s.stDevInfo[i].voutID = i;
    }
#endif
    qMsDebug() << QString("REQUEST_FLAG_SET_DISLAYOUT, %1").arg(m_layout_e);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_DISLAYOUT, (void *)&m_layout_s, sizeof(m_layout_s));
}

void PlaybackLayout::enterSingleLayout(int channel)
{
    if (m_isSingleLayout) {
        return;
    }
    m_currentChannel = channel;

    QMetaObject::invokeMethod(this, "dealVideoBar", Qt::QueuedConnection);

    QList<int> channelList;
    channelList.append(channel);
    setPlaybackLayout(channelList);

    m_isSingleLayout = true;

    //
    if (playbackState() == PlaybackState_Play) {
        //showWait();
        waitForRestartAllPlayback();
        //closeWait();
    }
}

void PlaybackLayout::leaveSingleLayout()
{
    if (!m_isSingleLayout) {
        return;
    }

    QMetaObject::invokeMethod(this, "dealVideoBar", Qt::QueuedConnection);

    setPlaybackLayout(m_channelList);

    m_isSingleLayout = false;
    update();

    //
    if (playbackState() == PlaybackState_Play) {
        //showWait();
        waitForRestartAllPlayback();
        //closeWait();
    }
}

void PlaybackLayout::onVideoClicked(int channel)
{
    setCurrentChannel(channel);
    //
    setCurrentVideo(channel);
    setSelectedChannel(channel);
    setCurrentTimeLine(channel);
    //zoom
    if (PlaybackZoom::instance()->isZoomMode()) {
        hide();

        PlaybackVideo *video = m_videoMap.value(channel);
        if (video) {
            int vapiWinId = video->vapiWinId();
            PlaybackZoom::instance()->showZoom(channel, vapiWinId);
        }
    }

    emit videoClicked(channel);
}

void PlaybackLayout::onVideoDoubleClicked(int channel)
{
    if (!m_isSingleLayout && m_layout_e == LAYOUT_1) {
        return;
    }
    if (m_isSingleLayout) {
        leaveSingleLayout();
    } else {
        enterSingleLayout(channel);
    }
}

void PlaybackLayout::dealVideoBar()
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

    PlaybackVideo *video = m_videoMap.value(m_currentChannel);
    if (video && video->isVisible()) {
        QRect rc = video->globalGeomery();
        gPlaybackVideoBar->show(m_currentChannel, rc);
    } else {
        gPlaybackVideoBar->hide();
    }
}
