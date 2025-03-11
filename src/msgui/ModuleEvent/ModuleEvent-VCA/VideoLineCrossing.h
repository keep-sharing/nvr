#ifndef VIDEOLINECROSSING_H
#define VIDEOLINECROSSING_H

#include <QMap>
#include <QWidget>

extern "C" {
#include "msg.h"
}

struct CrossLine {
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

class VideoLineCrossing : public QWidget {
    Q_OBJECT

    enum DrawMode {
        ModeNone,
        ModeDraw,
        ModeMove,
        ModeResizeBegin,
        ModeResizeEnd
    };

public:
    explicit VideoLineCrossing(QWidget *parent = 0);
    ~VideoLineCrossing();

    void setEnable(bool enable);
    void clearLine(int index);
    void clearAll();
    void setCurrentLine(int index);
    void setLineDirection(int direction);
    int lineDirection(int index);
    void saveCurrentLine();

    void setLineCrossInfo(ms_linecrossing_info *info);
    void getLineCrossInfo(ms_linecrossing_info *info);

    void setLineCrossInfo(ms_linecrossing_info2 *info);
    void getLineCrossInfo(ms_linecrossing_info2 *info);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPointF lineCenter(const QLineF &line);

    QPoint physicalPos(int x, int y);
    QPoint logicalPos(const QPoint &pos);

    QPoint checkPoint(const QPoint &point);

private:
    int m_currentLineIndex = 0;
    QMap<int, CrossLine> m_crossLineMap;
    CrossLine m_tempCrossLine;

    bool m_isPressed = false;
    QPoint m_pressPoint;
    QPoint m_tempBeginPoint;
    QPoint m_tempEndPoint;

    DrawMode m_currentMode = ModeDraw;
    bool m_enable = false;

    int m_realWidth = 1920;
    int m_realHeight = 1080;

    bool m_isPeopleCount = false;
};

#endif // VIDEOLINECROSSING_H
