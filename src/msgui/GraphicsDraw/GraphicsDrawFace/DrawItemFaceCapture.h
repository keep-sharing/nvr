#ifndef DRAWITEMFACECAPTURE_H
#define DRAWITEMFACECAPTURE_H

#include "DrawItemFaceMinDetection.h"
#include "MsGraphicsObject.h"

extern "C" {
#include "msg.h"
}
class DrawItemFaceCapture : public MsGraphicsObject {
    Q_OBJECT
public:
    enum {
        Type = UserType + 1
    };
    enum FaceCaptureMode {
        DetectionMode = 0,
        ShieldMode
    };

    explicit DrawItemFaceCapture(QGraphicsItem *parent = nullptr);
    void init();
    void startDraw();
    void setMinSize(int value);
    int type() const override;
    void setFaceMode(FaceCaptureMode mode);
    void regionFinish() override;
    void setItemEnable(bool enable);
    void setCurrentItem(DrawItemFacePolygon *item) override;
    void setItemsSeleteFalse() override;
    int findNewRegionIndex();
    void findCurrentItem();

    void clearAll();
    int clear();
    void clear(int index);
    void detectionSetAll();

    void showConflict() override;

    void setDetection(const QString &xList, const QString &yList);
    void getDetection(MS_POLYGON *polygon);
    void addShield(const MS_POLYGON &polygon, const int index);
    void getShield(MS_FACE_SHIELD *polygon);
    void updataShield(MS_FACE_SHIELD *polygon);
    void clearSelect();
    void setSelectNull();
    void refreshstack();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

private:
    bool clickedRegion(QPointF point);

    QPointF physicalPoint(const QPoint &p) const;
    QPoint logicalPoint(const QPointF &p) const;
signals:
    void conflicted();

private:
    int m_maxRegion = 4;

    bool m_pressed = false;
    QPointF m_pressPoint;
    int m_currentIndex = -1;

    DrawItemFacePolygon *m_selectedRegion = nullptr;
    QList<DrawItemFacePolygon *> m_regionList;
    DrawItemFacePolygon *m_detectionRegion = nullptr;
    DrawItemFaceMinDetection *m_minDetection = nullptr;

    FaceCaptureMode m_mode = DetectionMode;
};

#endif // DRAWITEMFACECAPTURE_H
