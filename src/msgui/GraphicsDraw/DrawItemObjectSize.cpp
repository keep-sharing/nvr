#include "DrawItemObjectSize.h"
#include "MyDebug.h"
#include "DrawSceneObjectSize.h"
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#define MIN_WIDTH 1
#define MIN_HEIGHT 1

DrawItemObjectSize::DrawItemObjectSize(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

void DrawItemObjectSize::setObjectRect(const QRectF &rect, const QString &text, MsQtVca::ObjectSizeType sizeType)
{
    m_text = text;
    m_sizeType = sizeType;
    QRectF itemRc = rect.adjusted(-m_margin, -m_margin, m_margin, m_margin);
    setItemRect(itemRc);
}

void DrawItemObjectSize::setItemRect(const QRectF &rect)
{
    dealResize(rect);
    setRect(rect);

    DrawSceneObjectSize *drawScene = static_cast<DrawSceneObjectSize *>(scene());
    drawScene->updateSize(m_sizeType);
}

QRectF DrawItemObjectSize::objectRect() const
{
    return m_objectRect;
}

void DrawItemObjectSize::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(QPen(QColor("#0AA9E3"), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(m_objectRect);

    QFontMetrics fm(painter->font());
    qreal textWidth = fm.width(m_text);
    qreal textHeight = fm.height();
    QRectF textRc(rect().left() - textWidth, rect().top() - textHeight, textWidth, textHeight);
    if (textRc.left() < scene()->sceneRect().left() || textRc.top() < scene()->sceneRect().top()) {
        if (m_objectRect.width() > textWidth && m_objectRect.height() > textHeight) {
            textRc.moveTopLeft(m_objectRect.topLeft());
        } else {
            textRc.moveTopLeft(rect().bottomRight());
        }
    }
    painter->drawText(textRc, Qt::AlignCenter, m_text);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#0AA9E3"));
    for (auto iter = m_anchorMap.constBegin(); iter != m_anchorMap.constEnd(); ++iter) {
        painter->drawEllipse(iter.value());
    }

    painter->restore();
}

void DrawItemObjectSize::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    detectCursor(event->pos());
    QGraphicsRectItem::hoverMoveEvent(event);
}

void DrawItemObjectSize::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressPoint = event->pos();
        m_pressDistance = m_pressPoint - rect().topLeft();
        m_tempGeometry = rect();
        m_operation = detectCursor(event->pos());

        QGraphicsRectItem::mousePressEvent(event);
    }
}

void DrawItemObjectSize::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressed = false;
    m_operation = ModeNull;
    detectCursor(event->pos());

    QGraphicsRectItem::mouseReleaseEvent(event);
}

