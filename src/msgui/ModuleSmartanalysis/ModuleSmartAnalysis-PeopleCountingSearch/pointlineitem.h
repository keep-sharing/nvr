#ifndef POINTLINEITEM_H
#define POINTLINEITEM_H

/******************************************************************
* @brief    人数统计折线图上的点
* @author   LiuHuanyu
* @date     2021-07-16
******************************************************************/

#include <QGraphicsRectItem>

class PointLineItem : public QGraphicsRectItem
{
public:
    PointLineItem(const QColor &color, int value, QGraphicsItem *parent = nullptr);

    void setRect(const QRectF &rect);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QColor m_color;
    int m_value;

    QGraphicsTextItem *m_textItem = nullptr;
};

#endif // POINTLINEITEM_H
