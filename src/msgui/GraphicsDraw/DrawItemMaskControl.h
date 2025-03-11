#ifndef DRAWITEMMASKCONTROL_H
#define DRAWITEMMASKCONTROL_H

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include "DrawItemMask.h"
extern "C" {
#include "msg.h"
}


class DrawItemMaskControl : public QGraphicsRectItem
{
    protected:
    enum OperationMode
    {
        ModeNew,            //新建一个item
        ModeOld,           //操作旧的一个item
        ModeNull
    };
public:
    enum {
        Type = UserType + 5
    };
    explicit DrawItemMaskControl(QGraphicsItem *parent = nullptr);

    void init();
    QRectF boundingRect() const override;
    int type() const override;
    void setMaxItemCount(int count);
    void clear();
    void clear(int index);
    void clearAll();
    void clearSelected();
    void setEnabled(bool enable);
    void setVisible(bool visible);

    void addMask(const mask_area_ex &area);
    void getMask(mask_area_ex *area_array);
    void updateMask(mask_area_ex *area_array, int count = MAX_MASK_AREA_NUM + MAX_MOSAIC_NUM);

    int getCurrentItemID();

    void setColorType(int type);

    bool itemTextVisible() const;
    void setItemTextVisible(bool newItemTextVisible);

    const QSize &itemMinSize() const;
    void setItemMinSize(const QSize &newItemMinSize);

    qreal itemBorderWidth() const;
    void setItemBorderWidth(qreal width);

    void refreshstack();

    void updateSingleColor(mask_area_ex *area_array, int color, int count = MAX_MASK_AREA_NUM + MAX_MOSAIC_NUM);

    //mosaic
    bool checkRegionNum(const mask_area_ex *area_array, int count = MAX_MASK_AREA_NUM + MAX_MOSAIC_NUM);
    void setSelectedItem(int index);

    bool getIsSupportMosaic() const;
    void setIsSupportMosaic(bool value);


    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

protected:
    int m_maxItemCount = 24;

    DrawItemMask *m_currentItem = nullptr;
    QList<DrawItemMask *> m_maskList;
    int m_colorType = 0;

    bool m_pressed = false;
    QPointF m_pressedPoint;

    bool m_isEnabled = false;
    OperationMode m_operation = ModeNull;

    bool m_itemTextVisible = true;
    QSize m_itemMinSize;

    qreal m_itemBorderWidth = 1;
    bool isSupportMosaic = false;
};

#endif // DRAWITEMMASKCONTROL_H
