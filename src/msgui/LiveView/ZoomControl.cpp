#include "ZoomControl.h"
#include "MsDevice.h"
#include "SubControl.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QtDebug>

ZoomControl::ZoomControl(QWidget *parent)
    : QWidget(parent)
{
    setCursor(Qt::OpenHandCursor);
}

int ZoomControl::nvrWidth() const
{
    return (qreal)width() * m_nvrScreenSize.width() / m_qtScreenSize.width();
}

int ZoomControl::nvrHeight() const
{
    return (qreal)height() * m_nvrScreenSize.height() / m_qtScreenSize.height();
}

void ZoomControl::clearZoom()
{
    m_zoom = 100;
    m_scale = 1;
}

void ZoomControl::showEvent(QShowEvent *event)
{
    int nvrWidth = 0;
    int nvrHeight = 0;
    vapi_get_screen_res(SubControl::instance()->currentScreen(), &nvrWidth, &nvrHeight);
    m_nvrScreenSize = QSize(nvrWidth, nvrHeight);
    //
    QRect screenRect = qApp->desktop()->geometry();
    m_qtScreenSize = screenRect.size();

    QWidget::showEvent(event);
}

void ZoomControl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QWidget::mousePressEvent(event);
    } else if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
        m_isPressed = true;
        m_pressDistance = event->globalPos() - pos();
    }
}

void ZoomControl::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (event->button() == Qt::RightButton) {
        close();
        QWidget::mouseReleaseEvent(event);
    } else if (event->button() == Qt::LeftButton) {
        setCursor(Qt::OpenHandCursor);
        m_isPressed = false;
    }
}

void ZoomControl::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isPressed) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (m_zoom == 100) {
        return;
    }

    QPoint p = event->globalPos() - m_pressDistance;
    if (p.x() > 0) {
        p.setX(0);
    }
    if (p.y() > 0) {
        p.setY(0);
    }
    if (p.x() + width() < parentWidget()->width()) {
        p.setX(parentWidget()->width() - width());
    }
    if (p.y() + height() < parentWidget()->height()) {
        p.setY(parentWidget()->height() - height());
    }
    //
    if (p != pos()) {
        move(p);
        //
        sendZoom();
    }
}

void ZoomControl::wheelEvent(QWheelEvent *event)
{
    QPoint mousePoint = event->pos();
    int numDegrees = event->delta() / 8;
    if (numDegrees > 0) {
        if (m_zoom < 1000) {
            m_zoom += 25;
        }
    } else {
        if (m_zoom > 100) {
            m_zoom -= 25;
        }
    }

    qreal lastScale = m_scale;
    m_scale = m_zoom / 100.0;

    if (lastScale == m_scale) {
        return;
    }

    QPoint p = mapToParent(mousePoint);
    QTransform transform;
    transform.translate(p.x(), p.y());
    transform.scale(m_scale / lastScale, m_scale / lastScale);
    transform.translate(-p.x(), -p.y());
    QRect previousRc = geometry();
    QRect nextRc = transform.mapRect(previousRc);

    setGeometry(nextRc);

    //
    sendZoom();
}

void ZoomControl::sendZoom()
{
    QSize parentSize = parentWidget()->size();
    QRect rc;
    if (m_zoom == 100) {
        rc = QRect(QPoint(0, 0), parentSize);
        setGeometry(rc);
    } else {
        rc.setLeft(-geometry().left() / m_scale);
        rc.setTop(-geometry().top() / m_scale);
        rc.setWidth(parentSize.width() / m_scale);
        rc.setHeight(parentSize.height() / m_scale);

        QRect tempRc = geometry();
        if (rc.left() < 0) {
            rc.moveLeft(0);
            tempRc.moveLeft(0);
        }
        if (rc.top() < 0) {
            rc.moveTop(0);
            tempRc.moveTop(0);
        }
        setGeometry(tempRc);
    }

    //
    if (m_nvrScreenSize != m_qtScreenSize) {
        QRect tempRc;
        tempRc.setX((qreal)rc.x() * m_nvrScreenSize.width() / m_qtScreenSize.width());
        tempRc.setY((qreal)rc.y() * m_nvrScreenSize.height() / m_qtScreenSize.height());
        tempRc.setWidth((qreal)rc.width() * m_nvrScreenSize.width() / m_qtScreenSize.width());
        tempRc.setHeight((qreal)rc.height() * m_nvrScreenSize.height() / m_qtScreenSize.height());
        rc = tempRc;
    }

    //
    qDebug() << QString("ZoomControl::sendZoom, scale: %1%").arg(m_zoom) << rc;
    emit zoomChanged(m_zoom, rc);
}
