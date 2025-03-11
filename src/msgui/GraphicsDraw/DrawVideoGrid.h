#ifndef DRAWVIDEOGRID_H
#define DRAWVIDEOGRID_H

#include "DrawVideoBase.h"

class DrawVideoGrid : public DrawVideoBase {
    enum RegionType {
        RegionGrid,
        RegionPolygon
    };

    enum MotionType {
        TypeDraw,
        TypeClean
    };

    struct MotionPoint {
        int row = 0;
        int column = 0;

        MotionPoint()
        {
        }

        MotionPoint(int r, int c)
        {
            row = r;
            column = c;
        }

        bool operator<(const MotionPoint &other) const
        {
            if (row != other.row) {
                return row < other.row;
            } else {
                return column < other.column;
            }
        }
    };

public:
    DrawVideoGrid(QGraphicsItem *parent = nullptr);

    void setRegion(char *region, const QString &xList, const QString &yList);
    void setPolygon(const QString &xList, const QString &yList);

    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QPointF physicalPoint(const QPoint &p) const;

    void updatePolygon();

private:
    RegionType m_regionType = RegionGrid;
    QMap<MotionPoint, MotionType> m_motionMap;

    QString m_xList;
    QString m_yList;
    QVector<QPointF> m_points;
};

#endif // DRAWVIDEOGRID_H
