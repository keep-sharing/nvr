#include "DrawVideoGrid.h"
#include "DrawScenePolygon.h"
#include <QPainter>

extern "C" {
#include "msdefs.h"
}

DrawVideoGrid::DrawVideoGrid(QGraphicsItem *parent)
    : DrawVideoBase(parent)
{
}

void DrawVideoGrid::setRegion(char *region, const QString &xList, const QString &yList)
{
    if (xList.isEmpty() || yList.isEmpty()) {
        m_regionType = RegionGrid;
        for (int row = 0; row < MOTION_H_CELL; ++row) {
            for (int column = 0; column < MOTION_W_CELL; ++column) {
                MotionPoint point(row, column);
                if (region[row * MOTION_W_CELL + column] == '1') {
                    m_motionMap[point] = TypeDraw;
                } else {
                    m_motionMap[point] = TypeClean;
                }
            }
        }
        update();
    } else {
        setPolygon(xList, yList);
    }
}

void DrawVideoGrid::setPolygon(const QString &xList, const QString &yList)
{
    m_regionType = RegionPolygon;

    m_xList = xList;
    m_yList = yList;

    updatePolygon();
    update();
}

void DrawVideoGrid::setRect(const QRectF &rect)
{
    DrawVideoBase::setRect(rect);

    updatePolygon();
}

void DrawVideoGrid::setRect(qreal x, qreal y, qreal w, qreal h)
{
    setRect(QRectF(x, y, w, h));
}

void DrawVideoGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();

    switch (m_regionType) {
    case RegionGrid: {
        qreal lineWidth = (qreal)(rect().width() - 1) / MOTION_W_CELL;
        qreal lineHeight = (qreal)(rect().height() - 1) / MOTION_H_CELL;

        QMap<MotionPoint, MotionType>::const_iterator iter = m_motionMap.constBegin();
        for (; iter != m_motionMap.constEnd(); ++iter) {
            const MotionPoint &point = iter.key();
            const MotionType &type = iter.value();

            switch (type) {
            case TypeDraw:
                switch (state()) {
                case StateNormal:
                    painter->setPen(QPen(QColor(29, 154, 255), 1));
                    break;
                case StateAlarm:
                    painter->setPen(QPen(QColor(255, 0, 0), 1));
                    break;
                }
                break;
            default:
                continue;
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
        break;
    }
    case RegionPolygon: {
        painter->setRenderHint(QPainter::Antialiasing);
        switch (state()) {
        case StateNormal:
            painter->setPen(QPen(QColor(29, 154, 255), 2));
            break;
        case StateAlarm:
            painter->setPen(QPen(QColor(255, 0, 0), 2));
            break;
        }
        painter->drawPolygon(m_points);
        break;
    }
    default:
        break;
    }

    painter->restore();
}

QPointF DrawVideoGrid::physicalPoint(const QPoint &p) const
{
    qreal x = p.x() / 1024.0 * scene()->sceneRect().width();
    qreal y = p.y() / 1024.0 * scene()->sceneRect().height();
    return QPointF(x, y);
}

void DrawVideoGrid::updatePolygon()
{
    m_points.clear();

    QStringList xs = m_xList.split(":", QString::SkipEmptyParts);
    QStringList ys = m_yList.split(":", QString::SkipEmptyParts);
    for (int i = 0; i < xs.size(); ++i) {
        int x = xs.at(i).toInt();
        int y = ys.at(i).toInt();
        if (x < 0 || y < 0) {
            break;
        }
        m_points.append(physicalPoint(QPoint(x, y)));
    }
}
