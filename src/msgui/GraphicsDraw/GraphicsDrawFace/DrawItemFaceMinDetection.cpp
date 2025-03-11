#include "DrawItemFaceMinDetection.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
DrawItemFaceMinDetection::DrawItemFaceMinDetection(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
}

void DrawItemFaceMinDetection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();

    drawObjectSize(painter);

    painter->restore();
}

void DrawItemFaceMinDetection::setObjectSize(int value)
{
    m_objectSize = value;
    update();
}

int DrawItemFaceMinDetection::type() const
{
    return Type;
}

void DrawItemFaceMinDetection::drawObjectSize(QPainter *painter)
{
    if (m_objectSize > 0) {
        painter->save();

        QRectF objectRect = scene()->sceneRect();
        objectRect.setWidth(scene()->sceneRect().width() * m_objectSize /2160);
        objectRect.setHeight(objectRect.width());
        objectRect.moveCenter(scene()->sceneRect().center());

        painter->setPen(QPen(QColor("#FF0000")));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(objectRect);

        painter->drawText(static_cast<int>(objectRect.left()), static_cast<int>(objectRect.top()) - 2, "Min. Detection Size");

        painter->restore();
    }
}
