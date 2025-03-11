#include "GraphicsPosDisplayRegionItem.h"

GraphicsPosDisplayRegionItem::GraphicsPosDisplayRegionItem(QGraphicsItem *parent)
    : DrawItemMask(parent)
{
    setItemMinSize(QSize(150, 50));
    setBorderWidth(3);
}

bool GraphicsPosDisplayRegionItem::itemTextVisible() const
{
    return false;
}

QColor GraphicsPosDisplayRegionItem::borderColor() const
{
    return DrawItemMask::borderColor();
}
