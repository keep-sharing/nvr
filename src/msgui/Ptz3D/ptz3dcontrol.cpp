#include "ptz3dcontrol.h"
#include "ui_ptz3dcontrol.h"
#include "centralmessage.h"
#include "ptzcontrol.h"
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QtDebug>

PTZ3DControl *PTZ3DControl::s_ptz3dControl = nullptr;

PTZ3DControl::PTZ3DControl(QWidget *parent)
    : BaseDialog(parent)
    , ui(new Ui::PTZ3DControl)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);

    s_ptz3dControl = this;

    installEventFilter(this);

    //
    m_sendZoomTimer = new QTimer(this);
    m_sendZoomTimer->setSingleShot(true);
    m_sendZoomTimer->setInterval(500);
    connect(m_sendZoomTimer, SIGNAL(timeout()), this, SLOT(onSendZoomTimer()));
}

PTZ3DControl::~PTZ3DControl()
{
    s_ptz3dControl = nullptr;
    m_ptzWidget = nullptr;
    delete ui;
}

PTZ3DControl *PTZ3DControl::instance()
{
    return s_ptz3dControl;
}

bool PTZ3DControl::isSupportPTZ3D(const QString &model)
{
    bool support = false;

    //Milesight PTZ模式仅支持MS-CXX41/42/61/71-XXXXX
    QRegExp rc("MS-C\\d\\d(\\d\\d)-");
    if (rc.indexIn(model) != -1) {
        int value = rc.cap(1).toInt();
        switch (value) {
        case 41:
        case 42:
        case 61:
        case 67:
        case 71:
            support = true;
            break;
        }
    }

    return support;
}

void PTZ3DControl::setChannel(int channel)
{
    m_channel = channel;
}

void PTZ3DControl::setPtzWidget(PtzControl *ptz)
{
    m_ptzWidget = ptz;
    m_ptzWidget->move(mapToGlobal(QPoint(0, 0)));
}

void PTZ3DControl::set3DEnable(bool enable)
{
    m_is3DEnable = enable;
}

void PTZ3DControl::setPtzEnable(bool enable)
{
    m_isPtzEnable = enable;

    if (m_isPtzEnable) {
        if (m_ptzWidget) {
            m_ptzWidget->show();
        }
    } else {
        if (m_ptzWidget) {
            m_ptzWidget->hide();
        }
    }
}

void PTZ3DControl::setManualTrackEnabled(bool enable)
{
    m_isManualTrackEnable = enable;

    qDebug() << "====PTZ3DControl::setManualTrackEnabled====";
    ms_manual_tracking tracking;
    memset(&tracking, 0, sizeof(tracking));
    tracking.chanid = m_channel;
    tracking.manual_tracking = enable;
    qDebug() << "----REQUEST_FLAG_SET_MANUAL_TRACKING";
    qDebug() << "----chanid:" << m_channel;
    qDebug() << "----manual_tracking:" << tracking.manual_tracking;
    sendMessage(REQUEST_FLAG_SET_MANUAL_TRACKING, (void *)&tracking, sizeof(tracking));
}

/**
 * @brief PTZ3DControl::closePtz3DForPopup
 * event popup时关闭ptz 3d
 */
void PTZ3DControl::closePtz3DForPopup()
{
    ptzDone(1);
}

void PTZ3DControl::closePtz3DForSwitchScreen()
{
    ptzDone(2);
}

void PTZ3DControl::closePtz3D(ExitReason reason)
{
    ptzDone(reason);
}

void PTZ3DControl::ptzPanelControl(IPC_PTZ_CONTORL_TYPE cmd, int value)
{
    IpcPtzControl control;
    memset(&control, 0, sizeof(IpcPtzControl));
    control.chnId = m_channel;
    control.controlType = cmd;
    control.controlAction = value;
    sendMessageOnly(REQUEST_FLAG_SET_IPC_PTZ_CONTROL_JSON, &control, sizeof(IpcPtzControl));
}

bool PTZ3DControl::isMoveToCenter()
{
    return false;
}

bool PTZ3DControl::eventFilter(QObject *obj, QEvent *evt)
{
    switch (evt->type()) {
    case QEvent::ContextMenu:
    case QEvent::Wheel:
        return true;
        break;
    default:
        break;
    }
    return BaseDialog::eventFilter(obj, evt);
}

void PTZ3DControl::showEvent(QShowEvent *evt)
{
    showMaximized();
    BaseDialog::showEvent(evt);
}

void PTZ3DControl::hideEvent(QHideEvent *evt)
{
    BaseDialog::hideEvent(evt);
}

void PTZ3DControl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressedPoint = event->pos();
    } else if (event->button() == Qt::RightButton) {
        ptzDone(0);
    }
}

void PTZ3DControl::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;

    if (event->button() == Qt::LeftButton) {
        if (m_is3DEnable) {
            if ((event->pos() - m_pressedPoint).manhattanLength() < 5) {
                set3DPosition(m_pressedPoint);
            } else {
                set3DPosition(m_ptzRubberBand);
            }
        }
        if (m_isManualTrackEnable) {
            setManualTrack(m_ptzRubberBand);
        }
    }

    m_showRect = false;
    update();
}

