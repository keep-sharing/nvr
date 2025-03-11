#include "MsGraphicsObject.h"
#include "MyDebug.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

MsGraphicsObject::MsGraphicsObject(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
}

void MsGraphicsObject::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void MsGraphicsObject::filterMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

QRectF MsGraphicsObject::boundingRect() const
{
    return m_rect;
}

void MsGraphicsObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)

#if 0
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::red, 3));
    painter->setBrush(QBrush(QColor(0, 0, 255, 40)));
    painter->drawRect(boundingRect());
    painter->restore();
#endif
}

void MsGraphicsObject::setItemsSeleteFalse()
{

}

void MsGraphicsObject::setCurrentItem(DrawItemFacePolygon *item)
{
  Q_UNUSED(item)
}

void MsGraphicsObject::showConflict()
{

}

void MsGraphicsObject::regionFinish()
{

}

QVariant MsGraphicsObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemSceneHasChanged:
        if (m_scene) {
            disconnect(scene(), SIGNAL(sceneRectChanged(QRectF)), this, SLOT(onSceneRectChanged(QRectF)));
        }
        m_scene = scene();
        connect(m_scene, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(onSceneRectChanged(QRectF)));
        m_rect = m_scene->sceneRect();
        prepareGeometryChange();
        break;
    default:
        break;
    }

    return QGraphicsObject::itemChange(change, value);
}

qreal MsGraphicsObject::sceneWidth() const
{
    if (scene()) {
        return scene()->sceneRect().width();
    } else {
        return 0;
    }
}

qreal MsGraphicsObject::sceneHeight() const
{
    if (scene()) {
        return scene()->sceneRect().height();
    } else {
        return 0;
    }
}

QRectF MsGraphicsObject::sceneRect() const
{
    if (scene()) {
        return scene()->sceneRect();
    } else {
        return QRectF();
    }
}

void MsGraphicsObject::setIpcMaxSize(int w, int h)
{
    m_ipcMaxWidth = w;
    m_ipcMaxHeight = h;
}

QPointF MsGraphicsObject::mapToQtPos(int x, int y)
{
    QMS_ASSERT(scene());

    const QRectF sceneRect = scene()->sceneRect();
    return QPointF((qreal)x / m_ipcMaxWidth * sceneRect.width(), (qreal)y / m_ipcMaxHeight * sceneRect.height());
}

QPoint MsGraphicsObject::mapToIpcPos(const QPointF &pos)
{
    QMS_ASSERT(scene());

    const QRectF sceneRect = scene()->sceneRect();
    return QPoint(pos.x() / sceneRect.width() * m_ipcMaxWidth, pos.y() / sceneRect.height() * m_ipcMaxHeight);
}

void MsGraphicsObject::onSceneRectChanged(const QRectF &rect)
{
    m_rect = rect;
}
