#ifndef SUBHISTOGRAMITEM_H
#define SUBHISTOGRAMITEM_H

#include <QGraphicsRectItem>

class TextItem;

class SubHistogramItem : public QGraphicsRectItem
{
public:
    SubHistogramItem(const QColor &color, int value, QGraphicsItem *parent = nullptr);

    void setRect(const QRectF &rect);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QColor m_color;
    int m_value;

    TextItem *m_textItem = nullptr;
};

#endif // SUBHISTOGRAMITEM_H
