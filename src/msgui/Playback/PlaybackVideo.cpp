#include "PlaybackVideo.h"
#include "ui_PlaybackVideo.h"
#include "DrawView.h"
#include "DynamicDisplayData.h"
#include "GraphicsScene.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackBar.h"
#include "PlaybackData.h"
#include "SubControl.h"
#include <QMouseEvent>

PlaybackVideo::PlaybackVideo(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlaybackVideo)
{
    ui->setupUi(this);
    showNoResource(false);

    //
    connect(qMsNvr, SIGNAL(noResourceChanged(int, int)), this, SLOT(onNoResourceChanged(int, int)));
    //
    connect(PlaybackBar::s_playbackBar, SIGNAL(posClicked(bool)), this, SLOT(onPosClicked(bool)));
    connect(PlaybackBar::s_playbackBar, SIGNAL(pauseClicked(bool)), this, SLOT(onPauseClicked(bool)));
    //
    m_viewPos = new DrawView(this);
    m_viewPos->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_scenePos = new GraphicsScene(this);
    m_viewPos->setScene(m_scenePos);
    connect(&gDynamicDisplayData, SIGNAL(posDataReceived(PosData)), this, SLOT(onPosDataReceived(PosData)));
}

PlaybackVideo::~PlaybackVideo()
{
    delete ui;
}

void PlaybackVideo::setChannel(int channel)
{
    QString text = qMsNvr->channelName(channel);
    setChannelText(text);
    m_channel = channel;
}

int PlaybackVideo::channel() const
{
    return m_channel;
}

void PlaybackVideo::setSplitChannel(int channel, int sid)
{
    QString text = qMsNvr->channelName(channel);
    setChannelText(QString("%1-%2").arg(text).arg(sid + 1));
    m_channel = channel;
}

void PlaybackVideo::showNoResource(bool show)
{
    //qWarning() << this << "PlaybackVideo::showNoResource," << VapiWinIdString(vapiWinId()) << show;
    if (show) {
        ui->label_noResource->setText(GET_TEXT("LIVEVIEW/20031", "No Resource"));
    }
    ui->label_noResource->setGeometry(rect());
    ui->label_noResource->setVisible(show);
}

void PlaybackVideo::setIndexInPage(int index)
{
    m_indexInPage = index;
}

int PlaybackVideo::indexInPage() const
{
    return m_indexInPage;
}

int PlaybackVideo::vapiWinId()
{
    return VAPI_WIN_ID(SubControl::instance()->currentScreen(), indexInPage());
}

void PlaybackVideo::setSid(int sid)
{
    m_sid = sid;
}

int PlaybackVideo::sid()
{
    return m_sid;
}

QRect PlaybackVideo::globalGeomery()
{
    return QRect(mapToGlobal(QPoint(0, 0)), size());
}

void PlaybackVideo::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        qDebug() << this << "PlaybackVideo::mousePressEvent," << VapiWinIdString(vapiWinId());
        emit mouseClicked(m_channel);
    }
    QWidget::mousePressEvent(event);
}

void PlaybackVideo::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit mouseDoubleClicked(m_channel);
    QWidget::mouseDoubleClickEvent(event);
}

void PlaybackVideo::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    //
    ui->label_noResource->setGeometry(rect());
    //
    m_viewPos->setGeometry(rect());
    for (auto iter = m_itemPosMap.constBegin(); iter != m_itemPosMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->setRect(m_scenePos->sceneRect());
    }
}

void PlaybackVideo::setChannelText(const QString &text)
{
    ui->label_channel->setText(text);
    ui->label_channel->adjustSize();
    ui->label_channel->move(20, 20);
}

void PlaybackVideo::onNoResourceChanged(int winid, int bNoResource)
{
    if (winid == vapiWinId()) {
        showNoResource(bNoResource);
    }
}

void PlaybackVideo::onPosClicked(bool show)
{
    for (auto iter = m_itemPosMap.constBegin(); iter != m_itemPosMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->clear();
        item->setVisible(show);
    }
}

void PlaybackVideo::onPauseClicked(bool pause)
{
    for (auto iter = m_itemPosMap.constBegin(); iter != m_itemPosMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->setPaused(pause);
    }
}

void PlaybackVideo::onPosDataReceived(PosData data)
{
    if (data.channel() != m_channel) {
        return;
    }
    if (data.streamFrom() != SST_LOCAL_PB) {
        return;
    }
    if (m_sid > -1 && data.sid() != m_sid) {
        return;
    }
    //
    if (gPlaybackData.playbackSpeed() != PLAY_SPEED_1X) {
        return;
    }
    //
    auto *item = m_itemPosMap.value(data.posId());
    if (!item) {
        item = new GraphicsItemPosText();
        m_scenePos->addItem(item);
        item->setVisible(PlaybackBar::s_playbackBar->isShowPosData());
        item->setRect(m_scenePos->sceneRect());
        item->setUpdateBehavior(GraphicsItemPosText::NoPartialUpdate);
        item->setZValue(data.posId());
        m_itemPosMap.insert(data.posId(), item);
    }
    item->setPosData(data);
}