void PTZ3DControl::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed && (m_is3DEnable || m_isManualTrackEnable)) {
        m_ptzRubberBand = QRect(m_pressedPoint, event->pos());
        m_showRect = true;
        update();
    }
    BaseDialog::mouseMoveEvent(event);
}

void PTZ3DControl::wheelEvent(QWheelEvent *event)
{
    if (m_is3DEnable) {
        if (!m_sendZoomTimer->isActive()) {
            m_sendZoomTimer->start();
        }

        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        m_zoomSteps += numSteps;
    }
    event->accept();
}

void PTZ3DControl::paintEvent(QPaintEvent *)
{
    if (m_showRect) {
        QPainter painter(this);
        painter.setPen(QPen(QColor(10, 169, 227), 3));
        painter.drawRect(m_ptzRubberBand);
    }
}

bool PTZ3DControl::isAddToVisibleList()
{
    return true;
}

void PTZ3DControl::escapePressed()
{
    ptzDone(0);
}

void PTZ3DControl::ptzDone(int result)
{
    if (m_isManualTrackEnable) {
        setManualTrackEnabled(false);
    }
    if (m_ptzWidget) {
        m_ptzWidget->hide();
    }

    done(result);
}

void PTZ3DControl::set3DPosition(int channel, const QPoint &point, int zoom)
{
    ms_3d_ptz_control_info ptzInfo;
    ptzInfo.chanid = channel;
    ptzInfo.ptz_cmd = 78;
    ptzInfo.ptz_3dposition_x = point.x();
    ptzInfo.ptz_3dposition_y = point.y();
    ptzInfo.ptz_3dposition_zoom = zoom;

    sendMessage(REQUEST_FLAG_SET_3D_PTZ_CTRL, (void *)&ptzInfo, sizeof(ms_3d_ptz_control_info));
}

void PTZ3DControl::set3DPosition(const QPoint &point)
{
    qreal x = (qreal)point.x() / width() * 100;
    qreal y = (qreal)point.y() / height() * 100;
    set3DPosition(m_channel, QPoint(x, y), 100);
}

void PTZ3DControl::set3DPosition(const QRect &rc)
{
    //放大还是缩小
    bool zoom = true;
    if (rc.width() < 0 || rc.height() < 0) {
        zoom = false;
    }
    QRect normalRect = rc.normalized();
    QPoint centerPoint = normalRect.center();

    qreal ratio = (qreal)rect().width() / rect().height();
    if ((qreal)normalRect.width() / normalRect.height() > ratio) {
        normalRect.setWidth(normalRect.height() * ratio);
    } else {
        normalRect.setHeight((qreal)normalRect.width() / ratio);
    }
    normalRect.moveCenter(centerPoint);

    //
    centerPoint.setX((qreal)centerPoint.x() / width() * 100);
    centerPoint.setY((qreal)centerPoint.y() / height() * 100);

    //
    qreal zoomRatio = (qreal)rect().width() / normalRect.width();
    if (zoomRatio > 5) {
        zoomRatio = 4;
    }
    if (zoom) {
        set3DPosition(m_channel, centerPoint, 100 * zoomRatio);
    } else {
        set3DPosition(m_channel, centerPoint, 100.0 / zoomRatio);
    }
}

/**
 * @brief PTZ3DControl::set3DPosition
 * @param zoom: 25-400，大于100为放大，小于100为缩小
 */
void PTZ3DControl::set3DPosition(int zoom)
{
    QPointF centerPoint = 0.5 * QPoint(0, 0) + 0.5 * QPoint(100, 100);
    set3DPosition(m_channel, centerPoint.toPoint(), zoom);
}

void PTZ3DControl::setManualTrack(const QRect &rc)
{
    qDebug() << "----PTZ3DControl::setManualTrack----";

    QRect normalRect = rc.normalized();

    MS_TRK_REGION region;
    memset(&region, 0, sizeof(region));
    region.chanid = m_channel;
    region.manual_tracking = 1;
    region.startX = (qreal)normalRect.left() / width() * 256;
    region.startY = (qreal)normalRect.top() / height() * 256;
    region.width = (qreal)normalRect.width() / width() * 256;
    region.height = (qreal)normalRect.height() / height() * 256;
    qDebug() << "----chanid:" << m_channel;
    qDebug() << "----(" << region.startX << "," << region.startY << "," << region.width << "," << region.height << ")";
    sendMessage(REQUEST_FLAG_SET_MANUAL_TRACKING_REGION, (void *)&region, sizeof(region));
}

void PTZ3DControl::onSendZoomTimer()
{
    if (m_zoomSteps < 0) {
        int zoom = 100 - 10 * qAbs(m_zoomSteps);
        if (zoom < 25) {
            zoom = 25;
        }
        //向后滚动，缩小
        set3DPosition(zoom);
    } else {
        int zoom = 100 + 10 * m_zoomSteps;
        if (zoom > 400) {
            zoom = 400;
        }
        //向前滚动，放大
        set3DPosition(zoom);
    }
    m_zoomSteps = 0;
}
