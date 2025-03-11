#include "networkchart.h"
#include <QPainter>
#include <qmath.h>
#include <QLocale>
#include <QMouseEvent>

NetworkChart::NetworkChart(QWidget *parent) :
    QWidget(parent)
{
    setMouseTracking(true);
}

void NetworkChart::appendValue(int value)
{
    m_valueVector.push_back(value);
    if (m_valueVector.size() > m_maxPoints)
    {
        m_valueVector.pop_front();
    }
    update();
}

void NetworkChart::clear()
{
    m_valueVector.clear();
    update();
}

void NetworkChart::setLineColor(const QColor &color)
{
    m_lineColor = color;
}

QString NetworkChart::valueString(qint64 bytes)
{
    static qint64 kbps = 1024;
    static qint64 mbps = 1024 * kbps;

    if (bytes >= mbps)
        return QString("%1 Mbps").arg(QLocale().toString(qreal(bytes) / mbps, 'f', 2));
    if (bytes >= kbps)
        return QString("%1 Kbps").arg(QLocale().toString(qreal(bytes) / kbps, 'f', 2));
    return QString("%1 bps").arg(QLocale().toString(bytes));
}

void NetworkChart::resizeEvent(QResizeEvent *event)
{
    m_borderRect = QRect(m_marginLeft, m_marginTop, width() - m_marginLeft - m_marginRight, height() - m_marginTop - m_marginBottom);
    m_maxPoints = m_borderRect.width() / m_xTickInterval + 1;
    m_yTickInterval = m_borderRect.height() / 10.0;

    QWidget::resizeEvent(event);
}

void NetworkChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    static qint64 kbps = 1024;
    static qint64 mbps = 1024 * kbps;

    //先找最大值，最小值
    int maxValue = 1 * mbps;
    int minValue = 0;
    if (!m_valueVector.isEmpty())
    {
        maxValue = m_valueVector.first();
        minValue = m_valueVector.first();
    }
    for (int i = 0; i < m_valueVector.size(); ++i)
    {
        const int &value = m_valueVector.at(i);
        if (value > maxValue)
        {
            maxValue = value;
        }
        if (value < minValue)
        {
            minValue = value;
        }
    }
    //
    if (minValue >= maxValue)
    {
        maxValue = minValue + 5 * mbps;
    }

    //确定y轴刻度
    int yTickCount = 10;
    int yAxisMaxValue = maxValue * 1.2;
    int yAxisMinValue = minValue / 1.2;

    //画边框
    painter.save();
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_borderRect);
    painter.restore();

    //x刻度线
    painter.save();
    painter.setPen(Qt::DotLine);
    for (int x = m_borderRect.left() + m_xTickInterval; x < m_borderRect.right(); x += m_xTickInterval)
    {
        painter.drawLine(QPoint(x, m_borderRect.top()), QPoint(x, m_borderRect.bottom()));
    }
    painter.restore();

    //y刻度线
    painter.save();
    for (qreal y = m_borderRect.top() + m_yTickInterval; y < m_borderRect.bottom(); y += m_yTickInterval)
    {
        painter.setPen(Qt::DotLine);
        painter.drawLine(QPointF(m_borderRect.left(), y), QPointF(m_borderRect.right(), y));
    }
    painter.restore();
    //
    painter.save();
    int yTickIndex = 0;
    for (qreal y = m_borderRect.bottom(); yTickIndex <= yTickCount; y -= m_yTickInterval)
    {
        if (yTickIndex % 2 == 0)
        {
            QRect textRect(0, y - m_yTickInterval / 2, m_marginLeft - 5, m_yTickInterval);
            QString text = "0";
            int bytes = yAxisMinValue + (yAxisMaxValue - yAxisMinValue) / yTickCount * yTickIndex;
            text = valueString(bytes);
            painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, text);
        }
        yTickIndex++;
    }
    painter.restore();

    //折线
    QVector<QPoint> m_pointVector;
    for (int i = 0; i < m_valueVector.size(); ++i)
    {
        const int &value = m_valueVector.at(i);
        QPoint point(m_borderRect.left() + m_xTickInterval * i, m_borderRect.bottom() - (qreal)(value - yAxisMinValue) / (yAxisMaxValue - yAxisMinValue) * m_borderRect.height());
        m_pointVector.append(point);
    }
    painter.save();
    painter.setPen(QPen(m_lineColor, 2));
    painter.drawPolyline(m_pointVector);
    painter.restore();

    //hover line
    if (m_isDrawHoverLine)
    {
        painter.save();
        painter.drawLine(QPoint(m_borderRect.left(), m_hoverLinePoint.y()), QPoint(m_borderRect.right(), m_hoverLinePoint.y()));
        int bytes = ((qreal)m_borderRect.bottom() - m_hoverLinePoint.y()) / m_borderRect.height() * (yAxisMaxValue - yAxisMinValue) + yAxisMinValue;
        QString text = valueString(bytes);
        QFontMetrics fm(painter.font());
        int textWidth = fm.width(text);
        int textHeight = fm.height();
        QRect textRect(m_hoverLinePoint.x() - textWidth / 2, m_hoverLinePoint.y() - textHeight, textWidth, textHeight);
        if (textRect.top() < m_borderRect.top())
        {
            textRect.moveTop(m_hoverLinePoint.y());
        }
        if (textRect.bottom() > m_borderRect.bottom())
        {
            textRect.moveBottom(m_hoverLinePoint.y());
        }
        if (textRect.right() > m_borderRect.right())
        {
            textRect.moveRight(m_borderRect.right());
        }
        if (textRect.left() < m_borderRect.left())
        {
            textRect.moveLeft(m_borderRect.left());
        }
        painter.drawText(textRect, Qt::AlignCenter, text);
        painter.restore();
    }
}

void NetworkChart::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
}

void NetworkChart::leaveEvent(QEvent *event)
{
    m_isDrawHoverLine = false;
    update();
    QWidget::leaveEvent(event);
}

void NetworkChart::mouseMoveEvent(QMouseEvent *event)
{
    if (m_borderRect.contains(event->pos()))
    {
        setCursor(Qt::CrossCursor);
        m_isDrawHoverLine = true;
        m_hoverLinePoint = event->pos();
    }
    else
    {
        unsetCursor();
        m_isDrawHoverLine = false;
    }
    update();
    QWidget::mouseMoveEvent(event);
}
