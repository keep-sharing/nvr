#include "VideoObject.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

#define MinWidth    20
#define MinHeight   20

VideoObject::VideoObject(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
}

void VideoObject::setObjectInfo(ms_vca_settings_info *info)
{
    m_realWidth = info->width;
    m_realHeight = info->height;

    m_minObject.setTopLeft(physicalPos(info->minobject_window_rectangle_x, info->minobject_window_rectangle_y));
    m_minObject.setWidth(physicalWidth(info->minobject_window_rectangle_width));
    m_minObject.setHeight(physicalHeight(info->minobject_window_rectangle_height));

    m_maxObject.setTopLeft(physicalPos(info->maxobject_window_rectangle_x, info->maxobject_window_rectangle_y));
    m_maxObject.setWidth(physicalWidth(info->maxobject_window_rectangle_width));
    m_maxObject.setHeight(physicalHeight(info->maxobject_window_rectangle_height));
}

int VideoObject::realWidth()
{
    return m_realWidth;
}

int VideoObject::realHeight()
{
    return m_realHeight;
}

void VideoObject::getObjectInfo(ms_vca_settings_info *info)
{
    QPoint minPoint = logicalPos(m_minObject.topLeft());
    info->minobject_window_rectangle_x = minPoint.x();
    info->minobject_window_rectangle_y = minPoint.y();
    info->minobject_window_rectangle_width = logicalWidth(m_minObject.width());
    info->minobject_window_rectangle_height = logicalHeight(m_minObject.height());

    QPoint maxPoint = logicalPos(m_maxObject.topLeft());
    info->maxobject_window_rectangle_x = maxPoint.x();
    info->maxobject_window_rectangle_y = maxPoint.y();
    info->maxobject_window_rectangle_width = logicalWidth(m_maxObject.width());
    info->maxobject_window_rectangle_height = logicalHeight(m_maxObject.height());
}

void VideoObject::getRealObjectInfo(ms_vca_settings_info *info)
{
    info->minobject_window_rectangle_x = m_minObject.topLeft().x();
    info->minobject_window_rectangle_y = m_minObject.topLeft().y();
    info->minobject_window_rectangle_width = (m_minObject.width() < 1)?1:m_minObject.width();
    info->minobject_window_rectangle_height = (m_minObject.height() < 1)?1:m_minObject.height();

    info->maxobject_window_rectangle_x = m_maxObject.topLeft().x();
    info->maxobject_window_rectangle_y = m_maxObject.topLeft().y();
    info->maxobject_window_rectangle_width = (m_maxObject.width() < 1)?1:m_maxObject.width();
    info->maxobject_window_rectangle_height = (m_maxObject.height() < 1)?1:m_maxObject.height();
}

void VideoObject::setRealObjectInfo(ms_vca_settings_info *info)
{
    m_minObject.setLeft(info->minobject_window_rectangle_x);
    m_minObject.setTop(info->minobject_window_rectangle_y);
    m_minObject.setWidth(info->minobject_window_rectangle_width);
    m_minObject.setHeight(info->minobject_window_rectangle_height);

    m_maxObject.setLeft(info->maxobject_window_rectangle_x);
    m_maxObject.setTop(info->maxobject_window_rectangle_y);
    m_maxObject.setWidth(info->maxobject_window_rectangle_width);
    m_maxObject.setHeight(info->maxobject_window_rectangle_height);

    if(m_mode == ModeMax)
    {
        m_objectRect = m_maxObject;
    }
    else
    {
        m_objectRect = m_minObject;
    }
    updateHandleRect();
    update();
}

void VideoObject::getVideoRectInfo(ms_video_rect_info *info)
{
    info->width = width();
    info->height = height();
}

void VideoObject::setMinObject()
{
    m_mode = ModeMin;
    m_objectRect = m_minObject;
    updateHandleRect();
    update();
}

