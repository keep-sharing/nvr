#include "DrawItemRoi.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QCursor>
#include <QtDebug>

DrawItemRoi::DrawItemRoi(QGraphicsItem *parent) :
    QGraphicsRectItem(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

void DrawItemRoi::setRect(const QRectF &rect)
{
    dealResize(rect);
    QGraphicsRectItem::setRect(rect);
}

void DrawItemRoi::setRect(qreal x, qreal y, qreal w, qreal h)
{
    setRect(QRectF(x, y, w, h));
}

void DrawItemRoi::setIndex(int index)
{
    m_index = index;
}

int DrawItemRoi::getIndex() const
{
    return m_index;
}

int DrawItemRoi::margin() const
{
    return m_margin;
}

QRectF DrawItemRoi::realRect() const
{
    QRectF rc;
    rc.setLeft(pos().x() + m_realRect.left());
    rc.setTop(pos().y() + m_realRect.top());
    rc.setWidth(m_realRect.width());
    rc.setHeight(m_realRect.height());
    return rc;
}

void DrawItemRoi::setRealRect(const QRectF &rc)
{
    QRectF rect;
    rect.setLeft(rc.x() - m_margin);
    rect.setTop(rc.y() - m_margin);
    rect.setRight(rc.right() + m_margin);
    rect.setBottom(rc.bottom() + m_margin);
    setRect(rect);
}

bool DrawItemRoi::isSelected() const
{
    return m_isSelected;
}

void DrawItemRoi::setSelected(bool selected)
{
    m_isSelected = selected;
    update();
}

void DrawItemRoi::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    //
    //    painter.save();
    //    painter.setPen(Qt::NoPen);
    //    painter.setBrush(Qt::red);
    //    painter.drawRect(rect());
    //    painter.restore();
    //
    painter->setPen(QPen(QColor("#0AA9E3"), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(m_realRect);

    if (m_index != -1)
    {
        QPoint point(m_realRect.left() + 5, m_realRect.top() + 15);
        painter->drawText(point, QString::number(m_index + 1));
    }

    //
    if (m_isSelected)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#0AA9E3"));
        auto anchorIter = m_anchorMap.constBegin();
        for (; anchorIter != m_anchorMap.constEnd(); ++anchorIter)
        {
            painter->drawEllipse(anchorIter.value());
        }
    }
    painter->restore();
}

void DrawItemRoi::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    detectCursor(event->pos());
    QGraphicsRectItem::hoverMoveEvent(event);
}

void DrawItemRoi::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressed = true;
    m_pressPoint = event->pos();
    m_pressDistance = m_pressPoint - rect().topLeft();
    m_tempGeometry = rect();
    m_operation = detectCursor(event->pos());

    QGraphicsRectItem::mousePressEvent(event);
}

void DrawItemRoi::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressed = false;
    m_operation = ModeNull;
    detectCursor(event->pos());

    QGraphicsRectItem::mouseReleaseEvent(event);
}

void DrawItemRoi::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_pressed)
    {
        //
        QPointF pos = mapToParent(event->pos());
        switch (m_operation)
        {
        case ModeResizeLeft:
            m_tempGeometry.setLeft(pos.x());
            break;
        case ModeResizeTopLeft:
            m_tempGeometry.setTopLeft(pos);
            break;
        case ModeResizeTop:
            m_tempGeometry.setTop(pos.y());
            break;
        case ModeResizeTopRight:
            m_tempGeometry.setTopRight(pos);
            break;
        case ModeResizeRight:
            m_tempGeometry.setRight(pos.x());
            break;
        case ModeResizeBottomRight:
            m_tempGeometry.setBottomRight(pos);
            break;
        case ModeResizeBottom:
            m_tempGeometry.setBottom(pos.y());
            break;
        case ModeResizeBottomLeft:
            m_tempGeometry.setBottomLeft(pos);
            break;
        case ModeMove:
            m_tempGeometry.moveTo(pos - m_pressDistance);
            break;
        default:
            break;
        }

        QRectF rc = m_tempGeometry.normalized();
        if (m_operation == ModeMove)
        {
            if (rc.left() < -m_margin)
            {
                rc.moveLeft(-m_margin);
            }
            if (rc.top() < -m_margin)
            {
                rc.moveTop(-m_margin);
            }
            if (rc.right() > scene()->width() + m_margin)
            {
                rc.moveRight(scene()->width() + m_margin);
            }
            if (rc.bottom() > scene()->height() + m_margin)
            {
                rc.moveBottom(scene()->height() + m_margin);
            }
        }
        else
        {
            if (rc.left() < -m_margin)
            {
                rc.setLeft(-m_margin);
            }
            if (rc.top() < -m_margin)
            {
                rc.setTop(-m_margin);
            }
            if (rc.right() > scene()->width() + m_margin)
            {
                rc.setRight(scene()->width() + m_margin);
            }
            if (rc.bottom() > scene()->height() + m_margin)
            {
                rc.setBottom(scene()->height() + m_margin);
            }
            //设置最小值
            if (rc.width() < 20)
            {
                rc.setWidth(20);
            }
            if (rc.height() < 20)
            {
                rc.setHeight(20);
            }
            if (rc.left() < -m_margin)
            {
                rc.moveLeft(-m_margin);
            }
            if (rc.top() < -m_margin)
            {
                rc.moveTop(-m_margin);
            }
            if (rc.right() > scene()->width() + m_margin)
            {
                rc.moveRight(scene()->width() + m_margin);
            }
            if (rc.bottom() > scene()->height() + m_margin)
            {
                rc.moveBottom(scene()->height() + m_margin);
            }
        }
        setRect(rc);
    }

    QGraphicsRectItem::mouseMoveEvent(event);
}

