#include "DrawItemPolygon.h"
#include "DrawControlPolygon.h"
#include "DrawItemPoint.h"
#include "DrawScenePolygon.h"
#include "MyDebug.h"
#include <QCursor>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

//吸附直径
const int AbsorbDiameter = 20;

DrawItem_Polygon::DrawItem_Polygon(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setAcceptHoverEvents(true);
    setFiltersChildEvents(true);

    m_cursorPen = QCursor(QPixmap(":/playback/playback/pen.png"), 8, 6);
    m_maxPointSize = 10;
    m_minPointSize = 3;
}

bool DrawItem_Polygon::isLineCross(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4)
{
    if (p1 == p3 || p1 == p4) {
        return false;
    }
    if (p3 == p1 || p3 == p2) {
        return false;
    }
    QPointF p;
    QLineF line1(p1, p2);
    QLineF line2(p3, p4);
    QLineF::IntersectType type = line1.intersect(line2, &p);
    switch (type) {
    case QLineF::NoIntersection:
        return false;
    case QLineF::BoundedIntersection:
        return true;
    case QLineF::UnboundedIntersection:
        return false;
    default:
        return false;
    }
}

bool DrawItem_Polygon::isLineContainsPoint(const QLineF &line, const QPointF &p)
{
    QLineF ac(line.p1(), p);
    QLineF bc(line.p2(), p);
    //ac + bc = ab, 则c在ab上
    if (qFuzzyCompare(ac.length() + bc.length(), line.length())) {
        if (ac.length() != 0 && bc.length() != 0) {
            return true;
        }
    }
    return false;
}

void DrawItem_Polygon::setPoints(const QVector<QPointF> &pointVect)
{
    m_isConflict = false;
    m_isFinished = false;
    clearPointItems();
    setRect(scene()->sceneRect());
    setPos(0, 0);

    if (!pointVect.isEmpty()) {
        for (int i = 0; i < pointVect.size(); ++i) {
            const QPointF &p = pointVect.at(i);
            addPointItem(mapFromScene(p));
        }
        m_isFinished = true;
    }
    updateGeometry();
    scene()->update(scene()->sceneRect());
}

QVector<QPointF> DrawItem_Polygon::points() const
{
    QVector<QPointF> vecPoint;
    for (int i = 0; i < m_pointItems.size(); ++i) {
        DrawItemPoint *item = m_pointItems.at(i);
        const QPointF &p = item->centerInParent();
        vecPoint.append(p);
    }
    return vecPoint;
}

QVector<QPointF> DrawItem_Polygon::scenePoints() const
{
    QVector<QPointF> vecPoint;
    for (int i = 0; i < m_pointItems.size(); ++i) {
        DrawItemPoint *item = m_pointItems.at(i);
        const QPointF &p = item->centerInScene();
        vecPoint.append(p);
    }
    return vecPoint;
}

void DrawItem_Polygon::clear()
{
    m_isFinished = false;
    m_isConflict = false;
    clearPointItems();
    clearText();
    unsetCursor();
    setRect(scene()->sceneRect());
    setPos(0, 0);
    updateGeometry();
    scene()->update(scene()->sceneRect());
}

bool DrawItem_Polygon::isConflict() const
{
    return m_isConflict;
}

bool DrawItem_Polygon::isFinished() const
{
    return m_isFinished;
}

bool DrawItem_Polygon::isEmpty() const
{
    return m_pointItems.isEmpty();
}

void DrawItem_Polygon::setConflict(bool value)
{
    m_isConflict = value;
}

bool DrawItem_Polygon::checkConflict()
{
    m_isConflict = false;

    for (int i = 0; i < m_pointItems.size(); ++i) {
        QPointF p1;
        QPointF p2;
        if (i == m_pointItems.size() - 1) {
            p1 = m_pointItems.at(i)->centerInParent();
            p2 = m_pointItems.at(0)->centerInParent();
        } else {
            p1 = m_pointItems.at(i)->centerInParent();
            p2 = m_pointItems.at(i + 1)->centerInParent();
        }

        for (int j = 0; j < m_pointItems.size() - 1; ++j) {
            QPointF p3;
            QPointF p4;
            if (j == m_pointItems.size() - 1) {
                p3 = m_pointItems.at(j)->centerInParent();
                p4 = m_pointItems.at(0)->centerInParent();
            } else {
                p3 = m_pointItems.at(j)->centerInParent();
                p4 = m_pointItems.at(j + 1)->centerInParent();
            }

            QLineF line1(p1, p2);
            QLineF line2(p3, p4);
            if (line1 == line2) {
                continue;
            } else {
                if (isLineCross(p1, p2, p3, p4)) {
                    m_isConflict = true;
                    return true;
                }
            }
        }
    }
    return false;
}

void DrawItem_Polygon::updateGeometry()
{
    prepareGeometryChange();
    showText();
}

