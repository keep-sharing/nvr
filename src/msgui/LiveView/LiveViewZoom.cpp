#include "LiveViewZoom.h"
#include "ui_LiveViewZoom.h"
#include "LiveViewTarget.h"
#include "MsGlobal.h"
#include "SubControl.h"
#include "centralmessage.h"
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QtDebug>

LiveViewZoom *LiveViewZoom::s_liveViewZoom = nullptr;

LiveViewZoom::LiveViewZoom(int winId, QWidget *parent)
    : BaseDialog(parent)
    , ui(new Ui::LiveViewZoom)
    , m_winId(winId)
{
    ui->setupUi(this);
    s_liveViewZoom = this;

    //
    m_labelScale = new QLabel(this);
    m_labelScale->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_labelScale->resize(100, 60);
    m_labelScale->setVisible(false);

    //
    m_control = new ZoomControl(this);
    connect(m_control, SIGNAL(zoomChanged(int, QRect)), this, SLOT(onZoomChanged(int, QRect)));

    m_timerAutoHide = new QTimer(this);
    m_timerAutoHide->setSingleShot(true);
    m_timerAutoHide->setInterval(2000);
    connect(m_timerAutoHide, SIGNAL(timeout()), this, SLOT(onTimerAutoHide()));
}

LiveViewZoom::~LiveViewZoom()
{
    s_liveViewZoom = nullptr;
    delete ui;
}

LiveViewZoom *LiveViewZoom::instance()
{
    return s_liveViewZoom;
}

void LiveViewZoom::showZoom(const LiveViewZoom::ZoomMode &mode, const QRect &rc)
{
    m_mode = mode;
    //
    setGeometry(rc);
    show();

    m_labelScale->setStyleSheet("color: #FFFFFF;\
                                background-color: rgba(0, 0, 0, 50%);\
                                font: 750 26pt;\
                                border-radius: 30px;");
    m_labelScale->move((width() - m_labelScale->width()) / 2, (height() - m_labelScale->height()) / 2);

    m_control->setGeometry(0, 0, rc.width(), rc.height());
    m_control->show();

    //初始化时左上角是全局的屏幕坐标，后面缩放时是基于原图像的缩放，按照(0,0)缩放
    QPoint globalPos = mapToGlobal(QPoint(0, 0));

    REQ_ZOOMIN2_S zoom_info;
    memset(&zoom_info, 0, sizeof(zoom_info));
    zoom_info.enState = STATE_ENTER;
    zoom_info.winid = m_winId;
    zoom_info.stRect.enMode = ZONE_MODE_USER;
    zoom_info.stRect.x = globalPos.x();
    zoom_info.stRect.y = globalPos.y();
    zoom_info.stRect.w = m_control->nvrWidth();
    zoom_info.stRect.h = m_control->nvrHeight();
    qDebug() << QString("LiveViewZoom::showZoom, %1, QRect(%2,%3 %4x%5)")
                    .arg(VapiWinIdString(m_winId))
                    .arg(zoom_info.stRect.x)
                    .arg(zoom_info.stRect.y)
                    .arg(zoom_info.stRect.w)
                    .arg(zoom_info.stRect.h);
    sendMessageOnly(REQUEST_FLAG_SET_ZOOMINMODE, (void *)&zoom_info, sizeof(zoom_info));
}

void LiveViewZoom::closeZoom()
{
    m_control->clearZoom();
    close();
}

void LiveViewZoom::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        close();
    }

    BaseDialog::mousePressEvent(event);
}

void LiveViewZoom::closeEvent(QCloseEvent *event)
{
    BaseDialog::closeEvent(event);
}

bool LiveViewZoom::isMoveToCenter()
{
    return false;
}

bool LiveViewZoom::isAddToVisibleList()
{
    return true;
}

void LiveViewZoom::escapePressed()
{
    close();
}

void LiveViewZoom::onZoomChanged(int zoom, QRect rc)
{
    //
    REQ_ZOOMIN2_S zoom_info;
    memset(&zoom_info, 0, sizeof(zoom_info));
    zoom_info.enState = STATE_ZOOMIN;
    zoom_info.winid = m_winId;
    zoom_info.stRect.enMode = ZONE_MODE_USER;
    zoom_info.stRect.x = rc.left();
    zoom_info.stRect.y = rc.top();
    zoom_info.stRect.w = rc.width();
    zoom_info.stRect.h = rc.height();

    //qDebug() << QString("LiveViewZoom::onZoomChanged, scale: %1%").arg(zoom) << rc;
    sendMessageOnly(REQUEST_FLAG_SET_ZOOMINMODE, (void *)&zoom_info, sizeof(zoom_info));

    m_labelScale->setText(QString("%1%").arg(zoom));
    m_labelScale->show();
    m_timerAutoHide->start();
}

void LiveViewZoom::onTimerAutoHide()
{
    m_labelScale->setVisible(false);
}
