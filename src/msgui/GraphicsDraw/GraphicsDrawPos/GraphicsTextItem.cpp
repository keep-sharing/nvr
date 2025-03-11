#include "GraphicsTextItem.h"
#include <QPainter>
#include <QGraphicsScene>
#include "MyDebug.h"

GraphicsTextItem::GraphicsTextItem(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WrapAnywhere);
    m_textLayout.setTextOption(textOption);
}

void GraphicsTextItem::setText(const QString &text, int size, QColor color)
{
    m_textLayout.setText(text);
    m_text = text;
    m_fontSize = size;
    m_fontColor = color;
}

void GraphicsTextItem::setTextWidth(qreal width)
{
    m_textWidth = width;

    QFontMetrics fontMetrics(font());
    int leading = fontMetrics.leading();
    qreal height = 0;
    //m_textLayout.setCacheEnabled(true);
    m_textLayout.setFont(font());
    m_textLayout.beginLayout();
    while (1) {
        QTextLine line = m_textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(textWidth());
        height += leading;
        line.setPosition(QPointF(0, height));
        height += line.height();
    }
    m_textLayout.endLayout();
}

qreal GraphicsTextItem::textWidth() const
{
    return m_textWidth;
}

void GraphicsTextItem::setMaxHeight(qreal height)
{
    m_maxHeight = height;
}

QFont GraphicsTextItem::font() const
{
    QFont f = scene()->font();
    f.setPointSize(m_fontSize);
    return f;
}

QRectF GraphicsTextItem::boundingRect() const
{
    return m_textLayout.boundingRect();
}

void GraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();

    painter->setFont(font());
    painter->setPen(m_fontColor);

    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WrapAnywhere);

    QRectF rc = boundingRect();
    if (m_maxHeight > 0) {
        rc.setHeight(qMin(rc.height(), m_maxHeight));
    }
    painter->drawText(rc, m_text, textOption);

    painter->restore();
}
