#include "commonvideo.h"
#include "ui_commonvideo.h"
#include "CameraData.h"
#include "CommonVideoData.h"
#include "DrawView.h"
#include "DynamicDisplayData.h"
#include "GraphicsItemPosText.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "SubControl.h"
#include "centralmessage.h"
#include "drawmask.h"
#include "drawmotion.h"
#include "msuser.h"
#include <QDesktopWidget>
#include <QPainter>
#include <qmath.h>

#define PIP_WINDOW_INDEX (MAX_WINS_NUM - 1)

req_pipmode2_s CommonVideo::s_pipmode;
QRect CommonVideo::s_nvrVideoGeometry;
QRect CommonVideo::s_qtVideoGeometry;
int CommonVideo::s_channel = -1;
bool CommonVideo::s_isPlaying = false;
bool CommonVideo::s_banOnBack = false;

CommonVideo *CommonVideo::s_commonVideo = nullptr;

CommonVideo::CommonVideo(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::CommonVideo)
{
    ui->setupUi(this);

    memset(&s_pipmode, 0, sizeof(s_pipmode));

    m_labelMessage = new NormalLabel(this);
    m_labelMessage->setStyleSheet("color: #FFFFFF;font-size: 30pt;");
    m_labelMessage->setAlignment(Qt::AlignCenter);
    m_labelMessage->setWordWrap(true);
    m_labelMessage->hide();

    ui->label_channel_2->setText(QString("%1:").arg(GET_TEXT("CHANNELMANAGE/30008", "Channel")));
    ui->label_name_2->setText(QString("%1:").arg(GET_TEXT("CAMERASTATUS/62002", "Name")));

    if (!m_commonView) {
        m_commonView = new DrawView(this);
        m_commonView->setGeometry(ui->frameVideo->geometry());
    }
    if (!m_commonScene) {
        m_commonScene = new GraphicsScene(this);
        m_commonView->setScene(m_commonScene);
    }

    connect(&gDynamicDisplayData, SIGNAL(posDataReceived(PosData)), this, SLOT(onPosDataReceived(PosData)));

    //
    connect(qMsNvr, SIGNAL(noResourceChanged(int, int)), this, SLOT(onNoResourceChanged(int, int)));
    connect(&gCameraData, SIGNAL(cameraFacePrivacyState(int, int)), this, SLOT(onCameraFacePrivacyState(int, int)));
}

CommonVideo::~CommonVideo()
{
    s_commonVideo = nullptr;
    delete ui;
}

CommonVideo *CommonVideo::instance()
{
    return s_commonVideo;
}

QRect CommonVideo::qtVideoGeometry()
{
    return s_qtVideoGeometry;
}

void CommonVideo::hideVideo()
{
    s_qtVideoGeometry = QRect();
}

void CommonVideo::showCurrentChannel(int channel)
{
    ui->label_channel_2->setText(QString("%1:").arg(GET_TEXT("CHANNELMANAGE/30008", "Channel")));
    ui->label_name_2->setText(QString("%1:").arg(GET_TEXT("CAMERASTATUS/62002", "Name")));
    if (channel < 0) {
        ui->label_channel->setText(QString("-"));
        setChannelName("-");
    } else {
        ui->label_channel->setText(QString::number(channel + 1));
        setChannelName(qMsNvr->channelName(channel));
    }
}

void CommonVideo::setCurrentChannel(int channel)
{
    s_channel = channel;
    showCurrentChannel(channel);

    for (auto iter = m_itemPosMap.constBegin(); iter != m_itemPosMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->clear();
    }
}

void CommonVideo::showPixmap(const QPixmap &pixmap)
{
    if (!m_pixmap) {
        m_pixmap = new QLabel(this);
        m_pixmap->setScaledContents(true);
    }
    m_pixmap->setGeometry(ui->frameVideo->geometry());
    m_pixmap->setPixmap(pixmap);
    m_pixmap->show();
}

void CommonVideo::showPixmap()
{
    if (m_pixmap) {
        m_pixmap->show();
    }
}

void CommonVideo::hidePixmap()
{
    if (m_pixmap) {
        m_pixmap->hide();
    }
}

