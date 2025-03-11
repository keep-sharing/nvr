#ifndef DRAWITEMPTZMASKCONTROL_H
#define DRAWITEMPTZMASKCONTROL_H

#include "DrawItemMaskControl.h"

class DrawItemPTZMaskControl : public DrawItemMaskControl
{
public:
    explicit DrawItemPTZMaskControl(QGraphicsItem *parent = nullptr);

    void editArea(const mask_area_ex &area);
    void hideArea();
    void hideArea(int index);
    void setItemRatio(int index, int ratio);
    void setCurrentItemSelected(bool enable);
    void cancelCurrentItem();

    void addMask(const mask_area_ex &area);
    Uint64 getMask(mask_area_ex *area_array);
    Uint64 updateMask(mask_area_ex *area_array, int count = MAX_MASK_AREA_NUM);


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // DRAWITEMPTZMASKCONTROL_H