void DrawItemRoi::dealResize(const QRectF &rect)
{
    m_realRect = rect;
    m_realRect.setLeft(m_realRect.left() + m_margin);
    m_realRect.setTop(m_realRect.top() + m_margin);
    m_realRect.setRight(m_realRect.right() - m_margin);
    m_realRect.setBottom(m_realRect.bottom() - m_margin);

    m_anchorMap.clear();
    int side = 4 * 2 + 1;
    QRectF rc(0, 0, side, side);

    //左上
    rc.moveCenter(m_realRect.topLeft());
    m_anchorMap.insert(AnchorTopLeft, rc);
    //右上
    rc.moveCenter(m_realRect.topRight());
    m_anchorMap.insert(AnchorTopRight, rc);
    //右下
    rc.moveCenter(m_realRect.bottomRight());
    m_anchorMap.insert(AnchorBottomRight, rc);
    //左下
    rc.moveCenter(m_realRect.bottomLeft());
    m_anchorMap.insert(AnchorBottomLeft, rc);

    if (m_realRect.height() > m_margin * 6)
    {
        //左
        rc.moveCenter(QPoint(m_realRect.left(), m_realRect.center().y()));
        m_anchorMap.insert(AnchorLeft, rc);
        //右
        rc.moveCenter(QPoint(m_realRect.right(), m_realRect.center().y()));
        m_anchorMap.insert(AnchorRight, rc);
    }

    if (m_realRect.width() > m_margin * 6)
    {
        //上
        rc.moveCenter(QPoint(m_realRect.center().x(), m_realRect.top()));
        m_anchorMap.insert(AnchorTop, rc);
        //下
        rc.moveCenter(QPoint(m_realRect.center().x(), m_realRect.bottom()));
        m_anchorMap.insert(AnchorBottom, rc);
    }
}

DrawItemRoi::OperationMode DrawItemRoi::detectCursor(const QPointF &pos)
{
    OperationMode mode = ModeNull;
    //
    if (m_realRect.contains(pos))
    {
        if (m_pressed)
        {
            setCursor(Qt::ClosedHandCursor);
        }
        else
        {
            setCursor(Qt::OpenHandCursor);
        }
        mode = ModeMove;
    }
    else
    {
        unsetCursor();

        //
        if (!m_isSelected)
        {
            mode = ModeNull;
            return mode;
        }
    }

    //
    auto anchorIter = m_anchorMap.constBegin();
    for (; anchorIter != m_anchorMap.constEnd(); ++anchorIter)
    {
        const QRectF &rc = anchorIter.value();
        if (rc.contains(pos))
        {
            const AnchorPosition &postion = anchorIter.key();
            switch (postion)
            {
            case AnchorLeft:
                setCursor(Qt::SizeHorCursor);
                mode = ModeResizeLeft;
                break;
            case AnchorTopLeft:
                setCursor(Qt::SizeFDiagCursor);
                mode = ModeResizeTopLeft;
                break;
            case AnchorTop:
                setCursor(Qt::SizeVerCursor);
                mode = ModeResizeTop;
                break;
            case AnchorTopRight:
                setCursor(Qt::SizeBDiagCursor);
                mode = ModeResizeTopRight;
                break;
            case AnchorRight:
                setCursor(Qt::SizeHorCursor);
                mode = ModeResizeRight;
                break;
            case AnchorBottomRight:
                setCursor(Qt::SizeFDiagCursor);
                mode = ModeResizeBottomRight;
                break;
            case AnchorBottom:
                setCursor(Qt::SizeVerCursor);
                mode = ModeResizeBottom;
                break;
            case AnchorBottomLeft:
                setCursor(Qt::SizeBDiagCursor);
                mode = ModeResizeBottomLeft;
                break;
            }
            //
            break;
        }
    }
    return mode;
}