void CommonVideo::playVideo(int channel)
{
    if (channel < 0) {
        qWarning() << QString("CommonVideo::playVideo, channel: %1").arg(channel);
        return;
    }

    //
    if (!gMsUser.hasLiveViewChannelPermission(channel)) {
        stopAllVideo();
        m_messageMap.insert(MSG_PERMISSION, 0);
        showMessage();
        showCurrentChannel(channel);
        return;
    } else {
        m_messageMap.remove(MSG_PERMISSION);
    }

    //
    if (gCameraData.isCameraFacePrivacyMode(channel) && !qMsNvr->isNT(channel) && !qMsNvr->is3536a()) {
        m_messageMap.insert(MSG_FACEPRIVACY, 0);
    } else {
        m_messageMap.remove(MSG_FACEPRIVACY);
    }
    showMessage();

    //
    setCurrentChannel(channel);

    //
    s_pipmode.winid = VAPI_WIN_ID(SubControl::instance()->currentScreen(), PIP_WINDOW_INDEX);
    s_pipmode.chnid = channel;
    s_pipmode.streamid = VAPI_STREAM_ID(channel, AV_FROM_LIVE, AV_TYPE_MAIN, 0);
    s_pipmode.stZone.enMode = ZONE_MODE_USER;
    s_pipmode.stZone.x = s_nvrVideoGeometry.left();
    s_pipmode.stZone.y = s_nvrVideoGeometry.top();
    s_pipmode.stZone.w = s_nvrVideoGeometry.width();
    s_pipmode.stZone.h = s_nvrVideoGeometry.height();

    qDebug() << QString("CommonVideo::playVideo, REQUEST_FLAG_SET_PIPMODE, channel: %1").arg(channel);
    sendMessageOnly(REQUEST_FLAG_SET_PIPMODE, &s_pipmode, sizeof(req_pipmode2_s));

    s_isPlaying = true;
    s_banOnBack = false;
}

void CommonVideo::stopVideo()
{
    stopAllVideo();
    showCurrentChannel(-1);
}

void CommonVideo::playbackVideo(int channel)
{
    if (channel < 0) {
        qWarning() << QString("CommonVideo::playbackVideo, channel: %1").arg(channel);
        return;
    }

    hidePixmap();

    setCurrentChannel(channel);

    s_pipmode.winid = VAPI_WIN_ID(SubControl::instance()->currentScreen(), PIP_WINDOW_INDEX);
    s_pipmode.chnid = channel;
    s_pipmode.streamid = VAPI_STREAM_ID(channel, AV_FROM_PB, AV_TYPE_NORMAL, 0);
    s_pipmode.stZone.enMode = ZONE_MODE_USER;
    s_pipmode.stZone.x = s_nvrVideoGeometry.left();
    s_pipmode.stZone.y = s_nvrVideoGeometry.top();
    s_pipmode.stZone.w = s_nvrVideoGeometry.width();
    s_pipmode.stZone.h = s_nvrVideoGeometry.height();

    qDebug() << QString("CommonVideo::playVideo, REQUEST_FLAG_SET_BACKUPMODE, channel: %1").arg(channel);
    sendMessageOnly(REQUEST_FLAG_SET_BACKUPMODE, (void *)&s_pipmode, sizeof(s_pipmode));

    s_isPlaying = true;
}

int CommonVideo::stopAllVideo()
{
    int channel = s_channel;
    if (s_channel < 0) {
        return channel;
    }
    if (!s_isPlaying) {
        return channel;
    }

    struct req_layout2_s reqLayout;
    memset(&reqLayout, 0, sizeof(reqLayout));
    reqLayout.enScreen = SubControl::instance()->currentScreen();
    reqLayout.enMode = DSP_MODE_LIVE;
    qDebug() << "CommonVideo::stopVideo";
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_DISLAYOUT, (void *)&reqLayout, sizeof(reqLayout));

    s_channel = -1;
    s_isPlaying = false;
    return channel;
}

bool CommonVideo::isPlaying()
{
    return s_isPlaying;
}

void CommonVideo::setDrawWidget(QWidget *widget)
{
    hideDrawWidget();
    m_drawWidget = widget;
    if (m_drawWidget) {
        m_drawWidget->setParent(this);
        m_drawWidget->setGeometry(ui->frameVideo->geometry());
    }
}

//TODO 这个用setDrawView替代
void CommonVideo::showDrawWidget(QWidget *widget)
{
    setDrawWidget(widget);
    if (m_drawWidget) {
        m_drawWidget->show();
    }
}

void CommonVideo::showDrawWidget()
{
    if (m_drawWidget) {
        m_drawWidget->show();
    }
}

void CommonVideo::hideDrawWidget()
{
    if (m_drawWidget) {
        m_drawWidget->hide();
    }
}

void CommonVideo::setDrawWidgetVisible(bool visible)
{
    if (m_drawWidget) {
        m_drawWidget->setVisible(visible);
    }
}

void CommonVideo::setDrawWidgetEnable(bool enable)
{
    if (m_drawWidget) {
        m_drawWidget->setEnabled(enable);
    }
}

