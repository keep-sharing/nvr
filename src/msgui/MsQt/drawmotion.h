#ifndef DRAWMOTION_H
#define DRAWMOTION_H

#include <QWidget>
#include <QMap>

class DrawMotion : public QWidget
{
    Q_OBJECT

public:
    enum Mode
    {
        MixedMode,
        CoverMode
    };

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

    explicit DrawMotion(QWidget *parent = 0);

    void clearAll();
    void selectAll();

    bool hasDrawRegion();
    void setRegion(char *region);
    void getRegion(char *region);

    void setMode(DrawMotion::Mode mode);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *) override;

private:
    void calcMotionMap(const QRect &rect_select);
    void drawMotionMap(QPainter &painter, const QMap<MotionPoint, MotionType> &motionMap);

signals:
    void drawFinished();

public slots:

private:
    Mode m_mode = MixedMode;
    QMap<MotionPoint, MotionType> m_motionMap;
    QMap<MotionPoint, MotionType> m_tempMotionMap;
    QPoint m_pressPos;
    QPoint m_movePos;
    bool m_pressed = false;
};

#endif // DRAWMOTION_H
