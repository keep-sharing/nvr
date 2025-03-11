#ifndef GUSTURELOCK_H
#define GUSTURELOCK_H

#include <QWidget>
#include <QMap>

typedef enum
{
    WhiteStyle, //白色主题
    GrayStyle   //灰色主题
}Style;

class GustureLock : public QWidget
{
    Q_OBJECT

    enum PointState
    {
        StateNone,
        StateSelected
    };

    struct PointInfo
    {
        //1-9
        int value = 0;
        QRect outerRect;
        QRect innerRect;
        PointState state = StateNone;

        bool isValid() const
        {
            return value > 0;
        }
    };

public:

    explicit GustureLock(QWidget *parent = nullptr);

    void setEnabled(bool enable);
    void setStyle(Style style);

protected:
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void paintEvent(QPaintEvent *) override;

private:
    void clearSelected();
    QString selectedText();
    PointInfo infoUnderMouse(const QPoint &p);
    void drawEnd();

signals:
    void drawFinished(const QString &text);
    void drawStart();

private:
    QList<PointInfo> m_listPoint;
    QMap<PointState, QColor> m_mapColor;
    int m_lineWidth = 2;

    bool m_isPressed = false;
    QPoint m_movePoint;
    QPoint m_lastSelectedPoint;
    QList<int> m_listSelected;
    QVector<QPoint> m_polylinePoints;
};

#endif // GUSTURELOCK_H
