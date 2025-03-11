#ifndef GRAPHICSPOSDISPLAYREGIONITEM_H
#define GRAPHICSPOSDISPLAYREGIONITEM_H

#include "DrawItemMask.h"

class GraphicsPosDisplayRegionItem : public DrawItemMask
{
public:
    GraphicsPosDisplayRegionItem(QGraphicsItem *parent = nullptr);

protected:
    virtual bool itemTextVisible() const override;
    virtual QColor borderColor() const override;
};

#endif // GRAPHICSPOSDISPLAYREGIONITEM_H
