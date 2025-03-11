#ifndef DRAWITEMPOINT_H
#define DRAWITEMPOINT_H

#include <QGraphicsRectItem>

class DrawItem_Polygon;

class DrawItemPoint : public QGraphicsRectItem
{
public:
    explicit DrawItemPoint(QGraphicsItem *parent = nullptr);

    //中心点在父坐标系中的位置
    QPointF centerInParent() const;
    //中心点在scene坐标系中的位置
    QPointF centerInScene() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setPointColor(const QColor &pointColor);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    bool m_isPressed = false;
    QPointF m_pressScenePos;
    QPointF m_startPos;
    DrawItem_Polygon *m_parentItem = nullptr;
    QColor m_pointColor;
};

#endif // DRAWITEMPOINT_H
