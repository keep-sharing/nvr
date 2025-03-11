#include "LiveViewSub.h"
#include "ui_LiveViewSub.h"
#include "ContentLiveView.h"
#include "CustomLayoutInfo.h"
#include "LivePage.h"
#include "LiveVideo.h"
#include "LiveViewOccupancyManager.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MyDebug.h"
#include "splashdialog.h"
#include "SubControl.h"

LiveViewSub *LiveViewSub::s_liveViewSub = nullptr;

LiveViewSub::LiveViewSub(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LiveViewSub)
{
    qDebug() << QString("LiveViewSub constructed.");
    ui->setupUi(this);

    LivePage *livePage = new LivePage(this);
    LivePage::setSubPage(livePage);
    livePage->hide();

    ui->videoContainerManager->setLogicScreen(VideoContainerManager::LogicSub);
    initializeVideo();

    hide();
}

LiveViewSub::~LiveViewSub()
{
    delete ui;
}

LiveViewSub *LiveViewSub::instance()
{
    if (!SubControl::instance()->isMultiSupported()) {
        return nullptr;
    }

    if (!s_liveViewSub) {
        s_liveViewSub = new LiveViewSub(ContentLiveView::instance());
    }
    return s_liveViewSub;
}

bool LiveViewSub::isSubEnable()
{
    return SubControl::instance()->isSubEnable();
}

void LiveViewSub::setGeometry(const QRect &rc)
{
    QWidget::setGeometry(rc);
    //
    resetTimeInfoMode();
}

void LiveViewSub::setLayoutMode(const CustomLayoutInfo &info, int page)
{
    if (!SubControl::instance()->isSubEnable()) {
        return;
    }

    ui->videoContainerManager->setLayoutMode(info, page);

    //
    if (g_splashSub && g_splashSub->isVisible()) {

    } else {
        if (m_timeWidget) {
            setTimeInfoMode(m_timeWidget->mode());
        }
    }
}

void LiveViewSub::showPage(int page, int count)
{
    QMetaObject::invokeMethod(this, "onShowPage", Qt::QueuedConnection, Q_ARG(int, page), Q_ARG(int, count));
}

void LiveViewSub::showVcaRects(VacDynamicBoxALL *info)
{
    LiveVideo *video = videoFromChannel(info->chnid);
    if (video) {
        video->drawScene()->showVcaRects(info);
    }
}

void LiveViewSub::showRegionRects(RegionalRectInfo *info)
{
    LiveVideo *video = videoFromChannel(info->chind);
    if (video) {
        video->drawScene()->showRegionRects(info);
    }
}

void LiveViewSub::showRegionAlarm(const RegionalAlarmInfo &info)
{
    LiveVideo *video = videoFromChannel(info.chnid);
    if (video) {
        video->drawScene()->showRegionAlarm(info);
    }
}

void LiveViewSub::setPopupVideo(int screen, int layout, const QList<int> &channels)
{
    if (LivePage::subPage()->isVisible()) {
        LivePage::subPage()->hide();
    }
    //
    switch (layout) {
    case LAYOUTMODE_1: {
        CustomLayoutInfo info("LAYOUTMODE_1", screen, 1, 1);
        info.setType(CustomLayoutKey::DefaultType);
        for (int r = 0; r < 1; ++r) {
            for (int c = 0; c < 1; ++c) {
                info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
            }
        }
        for (int i = 0; i < channels.size(); ++i) {
            info.insertChannel(i, channels.at(i));
        }
        ui->videoContainerManager->setLayoutMode(info, 0);
        break;
    }
    case LAYOUTMODE_4: {
        CustomLayoutInfo info("LAYOUTMODE_4", screen, 2, 2);
        info.setType(CustomLayoutKey::DefaultType);
        for (int r = 0; r < 2; ++r) {
            for (int c = 0; c < 2; ++c) {
                info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
            }
        }
        for (int i = 0; i < channels.size(); ++i) {
            info.insertChannel(i, channels.at(i));
        }
        ui->videoContainerManager->setLayoutMode(info, 0);
        break;
    }
    default:
        qMsWarning() << QString("invalid layout: %1").arg(layout);
        return;
    }
}

void LiveViewSub::setTimeInfoMode(int mode)
{
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
        if (SubControl::instance()->isSubEnable() && isVisible()) {
            m_timeWidget->show();
            m_timeWidget->raise();
        } else {
            m_timeWidget->hide();
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

void LiveViewSub::resetTimeInfoMode()
{
    int mode = qMsNvr->displayInfo().time_info;
    setTimeInfoMode(mode);
}

void LiveViewSub::updateDisplayInfo()
{
    for (auto iter = m_mapChannelVideo.constBegin(); iter != m_mapChannelVideo.constEnd(); ++iter) {
        LiveVideo *video = iter.value();
        video->updateDisplayInfo();
    }
}

void LiveViewSub::updateStreamInfo(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->updateStreamInfo();
    }
}

void LiveViewSub::updateMotion(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->updateMotion();
    }
}

void LiveViewSub::updateRecord(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->updateRecord();
    }
}

void LiveViewSub::updateVideoLoss(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->updateVideoLoss();
    }
}

void LiveViewSub::showNoResource(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->showNoResource();
    }
}

void LiveViewSub::updateAnprEvent(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->updateAnprEvent();
    }
}

void LiveViewSub::updateSmartEvent(int channel)
{
    LiveVideo *video = videoFromChannel(channel);
    if (video) {
        video->updateSmartEvent();
    }
}

void LiveViewSub::clearAllPosData()
{
    for (auto iter = m_mapChannelVideo.constBegin(); iter != m_mapChannelVideo.constEnd(); ++iter) {
        LiveVideo *video = iter.value();
        video->clearPos();
    }
}

void LiveViewSub::showEvent(QShowEvent *event)
{
    if (m_timeWidget) {
        setTimeInfoMode(m_timeWidget->mode());
    }
    QWidget::showEvent(event);
}

void LiveViewSub::hideEvent(QHideEvent *event)
{
    if (m_timeWidget) {
        m_timeWidget->hide();
    }
    QWidget::hideEvent(event);
}

void LiveViewSub::onShowPage(int page, int count)
{
    QRect rc = rect();
    LivePage::subPage()->adjustPos(rc);

    LivePage::subPage()->setPage(page, count);
    switch (LivePage::mode()) {
    case LivePage::ModeAlwaysShow:
        LivePage::subPage()->show();
        LivePage::subPage()->raise();
        break;
    case LivePage::ModeAlwaysHide:
        break;
    case LivePage::ModeAuto:
        LivePage::subPage()->showPage(2000);
        break;
    default:
        break;
    }
}

void LiveViewSub::initializeVideo()
{
    const int channelCount = qMsNvr->maxChannel();
    for (int i = 0; i < channelCount; ++i) {
        makeNewVideo(i);
    }
}

LiveVideo *LiveViewSub::makeNewVideo(int channel)
{
    LiveVideo *video = new LiveVideo(this);
    video->setChannel(channel);
    video->setVideoDragEnable(false);
    video->hide();
    m_mapChannelVideo.insert(channel, video);
    return video;
}

LiveVideo *LiveViewSub::videoFromChannel(int channel)
{
    LiveVideo *video = m_mapChannelVideo.value(channel, nullptr);
    return video;
}

LiveVideo *LiveViewSub::liveVideo(int channel)
{
    return m_mapChannelVideo.value(channel);
}
