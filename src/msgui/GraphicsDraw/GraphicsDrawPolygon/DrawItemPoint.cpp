#include "DrawItemPoint.h"
#include "DrawItemPolygon.h"
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QtDebug>

DrawItemPoint::DrawItemPoint(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
    , m_parentItem(static_cast<DrawItem_Polygon *>(parent))
{
    setAcceptHoverEvents(true);
    m_pointColor = QColor(10, 169, 227);
}

QPointF DrawItemPoint::centerInParent() const
{
    return mapToParent(rect().center());
}

QPointF DrawItemPoint::centerInScene() const
{
    return mapToScene(rect().center());
}

void DrawItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

#if 0
    if (m_parentItem->isConflict()) {
        painter->setBrush(QColor(255, 0, 0));
    } else {
        painter->setBrush(QColor(10, 169, 227));
    }
#else
    painter->setBrush(m_pointColor);
#endif

    painter->drawEllipse(rect());
}

void DrawItemPoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_parentItem->isFinished()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_isPressed = true;
        m_pressScenePos = event->scenePos();
        m_startPos = pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void DrawItemPoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_parentItem->isFinished()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_isPressed = false;
    }
}

void DrawItemPoint::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_parentItem->isFinished()) {
        return;
    }

    const QRectF &sceneRc = scene()->sceneRect();

    QPointF p = event->scenePos() - m_pressScenePos + m_startPos;
    //把当前矩形转换为scene坐标系，判断边界
    QRectF rc = rect();
    rc.moveTopLeft(m_parentItem->mapToScene(p));
    QPointF c = rc.center();
    if (c.x() < sceneRc.left()) {
        c.setX(sceneRc.left());
    }
    if (c.y() < sceneRc.top()) {
        c.setY(sceneRc.top());
    }
    if (c.x() > sceneRc.right()) {
        c.setX(sceneRc.right());
    }
    if (c.y() > sceneRc.bottom()) {
        c.setY(sceneRc.bottom());
    }
    rc.moveCenter(c);
    //从scene坐标系转换为parent坐标系
    p = m_parentItem->mapFromScene(rc.topLeft());
    rc.moveTopLeft(p);

    const QPointF &oldPos = pos();
    setPos(rc.topLeft());

    if (m_parentItem->checkConflict()) {
        setPos(oldPos);
        m_parentItem->setConflict(false);
    }

    m_parentItem->updateGeometry();
}

void DrawItemPoint::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    if (!m_parentItem->isFinished()) {
        return;
    }
    setCursor(Qt::OpenHandCursor);
}

void DrawItemPoint::setPointColor(const QColor &pointColor)
{
    m_pointColor = pointColor;
}
