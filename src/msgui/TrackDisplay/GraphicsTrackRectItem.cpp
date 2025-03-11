#include "GraphicsTrackRectItem.h"
#include "MyDebug.h"
#include <QPainter>

GraphicsTrackRectItem::GraphicsTrackRectItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
}

void GraphicsTrackRectItem::setTrackInfo(const TrackInfo &info)
{
    m_info = info;
    //prepareGeometryChange();
}

bool GraphicsTrackRectItem::valid() const
{
    return m_valid;
}

void GraphicsTrackRectItem::setValid(bool valid)
{
    m_valid = valid;
}

QRectF GraphicsTrackRectItem::boundingRect() const
{
    QRectF rc = m_info.rc;
    rc.adjust(-10, -10, 10, 10);
    return rc;
}

QPainterPath GraphicsTrackRectItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void GraphicsTrackRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHints(QPainter::Antialiasing);

    //
    painter->setPen(QPen(m_info.rcColor, 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(m_info.rc);

    //
    switch (m_info.mode) {
    case TrackModeVca:
    case TrackModeFisheye: {
        //id
        QPointF textPoint = m_info.rc.topLeft();
        textPoint.setX(textPoint.x() + 3);
        textPoint.setY(textPoint.y() + 15);
        painter->setPen(QColor(239, 236, 17));
        painter->drawText(textPoint, QString("ID: %1").arg(m_info.id));
        break;
    }
    default:
        break;
    }
}
