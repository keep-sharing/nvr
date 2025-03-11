#ifndef DRAWSCENEROI_H
#define DRAWSCENEROI_H

#include "DrawItemRoi.h"

#include <QGraphicsScene>

extern "C" {
#include "msg.h"
}

class DrawSceneRoi : public QGraphicsScene {
    Q_OBJECT

protected:
    enum OperationMode {
        ModeNew, //新建一个item
        ModeOld, //操作旧的一个item
        ModeNull
    };

public:
    explicit DrawSceneRoi(QObject *parent = nullptr);

    void setMaxItemCount(int count);
    void clear();
    void clear(int index);
    void clearAll();
    void setEnabled(bool enable);
    void setVisible(bool visible);

    void addRoiArea(const image_roi_area &area, int index);
    void getRoiArea(image_roi_area *area_array, int count);
    void updateRotArea(image_roi_area *area_array, int count);

    int getRoiWidth(const image_roi_area &area);
    int getRoiHeight(const image_roi_area &area);
    int getCurrentItemID();

    void setColorType(int type);
    void refreshstack();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

signals:

public slots:

protected:
    int m_maxItemCount = 8;

    DrawItemRoi *m_currentItem = nullptr;
    QList<DrawItemRoi *> m_maskList;
    int m_colorType = 0;

    bool m_pressed = false;
    QPointF m_pressedPoint;

    bool m_isEnabled = false;
    OperationMode m_operation = ModeNull;
};

#endif // DRAWSCENEROI_H
