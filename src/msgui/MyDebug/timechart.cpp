#include "timechart.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtDebug>

TimeChart::TimeChart(QWidget *parent) :
    QWidget(parent)
{
    setMouseTracking(true);

    m_painterTimer = new QTimer(this);
    connect(m_painterTimer, SIGNAL(timeout()), this, SLOT(onPainterTimer()));
    m_painterTimer->setInterval(5 * 1000);

    m_chartThread = new ChartThread();
    connect(m_chartThread, SIGNAL(resultImage(QImage)), this, SLOT(onResultImage(QImage)));
}

TimeChart::~TimeChart()
{
    m_chartThread->stopThread();
    delete m_chartThread;
}

void TimeChart::appendValue(int index, int value)
{
    m_chartThread->appendValue(index, value);
}

void TimeChart::clear()
{
    m_chartThread->clear();
}

void TimeChart::saveImage()
{
    bool result = m_image.save("/opt/app/bin/memory.png", "PNG");
    qDebug() << "TimeChart::saveImage," << result;
}

void TimeChart::showEvent(QShowEvent *)
{
    m_painterTimer->start();
    onPainterTimer();
}

void TimeChart::hideEvent(QHideEvent *)
{
    m_painterTimer->stop();
}

void TimeChart::resizeEvent(QResizeEvent *event)
{
    m_borderRect = QRect(m_marginLeft, m_marginTop, width() - m_marginLeft - m_marginRight, height() - m_marginTop - m_marginBottom);
    onPainterTimer();

    QWidget::resizeEvent(event);
}

void TimeChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.drawImage(0, 0, m_image);

    //hover line
    if (m_isDrawHoverLine)
    {
        painter.save();
        painter.drawLine(QPoint(m_borderRect.left(), m_hoverLinePoint.y()), QPoint(m_borderRect.right(), m_hoverLinePoint.y()));
        painter.drawLine(QPoint(m_hoverLinePoint.x(), m_borderRect.top()), QPoint(m_hoverLinePoint.x(), m_borderRect.bottom()));
        int value = ((qreal)m_borderRect.bottom() - m_hoverLinePoint.y()) / m_borderRect.height() * (m_yAxisMaxValue - m_yAxisMinValue) + m_yAxisMinValue;
        QString text = QString("%1%").arg(value);
        QFontMetrics fm(painter.font());
        int textWidth = fm.width(text);
        int textHeight = fm.height();
        QRect textRect(m_hoverLinePoint.x() - textWidth - 5, m_hoverLinePoint.y() - textHeight, textWidth, textHeight);
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
            textRect.moveRight(m_hoverLinePoint.x() - 5);
        }
        if (textRect.left() < m_borderRect.left())
        {
            textRect.moveLeft(m_hoverLinePoint.x() + 5);
        }
        painter.drawText(textRect, Qt::AlignCenter, text);
        painter.restore();
    }
}

void TimeChart::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
}

void TimeChart::leaveEvent(QEvent *event)
{
    m_isDrawHoverLine = false;
    update();
    QWidget::leaveEvent(event);
}

void TimeChart::mouseMoveEvent(QMouseEvent *event)
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

void TimeChart::onPainterTimer()
{
    m_chartThread->drawImage(width(), height());
}

void TimeChart::onResultImage(QImage image)
{
    m_image = image;
    update();
}
