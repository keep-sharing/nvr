#ifndef GRAPHICSDRAWLINECROSSINGITEM_H
#define GRAPHICSDRAWLINECROSSINGITEM_H

#include <QGraphicsRectItem>
class GraphicsDrawLineCrossingItem : public QGraphicsRectItem
{
    enum DIRECTION {
        DIRECTION_A_B   = 0,
        DIRECTION_B_A   = 1,
        DIRECTION_AB    = 2
    };

    enum DrawMode {
        ModeNone,
        ModeDraw,
        ModeMove,
        ModeResizeBegin,
        ModeResizeEnd
    };
    struct LineInfo {
        int in;
        int out;
        int sum;
        int capacity;
    };

public:
    explicit GraphicsDrawLineCrossingItem(QGraphicsItem *parent = nullptr);

    int index() const;
    void setIndex(int newIndex);

    QPointF beginPoint() const;
    void setBeginPoint(QPointF newBeginPoint);

    QPointF endPoint() const;
    void setEndPoint(QPointF newEndPoint);

    int direction() const;
    void setDirection(int newDirection);

    bool isComplete() const;

    void clear();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    void showText();
    QRect removeTextRect(QRect rect, QPointF point);

    void setIsShowIndex(bool isShowIndex);

    void setIsShowLineInfo(bool isShowLineInfo);
    void setLineInfo(int in, int out, int sum ,int capacity);
    QString getLineInfoText();
    void setOsdType(int osdType);

    void setAlarmState(int alarm);

protected:
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    //修正超过scene的point
    void correctScenePos(QPointF &p);

private:
    QPointF lineCenter(const QLineF &line);

    QRectF beginRect() const;
    QRectF endRect() const;
    QRectF contentRect() const;

    void drawArrow(QPainter *painter, QPointF p1, QPointF p2);

private:
    int m_index = 0;
    QPointF m_beginPoint;
    QPointF m_endPoint;
    int m_direction = 0;

    bool m_isComplete = false;
    bool m_isShowIndex =  false;
    DrawMode m_currentMode = ModeNone;
    QPointF m_pressPoint;
    QPointF m_tempBeginPoint;
    QPointF m_tempEndPoint;

    QColor m_lineColor = QColor(254, 254, 0);
    QColor m_pointColor = QColor(255, 0, 0);
    QColor m_textColor = QColor(255, 0, 0);
    QColor m_lineInfoColor = QColor(6, 174, 0);

    bool m_isShowLineInfo = false;

    QGraphicsTextItem *m_textIndexItem;
    QGraphicsTextItem *m_textInfoItem;
    int m_osdType = 0;
    LineInfo m_peopleCntInfo;
};

#endif // GRAPHICSDRAWLINECROSSINGITEM_H
