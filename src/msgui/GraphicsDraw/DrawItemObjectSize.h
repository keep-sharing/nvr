#ifndef DRAWITEMOBJECTSIZE_H
#define DRAWITEMOBJECTSIZE_H

#include <QGraphicsRectItem>
#include "msqtvca.h"

class DrawItemObjectSize : public QGraphicsRectItem {
    enum OperationMode {
        ModeResizeLeft, //改变一个item大小
        ModeResizeTopLeft,
        ModeResizeTop,
        ModeResizeTopRight,
        ModeResizeRight,
        ModeResizeBottomRight,
        ModeResizeBottom,
        ModeResizeBottomLeft,
        ModeNew, //新建一个item
        ModeMove, //移动一个item
        ModeNull
    };

    enum AnchorPosition {
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
    DrawItemObjectSize(QGraphicsItem *parent = nullptr);

    void setObjectRect(const QRectF &rect, const QString &text, MsQtVca::ObjectSizeType sizeType);
    void setItemRect(const QRectF &rect);
    QRectF objectRect() const;

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
    QRectF m_objectRect;
    QRectF m_tempGeometry;
    QMap<AnchorPosition, QRectF> m_anchorMap;
    OperationMode m_operation = ModeNull;

    bool m_pressed = false;
    QPointF m_pressPoint;
    QPointF m_pressDistance;

    QString m_text;
    MsQtVca::ObjectSizeType m_sizeType = MsQtVca::MinSize;
};

#endif // DRAWITEMOBJECTSIZE_H