void VideoObject::setMaxObject()
{
    m_mode = ModeMax;
    m_objectRect = m_maxObject;
    updateHandleRect();
    update();
}

void VideoObject::saveObject()
{
    QRect tempRect = m_objectRect.normalized();
    switch (m_mode)
    {
    case ModeMax:
        m_maxObject = tempRect;
        break;
    case ModeMin:
        m_minObject = tempRect;
        break;
    default:
        break;
    }
    m_objectRect = QRect();
    update();
}

void VideoObject::clearObject()
{
    m_maxObject = QRect();
    m_minObject = QRect();
    m_objectRect = QRect();
    update();
}

bool VideoObject::isVaild(const ObjectMode &mode)
{
    bool vaild = true;
    switch (mode)
    {
    case ModeMin:
        if (m_objectRect.width() > m_maxObject.width() || m_objectRect.height() > m_maxObject.height())
        {
            vaild = false;
        }
        break;
    case ModeMax:
        if (m_minObject.width() > m_objectRect.width() || m_minObject.height() > m_objectRect.height())
        {
            vaild = false;
        }
        break;
    default:
        vaild = false;
    }

    return vaild;
}

void VideoObject::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    m_pressPoint = event->pos();
    m_tempRectPos = m_objectRect.topLeft();
    setMouseCursor(event->pos());
}

void VideoObject::mouseReleaseEvent(QMouseEvent *event)
{
    bool isSizeChanged = false;
    bool isPosChanged = false;
    m_pressed = false;
    m_objectRect = m_objectRect.normalized();
    setMouseCursor(event->pos());
    if(m_objectRect.x() + m_objectRect.width() > width())
        m_objectRect.setWidth(width() - m_objectRect.x());
    if(m_objectRect.y() + m_objectRect.height() > height())
        m_objectRect.setHeight(height() - m_objectRect.y());
    switch (m_mode)
    {
    case ModeMax:
        if (m_maxObject.size() != m_objectRect.size())
            isSizeChanged = true;
        if (m_maxObject.topLeft() != m_objectRect.topLeft())
            isPosChanged = true;
        m_maxObject = m_objectRect;
        break;
    case ModeMin:
        if (m_minObject.size() != m_objectRect.size())
            isSizeChanged = true;
        if (m_minObject.topLeft() != m_objectRect.topLeft())
            isPosChanged = true;
        m_minObject = m_objectRect;
        break;
    default:
        break;
    }
    if (m_objectRect.isNull())
    {
        qDebug() << "[david debug] m_objectRect is null";
    }
    emit sigDrawup(isSizeChanged, isPosChanged);
    emit sigDrawup();
}

