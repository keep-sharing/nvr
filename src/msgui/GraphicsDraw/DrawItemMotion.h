#ifndef DRAWITEMMOTION_H
#define DRAWITEMMOTION_H

#include <QGraphicsRectItem>

class DrawItemMotion : public QGraphicsRectItem
{
    enum MotionType
    {
        TypeDraw,       //已经画的
        TypeClean,      //空白的
        TypeTempDraw,   //要画的
        TypeTempClean   //要清除的
    };

    struct MotionPoint
    {
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

        bool operator< (const MotionPoint &other) const
        {
            if (row != other.row)
            {
                return row < other.row;
            }
            else
            {
                return column < other.column;
            }
        }
    };

public:
    DrawItemMotion(QGraphicsItem *parent = nullptr);

    void clearAll();
    void selectAll();

    void setRegion(char *region);
    void getRegion(char *region);

    void setObjectSize(int value);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void calcMotionMap(const QRect &rect_select);
    void drawMotionMap(QPainter *painter, const QMap<MotionPoint, MotionType> &motionMap);
    void drawObjectSize(QPainter *painter);

private:
    QMap<MotionPoint, MotionType> m_motionMap;
    QMap<MotionPoint, MotionType> m_tempMotionMap;
    QPointF m_pressPos;
    QPointF m_movePos;
    bool m_pressed = false;

    int m_objectSize = -1;
};

#endif // DRAWITEMMOTION_H
