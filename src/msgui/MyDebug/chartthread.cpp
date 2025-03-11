#include "chartthread.h"
#include <QPainter>

ChartThread::ChartThread() :
    QObject(nullptr)
{
    moveToThread(&m_thread);
    m_thread.setObjectName("Qt-ChartThread");
    m_thread.start();

    connect(this, SIGNAL(sig_appendValue(int,int)), this, SLOT(onAppendValue(int,int)));
    connect(this, SIGNAL(sig_drawImage(int,int)), this, SLOT(onDrawImage(int,int)));
    connect(this, SIGNAL(sig_clear()), this, SLOT(onClear()));

    m_lineCpu.lineColor = QColor(247,190,129);
    m_lineMemory.lineColor = QColor(55,178,92);
}

void ChartThread::appendValue(int index, int value)
{
    emit sig_appendValue(index, value);
}

void ChartThread::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

void ChartThread::drawImage(int w, int h)
{
    emit sig_drawImage(w, h);
}

void ChartThread::clear()
{
    emit sig_clear();
}

void ChartThread::drawLine(QPainter *painter, const LineInfo &info)
{
    qreal xTickInterval;
    if (info.yValueVector.size() > 10)
    {
        xTickInterval = (qreal)m_borderRect.width() / info.yValueVector.size();
    }
    else
    {
        xTickInterval = (qreal)m_borderRect.width() / 10;
    }

    QVector<QPointF> m_pointVector;
    for (int i = 0; i < info.yValueVector.size(); ++i)
    {
        const int &value = info.yValueVector.at(i);
        QPointF point(m_borderRect.left() + xTickInterval * i, m_borderRect.bottom() - (qreal)value / m_yAxisMaxValue * m_borderRect.height());
        m_pointVector.append(point);
    }
    painter->save();
    painter->setPen(QPen(info.lineColor, 2));
    painter->drawPolyline(m_pointVector);
    painter->restore();
}

void ChartThread::onAppendValue(int index, int value)
{
    if (m_lineCpu.yValueVector.isEmpty())
    {
        m_startDateTime = QDateTime::currentDateTime();
    }
    else
    {
        m_endDateTime = QDateTime::currentDateTime();
    }

    switch (index) {
    case 0:
        m_lineCpu.yValueVector.append(value);
        break;
    case 1:
        m_lineMemory.yValueVector.append(value);
        break;
    }
}

void ChartThread::onDrawImage(int w, int h)
{
    m_borderRect = QRect(m_marginLeft, m_marginTop, w - m_marginLeft - m_marginRight, h - m_marginTop - m_marginBottom);
    m_size.setWidth(w);
    m_size.setHeight(h);
    //
    QImage image(m_size, QImage::Format_ARGB32);
    if (image.isNull())
    {
        return;
    }
    QPainter painter(&image);
    //画背景
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(image.rect());
    painter.restore();

    //表头
    painter.save();
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    QFontMetrics fm(font);
    QRect cpuRc(m_borderRect.left(), 0, m_borderRect.width() / 2 - 40, m_borderRect.top());
    painter.setPen(QPen(m_lineCpu.lineColor, 2));
    painter.drawText(cpuRc, Qt::AlignVCenter | Qt::AlignRight, "CPU");
    painter.drawLine(cpuRc.right() + 5, cpuRc.center().y(), cpuRc.right() + 30, cpuRc.center().y());

    QRect memoryRc(cpuRc.right() + 40, 0, m_borderRect.width() / 2, m_borderRect.top());
    painter.setPen(QPen(m_lineMemory.lineColor, 2));
    painter.drawText(memoryRc, Qt::AlignVCenter | Qt::AlignLeft, "Memory");
    painter.drawLine(memoryRc.left() + fm.width("Memory") + 5, memoryRc.center().y(), memoryRc.left() + fm.width("Memory") + 30, memoryRc.center().y());
    painter.restore();

    //画边框
    painter.save();
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_borderRect);
    painter.restore();

    //x刻度线
    m_xTickInterval = m_borderRect.width() / 10.0;
    painter.save();
    painter.setPen(Qt::DotLine);
    for (qreal x = m_borderRect.left() + m_xTickInterval; x < m_borderRect.right(); x += m_xTickInterval)
    {
        painter.drawLine(QPointF(x, m_borderRect.top()), QPointF(x, m_borderRect.bottom()));
    }
    painter.restore();

    //确定y轴刻度
    int yTickCount = 10;
    //y刻度线
    m_yTickInterval = m_borderRect.height() / 10.0;
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
            QString text = QString("%1%").arg(yTickIndex * 10);
            painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, text);
        }
        yTickIndex++;
    }
    painter.restore();

    //折线
    drawLine(&painter, m_lineCpu);
    drawLine(&painter, m_lineMemory);

    emit resultImage(image);
}

void ChartThread::onClear()
{
    m_lineCpu.yValueVector.clear();
    m_lineMemory.yValueVector.clear();
    //
    drawImage(m_size.width(), m_size.height());
}