void VideoObject::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed)
    {
        switch (m_position)
        {
        case Left:
        {
            int left = event->pos().x();
            if (m_objectRect.right() - left < MinWidth)
            {
                left = m_objectRect.right() - MinWidth;
            }
            m_objectRect.setLeft(left);
            break;
        }
        case TopLeft:
        {
            QPoint topLeft = event->pos();
            if (m_objectRect.right() - topLeft.x() < MinWidth)
            {
                topLeft.setX(m_objectRect.right() - MinWidth);
            }
            if (m_objectRect.bottom() - topLeft.y() < MinHeight)
            {
                topLeft.setY(m_objectRect.bottom() - MinHeight);
            }
            m_objectRect.setTopLeft(topLeft);
            break;
        }
        case Top:
        {
            int top = event->pos().y();
            if (m_objectRect.bottom() - top < MinHeight)
            {
                top = m_objectRect.bottom() - MinHeight;
            }
            m_objectRect.setTop(top);
            break;
        }
        case TopRight:
        {
            QPoint topRight = event->pos();
            if (topRight.x() - m_objectRect.left() < MinWidth)
            {
                topRight.setX(m_objectRect.left() + MinWidth);
            }
            if (m_objectRect.bottom() - topRight.y() < MinHeight)
            {
                topRight.setY(m_objectRect.bottom() - MinHeight);
            }
            m_objectRect.setTopRight(topRight);
            break;
        }
        case Right:
        {
            int right = event->pos().x();
            if (right - m_objectRect.left() < MinWidth)
            {
                right = m_objectRect.left() + MinWidth;
            }
            m_objectRect.setRight(right);
            break;
        }
        case BottomRight:
        {
            QPoint bottomRight = event->pos();
            if (bottomRight.x() - m_objectRect.left() < MinWidth)
            {
                bottomRight.setX(m_objectRect.left() + MinWidth);
            }
            if (bottomRight.y() - m_objectRect.top() < MinHeight)
            {
                bottomRight.setY(m_objectRect.top() + MinHeight);
            }
            m_objectRect.setBottomRight(bottomRight);
            break;
        }
        case Bottom:
        {
            int bottom = event->pos().y();
            if (bottom - m_objectRect.top() < MinHeight)
            {
                bottom = m_objectRect.top() + MinHeight;
            }
            m_objectRect.setBottom(bottom);
            break;
        }
        case BottomLeft:
        {
            QPoint bottomLeft = event->pos();
            if (m_objectRect.right() - bottomLeft.x() < MinWidth)
            {
                bottomLeft.setX(m_objectRect.right() - MinWidth);
            }
            if (bottomLeft.y() - m_objectRect.top() < MinHeight)
            {
                bottomLeft.setY(m_objectRect.top() + MinHeight);
            }
            m_objectRect.setBottomLeft(bottomLeft);
            break;
        }
        case Move:
        {
            QPoint movePos = m_tempRectPos + event->pos() - m_pressPoint;
            QRect tempRect = m_objectRect;
            tempRect.moveTo(movePos);
            if (tempRect.left() < 0)
            {
                movePos.setX(0);
            }
            if (tempRect.top() < 0)
            {
                movePos.setY(0);
            }
            if (tempRect.right() > width())
            {
                movePos.setX(width() - tempRect.width());
            }
            if (tempRect.bottom() > height())
            {
                movePos.setY(height() - tempRect.height());
            }
            m_objectRect.moveTo(movePos);
            break;
        }
        default:
            break;
        }
        if (m_objectRect.left() < 0)
        {
            m_objectRect.setLeft(0);
        }
        if (m_objectRect.right() > width())
        {
            m_objectRect.setRight(width());
        }
        if (m_objectRect.top() < 0)
        {
            m_objectRect.setTop(0);
        }
        if (m_objectRect.bottom() > height())
        {
            m_objectRect.setBottom(height());
        }
        updateHandleRect();
        update();
    }
    else
    {
        setMouseCursor(event->pos());
    }
}

void VideoObject::paintEvent(QPaintEvent *)
{
    if (m_objectRect.isNull())
    {
        return;
    }
    QRect tempRect = m_objectRect.normalized();

    QPainter painter(this);

    painter.save();
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#0000FF"), 2));
    painter.drawRect(tempRect);

    QString strText;
    switch (m_mode)
    {
    case ModeMax:
        strText = "Max.";
        break;
    case ModeMin:
        strText = "Min.";
        break;
    default:
        break;
    }
    if (!strText.isEmpty())
    {
        QPoint point(tempRect.left(), tempRect.top() - 5);
        if (point.y() < 20)
        {
            point.setY(point.y() + 20);
        }
        painter.drawText(point, strText);
    }
    painter.restore();

    painter.save();
    painter.setBrush(QColor("#000000"));
    painter.setPen(Qt::NoPen);
    painter.drawRect(m_handleMap.value(Left));
    painter.drawRect(m_handleMap.value(TopLeft));
    painter.drawRect(m_handleMap.value(Top));
    painter.drawRect(m_handleMap.value(TopRight));
    painter.drawRect(m_handleMap.value(Right));
    painter.drawRect(m_handleMap.value(BottomRight));
    painter.drawRect(m_handleMap.value(Bottom));
    painter.drawRect(m_handleMap.value(BottomLeft));
    painter.restore();
}

