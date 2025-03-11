#include "drawmotion.h"
#include <QMouseEvent>
#include <QPainter>
#include <QMouseEvent>
#include "msdefs.h"

DrawMotion::DrawMotion(QWidget *parent) :
    QWidget(parent)
{

}

void DrawMotion::clearAll()
{
    m_motionMap.clear();
    update();
}

void DrawMotion::selectAll()
{
    for (int row = 0; row < MOTION_H_CELL; ++row)
    {
        for (int column = 0; column < MOTION_W_CELL; ++column)
        {
            MotionPoint point(row, column);
            m_motionMap[point] = TypeDraw;
        }
    }
    update();
}

bool DrawMotion::hasDrawRegion()
{
    for (auto iter = m_motionMap.constBegin(); iter != m_motionMap.constEnd(); ++iter)
    {
        if (iter.value() == TypeDraw)
        {
            return true;
        }
    }
    return false;
}

void DrawMotion::setRegion(char *region)
{
    for (int row = 0; row < MOTION_H_CELL; ++row)
    {
        for (int column = 0; column < MOTION_W_CELL; ++column)
        {
            MotionPoint point(row, column);
            if (region[row * MOTION_W_CELL + column] == '1')
            {
                m_motionMap[point] = TypeDraw;
            }
            else
            {
                m_motionMap[point] = TypeClean;
            }
        }
    }
    update();
}

void DrawMotion::getRegion(char *region)
{
    for (int row = 0; row < MOTION_H_CELL; ++row)
    {
        for (int column = 0; column < MOTION_W_CELL; ++column)
        {
            MotionPoint point(row, column);
            if (m_motionMap.contains(point))
            {
                if (m_motionMap.value(point) == TypeDraw)
                {
                    region[row * MOTION_W_CELL + column] = '1';
                }
                else
                {
                    region[row * MOTION_W_CELL + column] = '0';
                }
            }
            else
            {
                region[row * MOTION_W_CELL + column] = '0';
            }
        }
    }
}

void DrawMotion::setMode(DrawMotion::Mode mode)
{
    m_mode = mode;
}

void DrawMotion::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressPos = event->pos();
        m_tempMotionMap.clear();
        if (m_mode == CoverMode) {
            clearAll();
        }
    } else {
        if (!isEnabled()) {
            QWidget::mousePressEvent(event);
        }
    }
}

void DrawMotion::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = false;

        QMap<MotionPoint, MotionType>::iterator iter = m_tempMotionMap.begin();
        for (; iter != m_tempMotionMap.end(); ++iter)
        {
            const MotionPoint &point = iter.key();
            const MotionType &type = iter.value();

            switch (type) {
            case TypeTempDraw:
                m_motionMap[point] = TypeDraw;
                break;
            case TypeTempClean:
                m_motionMap[point] = TypeClean;
                break;
            default:
                continue;
                break;
            }
        }
        m_tempMotionMap.clear();
        update();

        emit drawFinished();
    }
    else
    {
        QWidget::mouseReleaseEvent(event);
    }
}

void DrawMotion::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed)
    {
        qreal lineWidth = (qreal)width() / MOTION_W_CELL;
        qreal lineHeight = (qreal)height() / MOTION_H_CELL;

        QRect selectedRect(m_pressPos, event->pos());
        for (int row = 0; row < MOTION_H_CELL; ++row)
        {
            for (int column = 0; column < MOTION_W_CELL; ++column)
            {
                QRect rc(column * lineWidth, row * lineHeight, lineWidth, lineHeight);
                MotionPoint point(row, column);
                if (selectedRect.intersects(rc))
                {
                    if (m_motionMap.contains(point))
                    {
                        switch (m_motionMap.value(point)) {
                        case TypeDraw:
                            m_tempMotionMap[point] = TypeTempClean;
                            break;
                        case TypeClean:
                            m_tempMotionMap[point] = TypeTempDraw;
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        m_tempMotionMap[point] = TypeTempDraw;
                    }
                }
                else
                {
                    m_tempMotionMap.remove(point);
                }
            }
        }
        update();
    }
}

void DrawMotion::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setBrush(Qt::NoBrush);

    drawMotionMap(painter, m_motionMap);
    drawMotionMap(painter, m_tempMotionMap);
}

void DrawMotion::drawMotionMap(QPainter &painter, const QMap<DrawMotion::MotionPoint, DrawMotion::MotionType> &motionMap)
{
    qreal lineWidth = (qreal)(width() - 1) / MOTION_W_CELL;
    qreal lineHeight = (qreal)(height() - 1) / MOTION_H_CELL;

    QMap<MotionPoint, MotionType>::const_iterator iter = motionMap.constBegin();
    for (; iter != motionMap.constEnd(); ++iter)
    {
        const MotionPoint &point = iter.key();
        const MotionType &type = iter.value();

        switch (type) {
        case TypeDraw:
            painter.setPen(QPen(QColor("#1D9AFF"), 1, Qt::SolidLine));
            break;
        case TypeTempDraw:
        case TypeTempClean:
            painter.setPen(QPen(QColor("#FFFFFF"), 1, Qt::DashLine));
            break;
        default:
            continue;
            break;
        }

        qreal nPosY = point.row * lineHeight;
        qreal nPosX = point.column * lineWidth;
        QLineF line1(nPosX, nPosY, nPosX + lineWidth, nPosY);
        QLineF line2(nPosX, nPosY, nPosX, nPosY + lineHeight);
        QLineF line3(nPosX + lineWidth, nPosY, nPosX + lineWidth, nPosY + lineHeight);
        QLineF line4(nPosX, nPosY + lineHeight, nPosX + lineWidth, nPosY + lineHeight);
        painter.drawLine(line1);
        painter.drawLine(line2);
        painter.drawLine(line3);
        painter.drawLine(line4);
    }
}
