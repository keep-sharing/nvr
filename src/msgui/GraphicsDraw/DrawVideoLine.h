#ifndef DRAWVIDEOLINE_H
#define DRAWVIDEOLINE_H

#include "DrawVideoBase.h"

extern "C"
{
#include "msg.h"
}

class DrawVideoLine : public DrawVideoBase
{
    enum LineType
    {
        TypeNone,
        TypeLineCross,
        TypeLineCross2,
        TypePeopleCount
    };

    struct CrossLine
    {
        int index = 0;
        QPoint beginPoint;
        QPoint endPoint;
        bool beginComplete = false;
        bool endComplete = false;
        int direction = 0;
        bool showIndex = true;

        void clear()
        {
            beginPoint = QPoint();
            endPoint = QPoint();
            beginComplete = false;
            endComplete = false;
        }

        QRect beginRect() const
        {
            return QRect(beginPoint.x() - 4, beginPoint.y() - 4, 8, 8);
        }
        QRect endRect() const
        {
            return QRect(endPoint.x() - 4, endPoint.y() - 4, 8, 8);
        }
        QRect boundingRect() const
        {
            return QRect(beginPoint, endPoint);
        }
    };

public:
    DrawVideoLine(QGraphicsItem *parent = nullptr);
    ~DrawVideoLine();

    void setLineCrossInfo(ms_linecrossing_info *info);
    void setLineCrossInfo2(ms_linecrossing_info2 *info);

    void setLineCrossState(int state, int line);
    void setPeopleCountState(int state);
    void clearState() override;

    void setRect(const QRectF &rect);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    void drawLineCrossInfo(ms_linecrossing_info *info);
    void drawLineCrossInfo2(ms_linecrossing_info2 *info);
    void drawLineCrossInfo2(ms_linecrossing_info2 *info, int index);
    void drawLine(QPainter *painter, const CrossLine &line, bool alarm);

    QPointF lineCenter(const QLineF &line);

    QPoint physicalPos(int x, int y);
    QPoint logicalPos(const QPoint &pos);

private:
    LineType m_lineType = TypeNone;

    ms_linecrossing_info *m_linecrossing_info = nullptr;
    ms_linecrossing_info2 *m_linecrossing_info2 = nullptr;
    ms_smart_event_people_cnt *m_smart_event_people_cnt = nullptr;

    QMap<int, CrossLine> m_crossLineMap;

    int m_realWidth = 1920;
    int m_realHeight = 1080;
    int m_lineStatus[4] = {0};
};

#endif // DRAWVIDEOLINE_H
