#include "SubHistogramItem.h"
#include <QPainter>
#include "TextItem.h"

SubHistogramItem::SubHistogramItem(const QColor &color, int value, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    m_color(color),
    m_value(value)
{
    m_textItem = new TextItem(this);
    m_textItem->setTextColor(m_color);
    m_textItem->setText(QString("%1").arg(m_value));
    m_textItem->hide();
}

void SubHistogramItem::setRect(const QRectF &rect)
{
    QGraphicsRectItem::setRect(rect);

    //
    m_textItem->adjustPos(rect.topLeft());
}

void SubHistogramItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_color);
    painter->drawRect(rect());
    painter->restore();
}

void SubHistogramItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    if (m_textItem->isVisible())
    {
        m_textItem->hide();
    }
    else
    {
        m_textItem->show();
    }
}
