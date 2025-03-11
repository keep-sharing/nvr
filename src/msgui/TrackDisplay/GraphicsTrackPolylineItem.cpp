#include "GraphicsTrackPolylineItem.h"
#include <QPainter>

GraphicsTrackPolylineItem::GraphicsTrackPolylineItem(QGraphicsItem *item)
    : QGraphicsItem(item)
{
}

void GraphicsTrackPolylineItem::setTrackInfo(const TrackInfo &info)
{
    m_info = info;
    m_path = QPainterPath();
    const auto &points = info.trackPoints;
    for (int i = 0; i < points.size(); ++i) {
        const QPointF &p = points.at(i);
        if (i == 0) {
            m_path.moveTo(p);
        } else {
            m_path.lineTo(p);
        }
    }
    //prepareGeometryChange();
}

bool GraphicsTrackPolylineItem::valid() const
{
    return m_valid;
}

void GraphicsTrackPolylineItem::setValid(bool valid)
{
    m_valid = valid;
}

QRectF GraphicsTrackPolylineItem::boundingRect() const
{
    QRectF rc = m_path.boundingRect();
    rc.adjust(-10, -10, 10, 10);
    return rc;
}

void GraphicsTrackPolylineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHints(QPainter::Antialiasing);

    //
    switch (m_info.mode) {
    case TrackModeVca: {
        //line
        painter->save();
        painter->setPen(QPen(m_info.lineColor, 2));
        painter->drawPolyline(QPolygonF(m_info.trackPoints));
        painter->restore();

        //point
        painter->save();
        painter->setPen(QPen(m_info.pointColor, 8));
        painter->drawPoints(QPolygonF(m_info.trackPoints));
        painter->restore();
        break;
    }
    default:
        break;
    }
}
