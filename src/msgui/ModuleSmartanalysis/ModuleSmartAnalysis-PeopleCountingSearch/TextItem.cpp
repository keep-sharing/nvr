#include "TextItem.h"
#include <QPainter>
#include <QGraphicsScene>

TextItem::TextItem(QGraphicsItem *parent) :
    QGraphicsRectItem(parent)
{

}

void TextItem::setTextColor(const QColor &color)
{
    m_textColor = color;
}

void TextItem::setText(const QString &text)
{
    m_text = text;
}

void TextItem::adjustPos(const QPointF &pos)
{
    QFontMetrics fm(scene()->font());
    int w = fm.width(m_text);
    int h = fm.height();
    QRectF rc(0, 0, w + 6, h);
    rc.moveTop(pos.y());
    rc.moveRight(pos.x());
    setRect(rc);
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(m_borderColor);
    painter->setBrush(m_backgroundColor);
    painter->drawRoundedRect(rect(), 5, 5);

    painter->setPen(m_textColor);
    painter->drawText(rect(), Qt::AlignCenter, m_text);
    painter->restore();
}
