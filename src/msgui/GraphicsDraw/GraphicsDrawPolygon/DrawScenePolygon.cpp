#include "DrawScenePolygon.h"
#include "MyDebug.h"
#include "DrawItemPolygon.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>

const qreal BaseWidth = 1024.0;
const qreal BaseHeight = 1024.0;

DrawScenePolygon::DrawScenePolygon(QObject *parent)
    : QGraphicsScene(parent)
{
    connect(this, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(onSceneRectChanged(QRectF)));
}

void DrawScenePolygon::setPolygon(const QString &xList, const QString &yList)
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

void DrawScenePolygon::getPolygon(char *xList, int xSize, char *yList, int ySize)
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
    snprintf(xList, xSize, "%s", xStr.toStdString().c_str());
    snprintf(yList, ySize, "%s", yStr.toStdString().c_str());
}

void DrawScenePolygon::selectAll()
{
    QVector<QPointF> points;

    points.append(physicalPoint(QPoint(0, 0)));
    points.append(physicalPoint(QPoint(BaseWidth, 0)));
    points.append(physicalPoint(QPoint(BaseWidth, BaseHeight)));
    points.append(physicalPoint(QPoint(0, BaseHeight)));

    currentItem()->setPoints(points);
}

void DrawScenePolygon::clearAll()
{
    clearPolygon();
}

void DrawScenePolygon::clearPolygon()
{
    currentItem()->clear();
}

void DrawScenePolygon::showConflict()
{
    emit conflicted();
}

bool DrawScenePolygon::isFinished()
{
    return currentItem()->isFinished() || currentItem()->isEmpty();
}

void DrawScenePolygon::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);
}

void DrawScenePolygon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (itemAt(event->scenePos())) {
        QGraphicsScene::mousePressEvent(event);
    } else {
        event->accept();
    }
}

void DrawScenePolygon::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
}

QPointF DrawScenePolygon::physicalPoint(const QPoint &p) const
{
    qreal x = p.x() / BaseWidth * sceneRect().width();
    qreal y = p.y() / BaseHeight * sceneRect().height();
    return QPointF(x, y);
}

QPoint DrawScenePolygon::logicalPoint(const QPointF &p) const
{
    int x = qRound(p.x() / sceneRect().width() * BaseWidth);
    int y = qRound(p.y() / sceneRect().height() * BaseHeight);
    return QPoint(x, y);
}

DrawItem_Polygon *DrawScenePolygon::currentItem()
{
    if (!m_currentItem) {
        m_currentItem = new DrawItem_Polygon();
        m_currentItem->setSelected(true);
        m_currentItem->setEditable(true);
        addItem(m_currentItem);
    }
    return m_currentItem;
}

void DrawScenePolygon::onSceneRectChanged(const QRectF &rect)
{
    currentItem()->setRect(rect);
}
