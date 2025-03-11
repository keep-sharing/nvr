#ifndef DRAWSCENEPTZMASK_H
#define DRAWSCENEPTZMASK_H

#include "DrawSceneMask.h"

class DrawScenePtzMask : public DrawSceneMask
{
    Q_OBJECT
public:
    explicit DrawScenePtzMask(QObject *parent = nullptr);

    void AreaEdit(const mask_area_ex &area);
    void hideArea();
    void hideArea(int index);
    void setItemRatio(int index, int ratio);
    void setCurrentItemSelected(bool enable);
    void cancelCurrentItem();

    void addMask(const mask_area_ex &area);
    void getMask(mask_area_ex *area_array, int count = MAX_MASK_AREA_NUM);
    void updateMask(mask_area_ex *area_array, int count = MAX_MASK_AREA_NUM);


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // DRAWSCENEPTZMASK_H
