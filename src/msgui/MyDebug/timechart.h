#ifndef TIMECHART_H
#define TIMECHART_H

#include <QWidget>
#include <QDateTime>
#include <QTimer>
#include "chartthread.h"

class TimeChart : public QWidget
{
    Q_OBJECT
public:
    explicit TimeChart(QWidget *parent = nullptr);
    ~TimeChart();

    void appendValue(int index, int value);
    void clear();
    void saveImage();

protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:

signals:

private slots:
    void onPainterTimer();
    void onResultImage(QImage image);

private:
    ChartThread *m_chartThread = nullptr;
    QImage m_image;
    QTimer *m_painterTimer = nullptr;

    QRect m_borderRect;
    int m_marginLeft = 50;
    int m_marginTop = 30;
    int m_marginRight = 15;
    int m_marginBottom = 30;

    QDateTime m_startDateTime;
    QDateTime m_endDateTime;

    int m_yAxisMaxValue = 100;
    int m_yAxisMinValue = 0;
    qreal m_xTickInterval;    //x轴间距
    qreal m_yTickInterval;    //y轴间距

    //hover line
    bool m_isDrawHoverLine = false;
    QPoint m_hoverLinePoint;
};

#endif // TIMECHART_H
