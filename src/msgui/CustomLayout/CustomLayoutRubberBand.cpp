#include "CustomLayoutRubberBand.h"
#include <QPainter>

CustomLayoutRubberBand::CustomLayoutRubberBand(QGraphicsItem *parent) :
    QGraphicsRectItem(parent)
{

}

void CustomLayoutRubberBand::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(QColor(185, 185, 185));
    painter->setBrush(QColor(10, 168, 227, 50));
    painter->drawRect(rect());
}
