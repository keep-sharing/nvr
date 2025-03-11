#include "GraphicsDrawGrid.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

extern "C" {
#include "msdefs.h"
}

GraphicsDrawGrid::GraphicsDrawGrid(QGraphicsItem *parent)
    : MsGraphicsObject(parent)
{
}

void GraphicsDrawGrid::setRegion(char *region)
{
    for (int row = 0; row < MOTION_H_CELL; ++row) {
        for (int column = 0; column < MOTION_W_CELL; ++column) {
            GridPoint point(row, column);
            if (region[row * MOTION_W_CELL + column] == '1') {
                m_motionMap[point] = TypeDraw;
            } else {
                m_motionMap[point] = TypeClean;
            }
        }
    }
    update();
}

void GraphicsDrawGrid::getRegion(char *region)
{
    for (int row = 0; row < MOTION_H_CELL; ++row) {
        for (int column = 0; column < MOTION_W_CELL; ++column) {
            GridPoint point(row, column);
            if (m_motionMap.contains(point)) {
                if (m_motionMap.value(point) == TypeDraw) {
                    region[row * MOTION_W_CELL + column] = '1';
                } else {
                    region[row * MOTION_W_CELL + column] = '0';
                }
            } else {
                region[row * MOTION_W_CELL + column] = '0';
            }
        }
    }
}

void GraphicsDrawGrid::selectAll()
{
    for (int row = 0; row < MOTION_H_CELL; ++row) {
        for (int column = 0; column < MOTION_W_CELL; ++column) {
            GridPoint point(row, column);
            m_motionMap[point] = TypeDraw;
        }
    }
    update();
}

void GraphicsDrawGrid::clearAll()
{
    m_motionMap.clear();
    update();
}

bool GraphicsDrawGrid::hasDrawRegion()
{
    for (auto iter = m_motionMap.constBegin(); iter != m_motionMap.constEnd(); ++iter) {
        if (iter.value() == TypeDraw) {
            return true;
        }
    }
    return false;
}

void GraphicsDrawGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setBrush(Qt::NoBrush);

    drawMotionMap(painter, m_motionMap);
    drawMotionMap(painter, m_tempMotionMap);
}

void GraphicsDrawGrid::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressPos = event->pos();
        m_tempMotionMap.clear();
    } else {
        if (!isEnabled()) {
            MsGraphicsObject::mousePressEvent(event);
        }
    }
}

void GraphicsDrawGrid::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_pressed) {
        qreal lineWidth = sceneWidth() / MOTION_W_CELL;
        qreal lineHeight = sceneHeight() / MOTION_H_CELL;

        QRect selectedRect(m_pressPos.toPoint(), event->pos().toPoint());
        for (int row = 0; row < MOTION_H_CELL; ++row) {
            for (int column = 0; column < MOTION_W_CELL; ++column) {
                QRect rc(column * lineWidth, row * lineHeight, lineWidth, lineHeight);
                GridPoint point(row, column);
                if (selectedRect.intersects(rc)) {
                    if (m_motionMap.contains(point)) {
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
                    } else {
                        m_tempMotionMap[point] = TypeTempDraw;
                    }
                } else {
                    m_tempMotionMap.remove(point);
                }
            }
        }
        update();
    }
}

void GraphicsDrawGrid::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = false;

        QMap<GridPoint, GridType>::iterator iter = m_tempMotionMap.begin();
        for (; iter != m_tempMotionMap.end(); ++iter) {
            const GridPoint &point = iter.key();
            const GridType &type = iter.value();

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
    } else {
        MsGraphicsObject::mouseReleaseEvent(event);
    }
}

void GraphicsDrawGrid::drawMotionMap(QPainter *painter, const QMap<GridPoint, GridType> &gridMap)
{
    qreal lineWidth = (sceneWidth() - 1) / MOTION_W_CELL;
    qreal lineHeight =(sceneHeight() - 1) / MOTION_H_CELL;

    QMap<GridPoint, GridType>::const_iterator iter = gridMap.constBegin();
    for (; iter != gridMap.constEnd(); ++iter) {
        const GridPoint &point = iter.key();
        const GridType &type = iter.value();

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

        qreal nPosY = point.row() * lineHeight;
        qreal nPosX = point.column() * lineWidth;
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
