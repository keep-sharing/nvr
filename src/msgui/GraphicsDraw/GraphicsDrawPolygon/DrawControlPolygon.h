#ifndef DRAWCONTROLPOLYGON_H
#define DRAWCONTROLPOLYGON_H

#include "MsGraphicsObject.h"
extern "C" {
#include "msg.h"
}
class DrawItem_Polygon;

class DrawControlPolygon : public MsGraphicsObject {
    Q_OBJECT
public:
    explicit DrawControlPolygon(QGraphicsItem *parent = nullptr);

    void setPolygon(const QString &xList, const QString &yList);
    void getPolygon(char *xList, int xSize, char *yList, int ySize);

    void selectAll();
    void clearAll();
    void clearPolygon();

    void showConflict();

    bool isFinished();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    QPointF physicalPoint(const QPoint &p) const;
    QPoint logicalPoint(const QPointF &p) const;

    virtual DrawItem_Polygon *currentItem();

signals:
    void conflicted();

private:
    DrawItem_Polygon *m_currentItem = nullptr;

public slots:
};

#endif // DRAWCONTROLPOLYGON_H
