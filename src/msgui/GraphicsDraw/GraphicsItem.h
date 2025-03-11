#ifndef GRAPHICSITEM_H
#define GRAPHICSITEM_H

#include <QGraphicsItem>

enum GraphicsZValue {
    ZValueNormal = 0,
    ZValueDraw,
    ZValuePos
};

class GraphicsItem : public QGraphicsItem {
public:
    explicit GraphicsItem(QGraphicsItem *parent = nullptr);
};

#endif // GRAPHICSITEM_H
