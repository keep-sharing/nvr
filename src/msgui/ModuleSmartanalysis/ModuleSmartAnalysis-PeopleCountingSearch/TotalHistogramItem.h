#ifndef TOTALHISTOGRAMITEM_H
#define TOTALHISTOGRAMITEM_H

#include <QGraphicsRectItem>

class TotalHistogramItem : public QGraphicsRectItem
{
public:
    TotalHistogramItem(const QColor &color, int value, QGraphicsItem *parent = nullptr);

    void setRect(const QRectF &rect);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QColor m_color;
    int m_value;

    QGraphicsTextItem *m_textItem = nullptr;
};

#endif // TOTALHISTOGRAMITEM_H
