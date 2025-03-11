#include "TotalHistogramItem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

TotalHistogramItem::TotalHistogramItem(const QColor &color, int value, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    m_color(color),
    m_value(value)
{
    m_textItem = new QGraphicsTextItem(this);
    m_textItem->setDefaultTextColor(m_color);
    m_textItem->setAcceptHoverEvents(false);
    m_textItem->setPlainText(QString("%1").arg(m_value));
    m_textItem->adjustSize();
}

void TotalHistogramItem::setRect(const QRectF &rect)
{
    QGraphicsRectItem::setRect(rect);

    QRectF textRc = m_textItem->boundingRect();
    textRc.moveCenter(rect.center());
    textRc.moveBottom(rect.top() + 5);
    m_textItem->setPos(textRc.topLeft());
}

void TotalHistogramItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_color);
    painter->drawRect(rect());
    painter->restore();
}
