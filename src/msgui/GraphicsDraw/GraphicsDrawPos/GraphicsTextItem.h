#ifndef GRAPHICSTEXTITEM_H
#define GRAPHICSTEXTITEM_H

#include <QGraphicsRectItem>
#include <QTextLayout>

class GraphicsTextItem : public QGraphicsRectItem
{
public:
    explicit GraphicsTextItem(QGraphicsItem *parent = nullptr);

    void setText(const QString &text, int size, QColor color);

    void setTextWidth(qreal width);
    qreal textWidth() const;

    void setMaxHeight(qreal height);

    QFont font() const;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QTextLayout m_textLayout;
    qreal m_textWidth = 0;
    qreal m_maxHeight = 0;

    QString m_text;
    int m_fontSize = 0;
    QColor m_fontColor;
};

#endif // GRAPHICSTEXTITEM_H
