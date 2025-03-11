#ifndef NETWORKCHART_H
#define NETWORKCHART_H

#include <QWidget>

class NetworkChart : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkChart(QWidget *parent = 0);

    void appendValue(int value);
    void clear();

    void setLineColor(const QColor &color);

    static QString valueString(qint64 bytes);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:

signals:

public slots:

private:
    QVector<int> m_valueVector;
    int m_maxPoints = 0;

    QRect m_borderRect;

    int m_marginLeft = 110;
    int m_marginTop = 10;
    int m_marginRight = 5;
    int m_marginBottom = 10;

    int m_xTickInterval = 15;    //x轴间距
    qreal m_yTickInterval = 15;    //y轴间距

    QColor m_lineColor = Qt::red;

    //hover line
    bool m_isDrawHoverLine = false;
    QPoint m_hoverLinePoint;
};

#endif // NETWORKCHART_H
