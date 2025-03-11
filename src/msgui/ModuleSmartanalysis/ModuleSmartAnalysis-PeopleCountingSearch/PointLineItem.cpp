#include "pointlineitem.h"
#include <QPainter>

PointLineItem::PointLineItem(const QColor &color, int value, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    m_color(color),
    m_value(value)
{
    m_textItem = new QGraphicsTextItem(this);
    m_textItem->setAcceptHoverEvents(false);
    m_textItem->setDefaultTextColor(m_color);
    m_textItem->setPlainText(QString("%1").arg(m_value));
    m_textItem->adjustSize();
    m_textItem->hide();
}

void PointLineItem::setRect(const QRectF &rect)
{
    QGraphicsRectItem::setRect(rect);

    //
    QRectF textRc = m_textItem->boundingRect();
    textRc.moveCenter(rect.center());
    textRc.moveBottom(rect.top() + 6);
    m_textItem->setPos(textRc.topLeft());
}

void PointLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_color);
    painter->drawEllipse(rect());
    painter->restore();
}

void PointLineItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
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
