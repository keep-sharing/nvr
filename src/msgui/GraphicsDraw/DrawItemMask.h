#ifndef DRAWITEMMASK_H
#define DRAWITEMMASK_H

#include <QGraphicsRectItem>

class DrawItemMask : public QGraphicsRectItem
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
    enum {
        Type = UserType + 5
    };
    DrawItemMask(QGraphicsItem *parent = nullptr);

    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);

    void setIndex(int index);
    int getIndex() const;

    void setRatio(int ratio);
    int getRatio() const;

    int margin() const;

    QRectF realRect() const;
    void setRealRect(const QRectF &rc);

    bool isSelected() const;
    void setSelected(bool selected);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int type() const override;

    virtual bool itemTextVisible() const;
    void setItemTextVisible(bool newItemTextVisible);

    const QSize &itemMinSize() const;
    void setItemMinSize(const QSize &newItemMinSize);

    qreal borderWidth() const;
    void setBorderWidth(qreal newBorderWidth);
    void showText();

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual QColor borderColor() const;
    virtual QColor anchorColor() const;

    void dealResize(const QRectF &rect);
    OperationMode detectCursor(const QPointF &pos);

protected:
    qreal m_borderWidth = 1;
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
    int m_ratio = 1;

    bool m_itemTextVisible = true;
    QSize m_itemMinSize;
    QGraphicsTextItem *m_textItem;
};

#endif // DRAWITEMMASK_H
