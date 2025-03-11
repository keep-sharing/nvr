#include "DrawItemMotion.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "msdefs.h"

DrawItemMotion::DrawItemMotion(QGraphicsItem *parent) :
    QGraphicsRectItem(parent)
{

}

void DrawItemMotion::clearAll()
{
    m_motionMap.clear();
    update();
}

void DrawItemMotion::selectAll()
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

void DrawItemMotion::setRegion(char *region)
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

void DrawItemMotion::getRegion(char *region)
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

void DrawItemMotion::setObjectSize(int value)
{
    m_objectSize = value;
    update();
}

void DrawItemMotion::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setBrush(Qt::NoBrush);

    drawMotionMap(painter, m_motionMap);
    drawMotionMap(painter, m_tempMotionMap);
    drawObjectSize(painter);

    painter->restore();
}

void DrawItemMotion::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsRectItem::hoverMoveEvent(event);
}

void DrawItemMotion::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressed = true;
    m_pressPos = event->pos();
    m_tempMotionMap.clear();
}

void DrawItemMotion::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

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
}

void DrawItemMotion::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_pressed)
    {
        qreal lineWidth = (qreal)rect().width() / MOTION_W_CELL;
        qreal lineHeight = (qreal)rect().height() / MOTION_H_CELL;

        QRectF selectedRect(m_pressPos, event->pos());
        for (int row = 0; row < MOTION_H_CELL; ++row)
        {
            for (int column = 0; column < MOTION_W_CELL; ++column)
            {
                QRectF rc(column * lineWidth, row * lineHeight, lineWidth, lineHeight);
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

void DrawItemMotion::calcMotionMap(const QRect &rect_select)
{
    Q_UNUSED(rect_select)
}

void DrawItemMotion::drawMotionMap(QPainter *painter, const QMap<DrawItemMotion::MotionPoint, DrawItemMotion::MotionType> &motionMap)
{
    qreal lineWidth = (qreal)(rect().width() - 1) / MOTION_W_CELL;
    qreal lineHeight = (qreal)(rect().height() - 1) / MOTION_H_CELL;

    QMap<MotionPoint, MotionType>::const_iterator iter = motionMap.constBegin();
    for (; iter != motionMap.constEnd(); ++iter)
    {
        const MotionPoint &point = iter.key();
        const MotionType &type = iter.value();

        switch (type) {
        case TypeDraw:
            painter->setPen(QPen(QColor("#1D9AFF"), 1, Qt::SolidLine));
            break;
        case TypeTempDraw:
        case TypeTempClean:
            painter->setPen(QPen(QColor("#FFFFFF"), 1, Qt::DashLine));
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
        painter->drawLine(line1);
        painter->drawLine(line2);
        painter->drawLine(line3);
        painter->drawLine(line4);
    }
}

void DrawItemMotion::drawObjectSize(QPainter *painter)
{
    if (m_objectSize > 0)
    {
        painter->save();

        QRectF objectRect = rect();
        objectRect.setWidth(rect().width() / 2 / 100 * m_objectSize);
        objectRect.setHeight(rect().height() / 2 / 100 *m_objectSize);
        objectRect.moveCenter(rect().center());

        painter->setPen(QPen(QColor("#FF0000")));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(objectRect);

        painter->drawText(objectRect.left(), objectRect.top() - 2, "Min. Object Size");

        painter->restore();
    }
}