QPointF VideoObject::lineCenter(const QLineF &line)
{
    return 0.5 * line.p1() + 0.5 * line.p2();
}

void VideoObject::updateHandleRect()
{
    QRect rc(QPoint(0, 0), QSize(6, 6));
    m_handleMap[Left] = rc;
    m_handleMap[Left].moveCenter(QPoint(m_objectRect.left() - 1, m_objectRect.top() - 1 + m_objectRect.height() / 2));
    m_handleMap[TopLeft] = rc;
    m_handleMap[TopLeft].moveCenter(m_objectRect.topLeft());
    m_handleMap[Top] = rc;
    m_handleMap[Top].moveCenter(QPoint(m_objectRect.left() - 1 + m_objectRect.width() / 2, m_objectRect.top() - 1));
    m_handleMap[TopRight] = rc;
    m_handleMap[TopRight].moveCenter(m_objectRect.topRight());
    m_handleMap[Right] = rc;
    m_handleMap[Right].moveCenter(QPoint(m_objectRect.right(), m_objectRect.top() - 1 + m_objectRect.height() / 2));
    m_handleMap[BottomRight] = rc;
    m_handleMap[BottomRight].moveCenter(m_objectRect.bottomRight());
    m_handleMap[Bottom] = rc;
    m_handleMap[Bottom].moveCenter(QPoint(m_objectRect.left() - 1 + m_objectRect.width() / 2, m_objectRect.bottom()));
    m_handleMap[BottomLeft] = rc;
    m_handleMap[BottomLeft].moveCenter(m_objectRect.bottomLeft());
}

void VideoObject::setMouseCursor(const QPoint &point)
{
    if (m_handleMap.value(Left).contains(point))
    {
        m_position = Left;
        setCursor(Qt::SizeHorCursor);
    }
    else if (m_handleMap.value(TopLeft).contains(point))
    {
        m_position = TopLeft;
        setCursor(Qt::SizeFDiagCursor);
    }
    else if (m_handleMap.value(Top).contains(point))
    {
        m_position = Top;
        setCursor(Qt::SizeVerCursor);
    }
    else if (m_handleMap.value(TopRight).contains(point))
    {
        m_position = TopRight;
        setCursor(Qt::SizeBDiagCursor);
    }
    else if (m_handleMap.value(Right).contains(point))
    {
        m_position = Right;
        setCursor(Qt::SizeHorCursor);
    }
    else if (m_handleMap.value(BottomRight).contains(point))
    {
        m_position = BottomRight;
        setCursor(Qt::SizeFDiagCursor);
    }
    else if (m_handleMap.value(Bottom).contains(point))
    {
        m_position = Bottom;
        setCursor(Qt::SizeVerCursor);
    }
    else if (m_handleMap.value(BottomLeft).contains(point))
    {
        m_position = BottomLeft;
        setCursor(Qt::SizeBDiagCursor);
    }
    else if (m_objectRect.contains(point))
    {
        m_position = Move;
        setCursor(Qt::SizeAllCursor);
    }
    else
    {
        unsetCursor();
    }
}

QPoint VideoObject::physicalPos(int x, int y)
{
    return QPoint((qreal)x / m_realWidth * width(), (qreal)y / m_realHeight * height());
}

QPoint VideoObject::logicalPos(const QPoint &pos)
{
    return QPoint((qreal)pos.x() / width() * m_realWidth, (qreal)pos.y() / height() * m_realHeight);
}

qreal VideoObject::physicalWidth(qreal w)
{
    return w / m_realWidth * width();
}

qreal VideoObject::physicalHeight(qreal h)
{
    return h / m_realHeight * height();
}

qreal VideoObject::logicalWidth(qreal w)
{
    return w / width() * m_realWidth;
}

qreal VideoObject::logicalHeight(qreal h)
{
    return h / height() * m_realHeight;
}
