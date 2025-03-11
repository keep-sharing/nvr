#include "PlaybackZoom.h"
#include "ui_PlaybackZoom.h"
#include "BasePlayback.h"
#include "centralmessage.h"
#include "MsGlobal.h"
#include "MyDebug.h"
#include "PlaybackWindow.h"
#include "PlaybackLayout.h"
#include "SubControl.h"
#include <QMouseEvent>
#include <QPainter>

PlaybackZoom *PlaybackZoom::s_playbackZoom = nullptr;

PlaybackZoom::PlaybackZoom(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlaybackZoom)
{
    ui->setupUi(this);

    s_playbackZoom = this;

    //
    m_control = new ZoomControl(this);
    connect(m_control, SIGNAL(zoomChanged(int, QRect)), this, SLOT(onZoomChanged(int, QRect)));

    m_timerAutoHide = new QTimer(this);
    m_timerAutoHide->setSingleShot(true);
    m_timerAutoHide->setInterval(2000);
    connect(m_timerAutoHide, SIGNAL(timeout()), this, SLOT(onTimerAutoHide()));

    m_labelScale = new QLabel(this);
    m_labelScale->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_labelScale->resize(100, 60);
    m_labelScale->setVisible(false);
}

PlaybackZoom::~PlaybackZoom()
{
    s_playbackZoom = nullptr;
    delete ui;
}

PlaybackZoom *PlaybackZoom::instance()
{
    return s_playbackZoom;
}

void PlaybackZoom::showZoom(int channel, int vapiWinId)
{
    m_channel = channel;
    m_winId = vapiWinId;

    //
    setZoomMode(false);
    //
    const QRect &rc = BasePlayback::s_playbackLayout->geometry();
    setGeometry(rc);
    show();

    m_labelScale->setStyleSheet("color: #FFFFFF;\
                                background-color: rgba(0, 0, 0, 50%);\
                                font: 750 26pt;\
                                border-radius: 30px;");
    m_labelScale->move((width() - m_labelScale->width()) / 2, (height() - m_labelScale->height()) / 2);

    m_control->setGeometry(0, 0, rc.width(), rc.height());
    m_control->show();

    //
    REQ_ZOOMIN2_S zoom_info;
    memset(&zoom_info, 0, sizeof(zoom_info));
    zoom_info.enState = STATE_ENTER;
    zoom_info.winid = m_winId;
    zoom_info.stRect.enMode = ZONE_MODE_USER;
    zoom_info.stRect.x = 0;
    zoom_info.stRect.y = 0;
    zoom_info.stRect.w = m_control->nvrWidth();
    zoom_info.stRect.h = m_control->nvrHeight();
    qDebug() << QString("REQUEST_FLAG_SET_ZOOMINMODE, STATE_ENTER, winid: %1").arg(VapiWinIdString(m_winId));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_ZOOMINMODE, (void *)&zoom_info, sizeof(zoom_info));

    emit zoomStateChanged(1);
}

void PlaybackZoom::setZoomMode(bool zoom)
{
    if (zoom == m_isZoomMode) {
        return;
    }
    m_isZoomMode = zoom;
    if (m_isZoomMode) {
        qDebug() << "qApp->setOverrideCursor()";
        qApp->setOverrideCursor(QCursor(QPixmap(":/videobar/videobar/zoom.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    } else {
        qDebug() << "qApp->restoreOverrideCursor()";
        qApp->restoreOverrideCursor();
    }
}

bool PlaybackZoom::isZooming() const
{
    return m_channel >= 0;
}

void PlaybackZoom::closeZoom()
{
    //
    m_control->clearZoom();
    hide();
    //
    if (m_channel >= 0) {
        m_channel = -1;

        if (BasePlayback::playbackType() == SplitPlayback) {
            PlaybackLayoutSplit *splitLayout = PlaybackLayoutSplit::instance();
            splitLayout->show();
            splitLayout->resetSplitLayout();
        } else {
            PlaybackLayout::instance()->show();
            PlaybackLayout::instance()->resetLayout();
        }
    }

    emit zoomStateChanged(0);
}

bool PlaybackZoom::isZoomMode() const
{
    return m_isZoomMode;
}

void PlaybackZoom::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
    }
    QWidget::mousePressEvent(event);
}

void PlaybackZoom::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}

void PlaybackZoom::onZoomChanged(int zoom, QRect rc)
{
    REQ_ZOOMIN2_S zoom_info;
    memset(&zoom_info, 0, sizeof(zoom_info));
    zoom_info.enState = STATE_ZOOMIN;
    zoom_info.winid = m_winId;
    zoom_info.stRect.enMode = ZONE_MODE_USER;
    zoom_info.stRect.x = rc.left();
    zoom_info.stRect.y = rc.top();
    zoom_info.stRect.w = rc.width();
    zoom_info.stRect.h = rc.height();

    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_ZOOMINMODE, (void *)&zoom_info, sizeof(zoom_info));

    m_labelScale->setText(QString("%1%").arg(zoom));
    m_labelScale->show();
    m_timerAutoHide->start();
}

void PlaybackZoom::onTimerAutoHide()
{
    m_labelScale->setVisible(false);
}