void CommonVideo::showDrawView()
{
    if (m_drawView) {
        m_drawView->show();
    }
}

void CommonVideo::hideDrawView()
{
    if (m_drawView) {
        m_drawView->hide();
    }
}

void CommonVideo::setDrawScene(QGraphicsScene *scene)
{
    if (m_drawWidget) {
        m_drawWidget->hide();
    }

    if (!m_drawView) {
        m_drawView = new DrawView(this);
        m_drawView->setGeometry(ui->frameVideo->geometry());
    }
    m_drawView->setScene(scene);
}

void CommonVideo::showDrawScene(QGraphicsScene *scene)
{
    setDrawScene(scene);
    m_drawView->show();
}

void CommonVideo::removeDrawScene()
{
    if (m_drawView) {
        m_drawView->setScene(nullptr);
        m_drawView->hide();
    }
}

void CommonVideo::setDrawViewVisible(bool visible)
{
    if (m_drawView) {
        m_drawView->setVisible(visible);
    }
}

void CommonVideo::setDrawViewEnable(bool enable)
{
    if (m_drawView) {
        m_drawView->setEnabled(enable);
    }
}

void CommonVideo::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    adjustVideoRegion();
}

void CommonVideo::showEvent(QShowEvent *event)
{
    s_commonVideo = this;
    QWidget::showEvent(event);

    adjustVideoRegion();
}

void CommonVideo::hideEvent(QHideEvent *event)
{
    hideVideo();
    s_commonVideo = nullptr;
    QWidget::hideEvent(event);
}

void CommonVideo::mousePressEvent(QMouseEvent *event)
{
    if (s_banOnBack && event->button() == Qt::RightButton) {
        return;
    }
    QWidget::mousePressEvent(event);
}

void CommonVideo::adjustVideoRegion()
{
    //3536需要2对齐，3798需要4对齐
    //x,y进一法取对齐，w,h去掉小数取对齐
    const int alignValue = 4;
    //取一块16:9的区域显示视频，alignValue对齐
    int w = width() / alignValue * alignValue;
    int h = w * 9 / 16 / alignValue * alignValue;
    setMinimumHeight(h + 40);

    //转换为全局坐标，alignValue对齐
    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    int x = qCeil((qreal)globalPos.x() / alignValue) * alignValue;
    int y = qCeil((qreal)globalPos.y() / alignValue) * alignValue;
    s_qtVideoGeometry = QRect(x, y, w, h);

    int nvrWidth = 0;
    int nvrHeight = 0;
    vapi_get_screen_res(SubControl::instance()->currentScreen(), &nvrWidth, &nvrHeight);

    QRect screenRect = qApp->desktop()->geometry();
    if (screenRect.width() == nvrWidth && screenRect.height() == nvrHeight) {
        s_nvrVideoGeometry = s_qtVideoGeometry;
    } else {
        qreal xRatio = (qreal)screenRect.width() / nvrWidth;
        qreal yRatio = (qreal)screenRect.height() / nvrHeight;
        s_nvrVideoGeometry.setLeft(s_qtVideoGeometry.x() / xRatio);
        s_nvrVideoGeometry.setTop(s_qtVideoGeometry.y() / yRatio);
        s_nvrVideoGeometry.setWidth(s_qtVideoGeometry.width() / xRatio);
        s_nvrVideoGeometry.setHeight(s_qtVideoGeometry.height() / yRatio);
        //宽高alignValue对齐
        s_nvrVideoGeometry.setX(qCeil((qreal)s_nvrVideoGeometry.x() / alignValue) * alignValue);
        s_nvrVideoGeometry.setY(qCeil((qreal)s_nvrVideoGeometry.y() / alignValue) * alignValue);
        s_nvrVideoGeometry.setWidth(s_nvrVideoGeometry.width() / alignValue * alignValue);
        s_nvrVideoGeometry.setHeight(s_nvrVideoGeometry.height() / alignValue * alignValue);
    }

    //
    QPoint localPos = mapFromGlobal(s_qtVideoGeometry.topLeft());
    ui->frameVideo->setGeometry(localPos.x(), localPos.y(), s_qtVideoGeometry.width(), s_qtVideoGeometry.height());
    ui->widgetChannel->setGeometry(localPos.x(), s_qtVideoGeometry.height(), s_qtVideoGeometry.width(), 40);

    //
    m_labelMessage->setGeometry(ui->frameVideo->geometry());

    //
    if (m_drawWidget) {
        m_drawWidget->setGeometry(ui->frameVideo->geometry());
    }
    if (m_drawView) {
        m_drawView->setGeometry(ui->frameVideo->geometry());
    }

    //
    m_commonView->setGeometry(ui->frameVideo->geometry());
    for (auto iter = m_itemPosMap.constBegin(); iter != m_itemPosMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->setRect(m_commonScene->sceneRect());
    }

    qMsDebug() << "qt:" << s_qtVideoGeometry << "nvr:" << s_nvrVideoGeometry;
}