void DrawItemObjectSize::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_pressed) {
        //
        QPointF pos = mapToParent(event->pos());
        switch (m_operation) {
        case ModeResizeLeft:
        {
            qreal x = pos.x();
            if (x > m_tempGeometry.right() - MIN_WIDTH - m_margin * 2) {
                x = m_tempGeometry.right() - MIN_WIDTH - m_margin * 2;
            }
            if (x < -m_margin) {
                x = -m_margin;
            }
            m_tempGeometry.setLeft(x);
            break;
        }
        case ModeResizeTopLeft:
        {
            qreal y = pos.y();
            if (y > m_tempGeometry.bottom() - MIN_HEIGHT - m_margin * 2) {
                y = m_tempGeometry.bottom() - MIN_HEIGHT - m_margin * 2;
            }
            if (y < -m_margin) {
                y = -m_margin;
            }
            m_tempGeometry.setTop(y);

            qreal x = pos.x();
            if (x > m_tempGeometry.right() - MIN_WIDTH - m_margin * 2) {
                x = m_tempGeometry.right() - MIN_WIDTH - m_margin * 2;
            }
            if (x < -m_margin) {
                x = -m_margin;
            }
            m_tempGeometry.setLeft(x);
            break;
        }
        case ModeResizeTop:
        {
            qreal y = pos.y();
            if (y > m_tempGeometry.bottom() - MIN_HEIGHT - m_margin * 2) {
                y = m_tempGeometry.bottom() - MIN_HEIGHT - m_margin * 2;
            }
            if (y < -m_margin) {
                y = -m_margin;
            }
            m_tempGeometry.setTop(y);
            break;
        }
        case ModeResizeTopRight:
        {
            qreal y = pos.y();
            if (y > m_tempGeometry.bottom() - MIN_HEIGHT - m_margin * 2) {
                y = m_tempGeometry.bottom() - MIN_HEIGHT - m_margin * 2;
            }
            if (y < -m_margin) {
                y = -m_margin;
            }
            m_tempGeometry.setTop(y);

            qreal x = pos.x();
            if (x < m_tempGeometry.left() + MIN_WIDTH + m_margin * 2) {
                x = m_tempGeometry.left() + MIN_WIDTH + m_margin * 2;
            }
            if (x > scene()->width() + m_margin) {
                x = scene()->width() + m_margin;
            }
            m_tempGeometry.setRight(x);
            break;
        }
        case ModeResizeRight:
        {
            qreal x = pos.x();
            if (x < m_tempGeometry.left() + MIN_WIDTH + m_margin * 2) {
                x = m_tempGeometry.left() + MIN_WIDTH + m_margin * 2;
            }
            if (x > scene()->width() + m_margin) {
                x = scene()->width() + m_margin;
            }
            m_tempGeometry.setRight(x);
            break;
        }
        case ModeResizeBottomRight:
        {
            qreal y = pos.y();
            if (y < m_tempGeometry.top() + MIN_HEIGHT + m_margin * 2) {
                y = m_tempGeometry.top() + MIN_HEIGHT + m_margin * 2;
            }
            if (y > scene()->height() + m_margin) {
                y = scene()->height() + m_margin;
            }
            m_tempGeometry.setBottom(y);

            qreal x = pos.x();
            if (x < m_tempGeometry.left() + MIN_WIDTH + m_margin * 2) {
                x = m_tempGeometry.left() + MIN_WIDTH + m_margin * 2;
            }
            if (x > scene()->width() + m_margin) {
                x = scene()->width() + m_margin;
            }
            m_tempGeometry.setRight(x);
            break;
        }
        case ModeResizeBottom:
        {
            qreal y = pos.y();
            if (y < m_tempGeometry.top() + MIN_HEIGHT + m_margin * 2) {
                y = m_tempGeometry.top() + MIN_HEIGHT + m_margin * 2;
            }
            if (y > scene()->height() + m_margin) {
                y = scene()->height() + m_margin;
            }
            m_tempGeometry.setBottom(y);
            break;
        }
        case ModeResizeBottomLeft:
        {
            qreal y = pos.y();
            if (y < m_tempGeometry.top() + MIN_HEIGHT + m_margin * 2) {
                y = m_tempGeometry.top() + MIN_HEIGHT + m_margin * 2;
            }
            if (y > scene()->height() + m_margin) {
                y = scene()->height() + m_margin;
            }
            m_tempGeometry.setBottom(y);

            qreal x = pos.x();
            if (x > m_tempGeometry.right() - MIN_WIDTH - m_margin * 2) {
                x = m_tempGeometry.right() - MIN_WIDTH - m_margin * 2;
            }
            if (x < -m_margin) {
                x = -m_margin;
            }
            m_tempGeometry.setLeft(x);
            break;
        }
        case ModeMove:
        {
            m_tempGeometry.moveTo(pos - m_pressDistance);
            if (m_tempGeometry.left() < -m_margin) {
                m_tempGeometry.moveLeft(-m_margin);
            }
            if (m_tempGeometry.top() < -m_margin) {
                m_tempGeometry.moveTop(-m_margin);
            }
            if (m_tempGeometry.right() > scene()->width() + m_margin) {
                m_tempGeometry.moveRight(scene()->width() + m_margin);
            }
            if (m_tempGeometry.bottom() > scene()->height() + m_margin) {
                m_tempGeometry.moveBottom(scene()->height() + m_margin);
            }
            break;
        }
        default:
            break;
        }

        setItemRect(m_tempGeometry);
    }

    QGraphicsRectItem::mouseMoveEvent(event);
}

void DrawItemObjectSize::dealResize(const QRectF &rect)
{
    m_objectRect = rect;
    m_objectRect.setLeft(m_objectRect.left() + m_margin);
    m_objectRect.setTop(m_objectRect.top() + m_margin);
    m_objectRect.setRight(m_objectRect.right() - m_margin);
    m_objectRect.setBottom(m_objectRect.bottom() - m_margin);

    m_anchorMap.clear();
    int side = 4 * 2 + 1;
    QRectF rc(0, 0, side, side);

    //左上
    rc.moveCenter(m_objectRect.topLeft());
    m_anchorMap.insert(AnchorTopLeft, rc);
    //右上
    rc.moveCenter(m_objectRect.topRight());
    m_anchorMap.insert(AnchorTopRight, rc);
    //右下
    rc.moveCenter(m_objectRect.bottomRight());
    m_anchorMap.insert(AnchorBottomRight, rc);
    //左下
    rc.moveCenter(m_objectRect.bottomLeft());
    m_anchorMap.insert(AnchorBottomLeft, rc);

    if (m_objectRect.height() > m_margin * 6) {
        //左
        rc.moveCenter(QPoint(m_objectRect.left(), m_objectRect.center().y()));
        m_anchorMap.insert(AnchorLeft, rc);
        //右
        rc.moveCenter(QPoint(m_objectRect.right(), m_objectRect.center().y()));
        m_anchorMap.insert(AnchorRight, rc);
    }

    if (m_objectRect.width() > m_margin * 6) {
        //上
        rc.moveCenter(QPoint(m_objectRect.center().x(), m_objectRect.top()));
        m_anchorMap.insert(AnchorTop, rc);
        //下
        rc.moveCenter(QPoint(m_objectRect.center().x(), m_objectRect.bottom()));
        m_anchorMap.insert(AnchorBottom, rc);
    }
}

DrawItemObjectSize::OperationMode DrawItemObjectSize::detectCursor(const QPointF &pos)
{
    OperationMode mode = ModeNull;
    //
    if (m_objectRect.contains(pos)) {
        if (m_pressed) {
            setCursor(Qt::ClosedHandCursor);
        } else {
            setCursor(Qt::OpenHandCursor);
        }
        mode = ModeMove;
    } else {
        unsetCursor();
    }

    //
    auto anchorIter = m_anchorMap.constBegin();
    for (; anchorIter != m_anchorMap.constEnd(); ++anchorIter) {
        const QRectF &rc = anchorIter.value();
        if (rc.contains(pos)) {
            const AnchorPosition &postion = anchorIter.key();
            switch (postion) {
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