void DrawItem_Polygon::setSelected(bool selected)
{
    m_isSelected = selected;
}

bool DrawItem_Polygon::isSelected() const
{
    return m_isSelected;
}

void DrawItem_Polygon::setEditable(bool enable)
{
    for (int i = 0; i < m_pointItems.size(); ++i) {
        DrawItemPoint *item = m_pointItems.at(i);
        item->setVisible(enable);
    }
    m_isEditable = enable;
    update();
}

bool DrawItem_Polygon::isEditable() const
{
    return m_isEditable;
}

void DrawItem_Polygon::setPointSizeRange(int max, int min)
{
    m_maxPointSize = max;
    m_minPointSize = min;
    if (m_minPointSize < 3) {
        m_minPointSize = 3;
    }
}

void DrawItem_Polygon::beginEdit()
{
    m_isEditting = true;
}

void DrawItem_Polygon::endEdit()
{
    m_isEditting = false;
}

bool DrawItem_Polygon::isEditting() const
{
    return m_isEditting;
}

void DrawItem_Polygon::setAlarmable(bool enable)
{
    m_isAlarmable = enable;
}

bool DrawItem_Polygon::isAlarmable() const
{
    return m_isAlarmable;
}

DrawScenePolygon *DrawItem_Polygon::scene() const
{
    return static_cast<DrawScenePolygon *>(QGraphicsRectItem::scene());
}

QRectF DrawItem_Polygon::boundingRect() const
{
    if (isFinished()) {
        return shape().boundingRect();
    } else {
        return rect();
    }
}

QPainterPath DrawItem_Polygon::shape() const
{
    QPainterPath path;
    if (isFinished()) {
        path.addPolygon(points());
    } else {
        path.addRect(boundingRect());
    }
    return path;
}

void DrawItem_Polygon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHints(QPainter::Antialiasing);

#if 0
    painter->setPen(Qt::red);
    painter->drawRect(boundingRect());
#endif

    const QVector<QPointF> &vecPoint = points();
    if (vecPoint.isEmpty()) {
        return;
    }

#if 0
    if (isConflict()) {
        painter->setPen(QPen(QColor(255, 0, 0), 2));
    } else {
        painter->setPen(QPen(QColor(10, 169, 227), 2));
    }
#else
    QPen pen;
    pen.setColor(lineColor());
    if (isSelected() && isEditable()) {
        pen.setWidth(3);
    } else {
        pen.setWidth(2);
    }
    painter->setPen(pen);
#endif

    if (isFinished()) {
        painter->drawPolygon(vecPoint);
    } else {
        painter->drawPolyline(vecPoint);
        painter->drawLine(vecPoint.last(), m_hoverPos);
    }
}

bool DrawItem_Polygon::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (!isSelected() || !isEditable()) {
        return QGraphicsRectItem::sceneEventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::GraphicsSceneHoverMove: {
        if (!isFinished()) {
            QGraphicsSceneHoverEvent *hoverEvent = static_cast<QGraphicsSceneHoverEvent *>(event);
            hoverMoveEvent(hoverEvent);
        }
        break;
    }
    case QEvent::GraphicsSceneMousePress: {
        if (!isFinished()) {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
            mousePressEvent(mouseEvent);
            return true;
        }
        break;
    }
    default:
        break;
    }

    return QGraphicsRectItem::sceneEventFilter(watched, event);
}