QRect CommonVideo::videoFrameGeometry() const
{
    return ui->frameVideo->geometry();
}

void CommonVideo::onPosDataReceived(PosData data)
{
    if (!s_isPlaying) {
        return;
    }
    if (data.channel() != s_channel) {
        return;
    }
    if (data.streamFormat() != STREAM_MAIN) {
        return;
    }
    switch (data.streamFrom()) {
    case SST_IPC_STREAM:
        if (s_pipmode.streamid != VAPI_STREAM_ID(s_channel, AV_FROM_LIVE, AV_TYPE_MAIN, 0)) {
            return;
        }
        break;
    case SST_LOCAL_PB:
        if (s_pipmode.streamid != VAPI_STREAM_ID(s_channel, AV_FROM_PB, AV_TYPE_NORMAL, 0)) {
            return;
        }
        break;
    default:
        return;
    }

    auto *item = m_itemPosMap.value(data.posId());
    if (!item) {
        item = new GraphicsItemPosText();
        m_commonScene->addItem(item);
        item->setRect(m_commonScene->sceneRect());
        item->setUpdateBehavior(GraphicsItemPosText::NoPartialUpdate);

        m_itemPosMap.insert(data.posId(), item);
        item->setZValue(data.posId());
    }
    item->setPosData(data);
}

void CommonVideo::setBanOnBack(bool value)
{
    s_banOnBack = value;
}

void CommonVideo::setPosVisible(bool visible)
{
    for (auto iter = m_itemPosMap.constBegin(); iter != m_itemPosMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->setVisible(visible);
    }
}

void CommonVideo::setPosPaused(bool pause)
{
    for (auto iter = m_itemPosMap.constBegin(); iter != m_itemPosMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->setPaused(pause);
    }
}

void CommonVideo::clearPos()
{
    for (auto iter = m_itemPosMap.constBegin(); iter != m_itemPosMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->clear();
    }
}

void CommonVideo::addGraphicsItem(QGraphicsItem *item)
{
    if (m_drawWidget) {
        m_drawWidget->hide();
    }
    qDebug()<<"gsjt";
    //
    m_commonScene->addItem(item);
}

void CommonVideo::removeGraphicsItem(QGraphicsItem *item)
{
    m_commonScene->removeItem(item);
}

void CommonVideo::showMessage()
{
    m_labelMessage->clear();
    if (!m_messageMap.isEmpty()) {
        auto iter = m_messageMap.constBegin();
        switch (iter.key()) {
        case MSG_PERMISSION:
            m_labelMessage->setText(GET_TEXT("LIVEVIEW/20109", "No Permission"));
            break;
        case MSG_RESOURCE:
            m_labelMessage->setText(GET_TEXT("LIVEVIEW/20031", "No Resource"));
            break;
        case MSG_FACEPRIVACY:
            m_labelMessage->setText(GET_TEXT("LIVEVIEW/20113", "Cannot view in Face Privacy Mode"));
            break;
        }
        m_labelMessage->show();
    } else {
        m_labelMessage->hide();
    }
}

void CommonVideo::hideMessage()
{
    m_labelMessage->hide();
}

void CommonVideo::paintEvent(QPaintEvent *)
{
    //    QPainter painter(this);
    //    painter.setBrush(Qt::red);
    //    painter.drawRect(rect());
}

void CommonVideo::setChannelName(const QString &name)
{
    QFontMetrics fm(ui->label_name->font());
    QString text = fm.elidedText(name, Qt::ElideRight, 100);
    ui->label_name->setText(text);
}

void CommonVideo::onNoResourceChanged(int winid, int bNoResource)
{
    if (winid == s_pipmode.winid) {
        if (bNoResource) {
            m_messageMap.insert(MSG_RESOURCE, 0);
        } else {
            m_messageMap.remove(MSG_RESOURCE);
        }
        showMessage();
    }
}

void CommonVideo::onCameraFacePrivacyState(int channel, int state)
{
    if (channel == s_channel) {
        if (state == 1 && !qMsNvr->isNT(channel) && !qMsNvr->is3536a()) {
            m_messageMap.insert(MSG_FACEPRIVACY, 0);
        } else {
            m_messageMap.remove(MSG_FACEPRIVACY);
        }
        showMessage();
    }
}
