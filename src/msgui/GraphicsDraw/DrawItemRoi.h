#ifndef DRAWITEMROI_H
#define DRAWITEMROI_H

#include <QGraphicsRectItem>

class DrawItemRoi : public QGraphicsRectItem
{
    enum OperationMode
    {
        ModeResizeLeft,     //改变一个item大小
        ModeResizeTopLeft,
        ModeResizeTop,
        ModeResizeTopRight,
        ModeResizeRight,
        ModeResizeBottomRight,
        ModeResizeBottom,
        ModeResizeBottomLeft,
        ModeNew,            //新建一个item
        ModeMove,           //移动一个item
        ModeNull
    };
    enum AnchorPosition
    {
        AnchorLeft,
        AnchorTopLeft,
        AnchorTop,
        AnchorTopRight,
        AnchorRight,
        AnchorBottomRight,
        AnchorBottom,
        AnchorBottomLeft
    };

public:
    DrawItemRoi(QGraphicsItem *parent = nullptr);

    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);

    void setIndex(int index);
    int getIndex() const;

    int margin() const;

    QRectF realRect() const;
    void setRealRect(const QRectF &rc);

    bool isSelected() const;
    void setSelected(bool selected);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void dealResize(const QRectF &rect);
    OperationMode detectCursor(const QPointF &pos);

private:
    QColor m_color;
    int m_margin = 4;
    bool m_isSelected = false;
    QRectF m_realRect;
    QRectF m_tempGeometry;
    QMap<AnchorPosition, QRectF> m_anchorMap;
    OperationMode m_operation = ModeNull;

    bool m_pressed = false;
    QPointF m_pressPoint;
    QPointF m_pressDistance;
    int m_index = -1;
};

#endif // DRAWITEMROI_H
