#include "DrawItemFacePolygon.h"
#include "DrawItemAnprControl.h"
#include "DrawItemFaceCapture.h"
#include "DrawItemPoint.h"
#include "DrawScenePolygon.h"
#include "MyDebug.h"
#include <QCursor>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

//吸附直径
const int AbsorbDiameter = 20;
DrawItemFacePolygon::DrawItemFacePolygon(QGraphicsItem *parent)
    : DrawItem_Polygon(parent)
{
    m_textItem = new QGraphicsTextItem(this);
}

void DrawItemFacePolygon::setIsFinished(bool value)
{
    m_isFinished = value;
}

int DrawItemFacePolygon::type() const
{
    return Type;
}

void DrawItemFacePolygon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton && (m_isFinished || isEmpty())) {
        return;
    }
    MsGraphicsObject *parent;
    if (parentItem()->type() == UserType + 4) {
        parent = qgraphicsitem_cast<DrawItemAnprControl *>(parentItem());
    } else {
        parent = qgraphicsitem_cast<DrawItemFaceCapture *>(parentItem());
    }
    parent->setItemsSeleteFalse();
    if (!isEditable()) {
        parent->setCurrentItem(this);
    }
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
        if (m_pointItems.size() > m_minPointSize - 1 && !m_isFinished) {
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
    if (isFinished() && !isEmpty()) {
        parent->regionFinish();
    }
}

void DrawItemFacePolygon::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    //判断当前鼠标位置区域数量，确定是否可以绘制
    int count = 0;
    for (auto i : scene()->items(event->scenePos())) {
        if ((i->type() == Type || i->type() == DrawItemPoint::Type) && i->isEnabled()) {
            count++;
        }
    }
    if (count > 1 && m_pointItems.isEmpty()) {
        setIsFinished(true);
        setParentItem(nullptr);
        hide();
        return;
    }
    //判断修改当前鼠标样式
    m_isUnderMouse = false;

    if (isFinished()) {
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

QColor DrawItemFacePolygon::lineColor()
{
    if (index() < 0 || parentItem()->type() == UserType + 4) {
        return QColor(10, 169, 227);
    } else {
        return QColor(255, 255, 0);
    }
}

void DrawItemFacePolygon::showText()
{
    if (isEmpty()) {
        m_textItem->setPlainText("");
        return;
    }
    if (!isFinished()) {
        m_textItem->setPlainText("");
        return;
    }

    //ipc数据变化才会推，这里主动获取缓存中的stay value
    showStayValue();
    QRect textRect = m_textItem->boundingRect().toRect();

    auto vecPoint = points();
    QPolygonF polygon(vecPoint);
    QRegion region(polygon.toPolygon());
    QRect regionRect = region.boundingRect();

    //显示在左上角那个点的上方
    QMap<qreal, int> lengthMap;
    for (int i = 0; i < vecPoint.size(); ++i) {
        qreal length = QLineF(regionRect.topLeft(), vecPoint.at(i)).length();
        lengthMap.insert(length, i);
    }
    QPoint topLeftPoint = vecPoint.at(lengthMap.begin().value()).toPoint();
    textRect.moveBottomLeft(topLeftPoint);
    QPointF sp = mapToScene(textRect.topLeft());
    textRect.moveTopLeft(sp.toPoint());
    if (textRect.left() < scene()->sceneRect().left()) {
        textRect.moveLeft(scene()->sceneRect().left());
    }
    if (textRect.top() < scene()->sceneRect().top()) {
        textRect.moveTop(scene()->sceneRect().top());
    }
    if (textRect.right() > scene()->sceneRect().right()) {
        textRect.moveRight(scene()->sceneRect().right());
    }
    if (textRect.bottom() > scene()->sceneRect().bottom()) {
        textRect.moveBottom(scene()->sceneRect().bottom());
    }
    QPointF p = mapFromScene(textRect.topLeft());
    textRect.moveTopLeft(p.toPoint());

    //
    m_textItem->setPos(textRect.topLeft());
}

void DrawItemFacePolygon::clearText()
{
    m_textItem->setPlainText("");
}

void DrawItemFacePolygon::addPointItem(const QPointF &p)
{
    DrawItemPoint *item = new DrawItemPoint(this);
    item->setPointColor(lineColor());
    QRectF rc(0, 0, 8, 8);
    item->setRect(rc);
    rc.moveCenter(p);
    item->setPos(rc.topLeft());
    m_pointItems.append(item);
}

void DrawItemFacePolygon::showStayValue()
{
    QString text;
    if (index() < 0) {
        text = QString("Detection Region");
    } else if (parentItem()->type() == UserType + 4) {
        text = QString("Region%1").arg(index() + 1);
    } else {
        text = QString("Shield Region%1").arg(index() + 1);
    }
    m_textItem->setDefaultTextColor(QColor(255, 30, 16));
    m_textItem->setPlainText(text);
    if (!m_textItem->isVisible()) {
        m_textItem->show();
    }
}

void DrawItemFacePolygon::hideStayValue()
{
    m_textItem->hide();
}
void DrawItemFacePolygon::setIsEnable(bool isEnable)
{
    unsetCursor();
    setEnabled(isEnable);
}

void DrawItemFacePolygon::setPoints(const QVector<QPointF> &pointVect)
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

bool DrawItemFacePolygon::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::GraphicsSceneHoverMove: {
        if (isEnabled()) {
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
