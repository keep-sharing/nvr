#ifndef CHARTTHREAD_H
#define CHARTTHREAD_H

#include <QObject>
#include <QThread>
#include <QRect>
#include <QDateTime>
#include <QColor>
#include <QImage>

struct LineInfo
{
    QVector<quint32> xValueVector;
    QVector<quint32> yValueVector;
    QColor lineColor = Qt::red;
};

class ChartThread : public QObject
{
    Q_OBJECT
public:
    explicit ChartThread();

    void appendValue(int index, int value);
    void drawImage(int w, int h);
    void clear();

    void stopThread();

private:
    void drawLine(QPainter *painter, const LineInfo &info);

signals:
    void sig_appendValue(int index, int value);
    void sig_drawImage(int w, int h);
    void sig_clear();

    void resultImage(QImage image);

private slots:
    void onAppendValue(int index, int value);
    void onDrawImage(int w, int h);
    void onClear();

private:
    QThread m_thread;

    QSize m_size;

    LineInfo m_lineCpu;
    LineInfo m_lineMemory;

    int m_maxPoints = 100000;

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
};

#endif // CHARTTHREAD_H