void DrawItem_Polygon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isSelected() || !isEditable()) {
        return;
    }

    const QPointF &p = event->scenePos();

    switch (event->button()) {
    case Qt::LeftButton: {
        m_isLeftPressed = true;
        if (m_isConflict) {
            return;
        }
        if (m_isFinished) {
            if (m_isUnderMouse) {
                m_pressScenePos = p;
                m_startPos = pos();

                m_startRc = boundingRect();
                m_startRc.moveTopLeft(mapToScene(m_startRc.topLeft()));
            }
            return;
        }

        //是否在第一个点附近
        if (!m_pointItems.isEmpty()) {
            const QPointF &firstPos = m_pointItems.first()->centerInScene();
            QRectF rc(0, 0, AbsorbDiameter, AbsorbDiameter);
            rc.moveCenter(firstPos);
            if (rc.contains(m_hoverPos)) {
                m_hoverPos = firstPos;
                if (m_pointItems.size() < m_minPointSize) {
                    return;
                }
                m_isFinished = true;
                updateGeometry();
            }
        }

        if (!m_isFinished) {
            m_hoverPos = p;
            addPointItem(mapFromScene(p));
            if (m_pointItems.size() >= m_maxPointSize) {
                updateGeometry();
                if (checkConflict()) {
                    m_isFinished = false;
                    DrawControlPolygon *parent = qgraphicsitem_cast<DrawControlPolygon *>(parentItem());
                    parent->showConflict();
                } else {
                    m_isFinished = true;
                }
            }
        }

        showText();
        scene()->update(scene()->sceneRect());
        break;
    }
    case Qt::RightButton: {
        if (m_pointItems.size() > m_minPointSize - 1) {
            const QPointF &p3 = m_pointItems.last()->centerInParent();
            const QPointF &p4 = m_pointItems.first()->centerInParent();

            m_isConflict = false;
            for (int i = 0; i < m_pointItems.size() - 1; ++i) {
                const QPointF &p1 = m_pointItems.at(i)->centerInParent();
                const QPointF &p2 = m_pointItems.at(i + 1)->centerInParent();
                if (isLineCross(p1, p2, p3, p4)) {
                    m_isConflict = true;
                    break;
                }
            }

            if (m_isConflict) {
                m_isFinished = false;
                DrawControlPolygon *parent = qgraphicsitem_cast<DrawControlPolygon *>(parentItem());
                parent->showConflict();
            } else {
                m_isFinished = true;
            }

            showText();
            scene()->update(scene()->sceneRect());
        } else if (m_pointItems.size() == 2) {
            QRectF rc(m_pointItems.first()->centerInParent(), m_pointItems.last()->centerInParent());
            QRectF normalRc = rc.normalized();
            if (normalRc.width() < 2 || normalRc.height() < 2) {
                return;
            }
            clearPointItems();
            addPointItem(rc.topLeft());
            addPointItem(rc.topRight());
            addPointItem(rc.bottomRight());
            addPointItem(rc.bottomLeft());

            m_isFinished = true;
            showText();
            scene()->update(scene()->sceneRect());
        }
        break;
    }
    default:
        break;
    }
}

void DrawItem_Polygon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    if (!isSelected() || !isEditable()) {
        return;
    }

    m_isLeftPressed = false;
}

void DrawItem_Polygon::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isSelected() || !isEditable()) {
        return;
    }

    if (!m_isLeftPressed) {
        return;
    }
    if (m_isFinished) {
        if (m_isUnderMouse) {
            const QRectF &sceneRc = scene()->sceneRect();
            QRectF rc = m_startRc.translated(event->scenePos() - m_pressScenePos);

            if (rc.left() < sceneRc.left()) {
                rc.moveLeft(sceneRc.left());
            }
            if (rc.top() < sceneRc.top()) {
                rc.moveTop(sceneRc.top());
            }
            if (rc.right() > sceneRc.right()) {
                rc.moveRight(sceneRc.right());
            }
            if (rc.bottom() > sceneRc.bottom()) {
                rc.moveBottom(sceneRc.bottom());
            }
            setPos(rc.topLeft() - m_startRc.topLeft() + m_startPos);
            showText();
        }
    }
}

void DrawItem_Polygon::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!isSelected() || !isEditable()) {
        setCursor(Qt::ArrowCursor);
        return;
    }

    m_isUnderMouse = false;

    if (m_isFinished) {
        setCursor(Qt::SizeAllCursor);
        m_isUnderMouse = true;
        return;
    } else {
        setCursor(m_cursorPen);
    }
    if (m_pointItems.isEmpty()) {
        return;
    }

    m_hoverPos = event->scenePos();

    //吸附
    const QPointF &firstPos = m_pointItems.first()->centerInScene();
    QRectF rc(0, 0, AbsorbDiameter, AbsorbDiameter);
    rc.moveCenter(firstPos);
    if (rc.contains(m_hoverPos)) {
        m_hoverPos = firstPos;
    }

    const QPointF &p3 = m_pointItems.last()->centerInParent();
    const QPointF &p4 = m_hoverPos;

    m_isConflict = false;
    for (int i = 0; i < m_pointItems.size() - 1; ++i) {
        const QPointF &p1 = m_pointItems.at(i)->centerInParent();
        const QPointF &p2 = m_pointItems.at(i + 1)->centerInParent();
        if (isLineCross(p1, p2, p3, p4)) {
            m_isConflict = true;
            break;
        }
    }

    scene()->update(scene()->sceneRect());
}

QColor DrawItem_Polygon::lineColor()
{
    return QColor(10, 169, 227);
}

void DrawItem_Polygon::showText()
{
}

void DrawItem_Polygon::clearText()
{
}

void DrawItem_Polygon::setIndex(int newIndex)
{
    m_index = newIndex;
}

int DrawItem_Polygon::index() const
{
    return m_index;
}

void DrawItem_Polygon::clearPointItems()
{
    for (int i = 0; i < m_pointItems.size(); ++i) {
        DrawItemPoint *item = m_pointItems.at(i);
        delete item;
    }
    m_pointItems.clear();
}

void DrawItem_Polygon::addPointItem(const QPointF &p)
{
    DrawItemPoint *item = new DrawItemPoint(this);
    QRectF rc(0, 0, 8, 8);
    item->setRect(rc);
    rc.moveCenter(p);
    item->setPos(rc.topLeft());
    m_pointItems.append(item);
}
