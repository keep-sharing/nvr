#ifndef GRAPHICSDRAWGRID_H
#define GRAPHICSDRAWGRID_H

#include "GridPoint.h"
#include "MsGraphicsObject.h"

class GraphicsDrawGrid : public MsGraphicsObject {
    Q_OBJECT

    enum GridType {
        TypeDraw, //已经画的
        TypeClean, //空白的
        TypeTempDraw, //要画的
        TypeTempClean //要清除的
    };

public:
    explicit GraphicsDrawGrid(QGraphicsItem *parent = nullptr);

    void setRegion(char *region);
    void getRegion(char *region);

    void selectAll();
    void clearAll();

    bool hasDrawRegion();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void drawMotionMap(QPainter *painter, const QMap<GridPoint, GridType> &gridMap);

signals:

private:
    QMap<GridPoint, GridType> m_motionMap;
    QMap<GridPoint, GridType> m_tempMotionMap;
    QPointF m_pressPos;
    QPointF m_movePos;
    bool m_pressed = false;
};

#endif // GRAPHICSDRAWGRID_H
