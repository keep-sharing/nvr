#include "DrawControlPolygon.h"
#include "DrawItemPolygon.h"
#include "GraphicsScene.h"
#include "MyDebug.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>

const qreal BaseWidth = 1024.0;
const qreal BaseHeight = 1024.0;

DrawControlPolygon::DrawControlPolygon(QGraphicsItem *parent)
    : MsGraphicsObject(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setFiltersChildEvents(true);
}

void DrawControlPolygon::setPolygon(const QString &xList, const QString &yList)
{
    QVector<QPointF> points;

    QStringList xs = xList.split(":", QString::SkipEmptyParts);
    QStringList ys = yList.split(":", QString::SkipEmptyParts);
    for (int i = 0; i < xs.size(); ++i) {
        int x = xs.at(i).toInt();
        int y = ys.at(i).toInt();
        if (x < 0 || y < 0) {
            break;
        }
        points.append(physicalPoint(QPoint(x, y)));
    }
    currentItem()->setPoints(points);
}

void DrawControlPolygon::getPolygon(char *xList, int xSize, char *yList, int ySize)
{
    QString xStr;
    QString yStr;
    const QVector<QPointF> &points = currentItem()->scenePoints();
    for (int i = 0; i < 10; ++i) {
        if (i < points.size()) {
            const QPointF &p = points.at(i);
            const QPoint &lp = logicalPoint(p);
            xStr.append(QString("%1:").arg(lp.x()));
            yStr.append(QString("%1:").arg(lp.y()));
        } else {
            xStr.append("-1:");
            yStr.append("-1:");
        }
    }
    snprintf(xList, static_cast<size_t>(xSize), "%s", xStr.toStdString().c_str());
    snprintf(yList, static_cast<size_t>(ySize), "%s", yStr.toStdString().c_str());
}

void DrawControlPolygon::selectAll()
{
    QVector<QPointF> points;

    points.append(physicalPoint(QPoint(0, 0)));
    points.append(physicalPoint(QPoint(static_cast<int>(BaseWidth), 0)));
    points.append(physicalPoint(QPoint(static_cast<int>(BaseWidth), static_cast<int>(BaseHeight))));
    points.append(physicalPoint(QPoint(0, static_cast<int>(BaseHeight))));

    currentItem()->setPoints(points);
}

void DrawControlPolygon::clearAll()
{
    clearPolygon();
}

void DrawControlPolygon::clearPolygon()
{
    currentItem()->clear();
}

void DrawControlPolygon::showConflict()
{
    emit conflicted();
}

bool DrawControlPolygon::isFinished()
{
    return currentItem()->isFinished() || currentItem()->isEmpty();
}

void DrawControlPolygon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    bool hasRegion = false;
    QList<QGraphicsItem *> temp = scene()->items(event->scenePos());
    for (auto *i : temp) {
        if (i->cursor().shape() != Qt::ArrowCursor) {
            hasRegion = true;
            break;
        }
    }

    if (hasRegion) {
        MsGraphicsObject::mousePressEvent(event);
    } else {
        event->accept();
    }
}

void DrawControlPolygon::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    MsGraphicsObject::mouseMoveEvent(event);
}

QVariant DrawControlPolygon::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemEnabledChange: {
        for (auto item : childItems()) {
            item->setCursor(Qt::ArrowCursor);
        }
        break;
    }
    case ItemVisibleHasChanged: {
        for (auto item : childItems()) {
            item->setVisible(isVisible());
        }
        break;
    }
    default:
        break;
    }
    return QGraphicsItem::itemChange(change, value);
}

QPointF DrawControlPolygon::physicalPoint(const QPoint &p) const
{
    qreal x = p.x() / BaseWidth * sceneRect().width();
    qreal y = p.y() / BaseHeight * sceneRect().height();
    return QPointF(x, y);
}

QPoint DrawControlPolygon::logicalPoint(const QPointF &p) const
{
    int x = qRound(p.x() / sceneRect().width() * BaseWidth);
    int y = qRound(p.y() / sceneRect().height() * BaseHeight);
    return QPoint(x, y);
}

DrawItem_Polygon *DrawControlPolygon::currentItem()
{
    if (!m_currentItem) {
        m_currentItem = new DrawItem_Polygon();
        m_currentItem->setSelected(true);
        m_currentItem->setEditable(true);
        m_currentItem->setParentItem(this);
    }
    return m_currentItem;
}

