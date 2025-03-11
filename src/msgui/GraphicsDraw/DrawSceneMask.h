#ifndef DRAWSCENEMASK_H
#define DRAWSCENEMASK_H

#include "DrawItemMask.h"
#include <QGraphicsScene>

extern "C" {
#include "msg.h"
}

class DrawSceneMask : public QGraphicsScene {
    Q_OBJECT

protected:
    enum OperationMode {
        ModeNew, //新建一个item
        ModeOld, //操作旧的一个item
        ModeNull
    };

public:
    explicit DrawSceneMask(QObject *parent = nullptr);

    void setMaxItemCount(int count);
    void clear();
    void clear(int index);
    void clearAll();
    void clearSelected();
    void setEnabled(bool enable);
    void setVisible(bool visible);

    void addMask(const mask_area_ex &area);
    void getMask(mask_area_ex *area_array, int count = MAX_MASK_AREA_NUM + MAX_MOSAIC_NUM);
    void updateMask(mask_area_ex *area_array, int count = MAX_MASK_AREA_NUM + MAX_MOSAIC_NUM);

    int getCurrentItemID();

    void setColorType(int type);

    bool itemTextVisible() const;
    void setItemTextVisible(bool newItemTextVisible);

    const QSize &itemMinSize() const;
    void setItemMinSize(const QSize &newItemMinSize);

    qreal itemBorderWidth() const;
    void setItemBorderWidth(qreal width);

    //pos
    void setPosItemRegion(int x, int y, int w, int h);
    void getPosItemRegion(int &x, int &y, int &w, int &h);
    QRect posGlobalGeometry() const;

    //mosaic
    bool checkRegionNum(const mask_area_ex *area_array, int count = MAX_MASK_AREA_NUM + MAX_MOSAIC_NUM);
    void setSelectedItem(int index);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void mouseDragging();

public slots:

protected:
    int m_maxItemCount = 24;

    DrawItemMask *m_currentItem = nullptr;
    QList<DrawItemMask *> m_maskList;
    int m_colorType = 0;

    bool m_pressed = false;
    QPointF m_pressedPoint;

    bool m_isEnabled          = false;
    OperationMode m_operation = ModeNull;

    bool m_itemTextVisible = true;
    QSize m_itemMinSize;

    qreal m_itemBorderWidth = 1;
};

#endif // DRAWSCENEMASK_H
