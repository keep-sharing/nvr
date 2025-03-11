#include "maskwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtDebug>

MaskWidget::MaskWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
}

void MaskWidget::setColor(const QColor &color)
{
    m_color = color;
    update();
}

QColor MaskWidget::getColor()
{
    return m_color;
}

void MaskWidget::setColorType(int type)
{
    switch (type)
    {
    case 0:
        setColor(Qt::white);
        break;
    case 1:
        setColor(Qt::black);
        break;
    case 2:
        setColor(Qt::blue);
        break;
	case 3:
        setColor(Qt::yellow);
        break;
	case 4:
        setColor(Qt::green);
        break;
	case 5:
        //setColor(Qt::brown);
        setColor(QColor("#7F2A2A"));
        break;
	case 6:
        setColor(Qt::red);
        break;
	case 7:
        //setColor(Qt::purple);
        setColor(QColor("#8A2CDE"));
        break;
    default:
        break;
    }
}

void MaskWidget::setSelected(bool selected)
{
    m_isSelected = selected;
    update();
}

QRect MaskWidget::realRect() const
{
    QRect rc;
    rc.setLeft(pos().x() + m_realRect.left());
    rc.setTop(pos().y() + m_realRect.top());
    rc.setWidth(m_realRect.width());
    rc.setHeight(m_realRect.height());
    return rc;
}

void MaskWidget::setRealRect(const QRect &rc)
{
    QRect rect;
    rect.setLeft(rc.x() - m_margin);
    rect.setTop(rc.y() - m_margin);
    rect.setRight(rc.right() + m_margin);
    rect.setBottom(rc.bottom() + m_margin);
    setGeometry(rect);
}

bool MaskWidget::isUnderMouse(const QPoint &pos) const
{
    const QPoint &p = mapFromParent(pos);
    if (m_realRect.contains(p))
    {
        return true;
    }

    auto anchorIter = m_anchorMap.constBegin();
    for (; anchorIter != m_anchorMap.constEnd(); ++anchorIter)
    {
        const QRect &rc = anchorIter.value();
        if (rc.contains(p))
        {
            return true;
        }
    }
    return false;
}

int MaskWidget::margin() const
{
    return m_margin;
}

void MaskWidget::resizeEvent(QResizeEvent *event)
{
    m_realRect = rect();
    m_realRect.setLeft(m_realRect.left() + m_margin);
    m_realRect.setTop(m_realRect.top() + m_margin);
    m_realRect.setRight(m_realRect.right() - m_margin);
    m_realRect.setBottom(m_realRect.bottom() - m_margin);

    m_anchorMap.clear();
    int side = 4 * 2 + 1;
    QRect rc(0, 0, side, side);

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


    QWidget::resizeEvent(event);
}

void MaskWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    //
//    painter.save();
//    painter.setPen(Qt::NoPen);
//    painter.setBrush(Qt::red);
//    painter.drawRect(rect());
//    painter.restore();
    //
    painter.save();
    painter.setPen(QPen(QColor("#0AA9E3"), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_realRect);

	if (m_index != -1)
	{
		QPoint point(m_realRect.left() + 5, m_realRect.top() + 15);
		painter.drawText(point, QString::number(m_index + 1));
	}
	
    painter.restore();

    //
    if (m_isSelected)
    {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#0AA9E3"));
        auto anchorIter = m_anchorMap.constBegin();
        for (; anchorIter != m_anchorMap.constEnd(); ++anchorIter)
        {
            painter.drawEllipse(anchorIter.value());
        }
        painter.restore();
    }
}

void MaskWidget::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    m_pressPoint = mapToParent(event->pos());
    m_pressDistance = m_pressPoint - pos();
    m_tempGeometry = geometry();
    detectCursor(event->pos(), true);
    QWidget::mousePressEvent(event);
}

void MaskWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_operation = ModeNull;
    detectCursor(event->pos());
    QWidget::mouseReleaseEvent(event);
}

void MaskWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_pressed)
    {
        detectCursor(event->pos());
        QWidget::mouseMoveEvent(event);
        return;
    }

    //
    QPoint pos = mapToParent(event->pos());
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

    QRect rc = m_tempGeometry.normalized();
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
        if (rc.right() > parentWidget()->width() + m_margin)
        {
            rc.moveRight(parentWidget()->width() + m_margin);
        }
        if (rc.bottom() > parentWidget()->height() + m_margin)
        {
            rc.moveBottom(parentWidget()->height() + m_margin);
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
        if (rc.right() > parentWidget()->width() + m_margin)
        {
            rc.setRight(parentWidget()->width() + m_margin);
        }
        if (rc.bottom() > parentWidget()->height() + m_margin)
        {
            rc.setBottom(parentWidget()->height() + m_margin);
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
        if (rc.right() > parentWidget()->width() + m_margin)
        {
            rc.moveRight(parentWidget()->width() + m_margin);
        }
        if (rc.bottom() > parentWidget()->height() + m_margin)
        {
            rc.moveBottom(parentWidget()->height() + m_margin);
        }
    }
    setGeometry(rc);

    QWidget::mouseMoveEvent(event);
}

void MaskWidget::detectCursor(const QPoint &pos, bool setMode/* = false*/)
{
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
        if (setMode)
        {
            m_operation = ModeMove;
        }
    }
    else
    {
        unsetCursor();

        //
        if (!m_isSelected)
        {
            m_operation = ModeNull;
            return;
        }
    }

    //
    auto anchorIter = m_anchorMap.constBegin();
    for (; anchorIter != m_anchorMap.constEnd(); ++anchorIter)
    {
        const QRect &rc = anchorIter.value();
        if (rc.contains(pos))
        {
            const AnchorPosition &postion = anchorIter.key();
            switch (postion)
            {
            case AnchorLeft:
                setCursor(Qt::SizeHorCursor);
                break;
            case AnchorTopLeft:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case AnchorTop:
                setCursor(Qt::SizeVerCursor);
                break;
            case AnchorTopRight:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case AnchorRight:
                setCursor(Qt::SizeHorCursor);
                break;
            case AnchorBottomRight:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case AnchorBottom:
                setCursor(Qt::SizeVerCursor);
                break;
            case AnchorBottomLeft:
                setCursor(Qt::SizeBDiagCursor);
                break;
            }
            //
            if (setMode)
            {
                m_operation = (OperationMode)postion;
            }

            break;
        }
    }
}

void MaskWidget::setIndex(int index)
{
	m_index = index;
}

int MaskWidget::getIndex()
{
    return m_index;
}
