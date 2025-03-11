#include "toolbutton.h"
#include <QPainter>
#include <QKeyEvent>
#include <QtDebug>

ToolButton::ToolButton(QWidget *parent) :
    QToolButton(parent)
{
    setMouseTracking(true);
}

void ToolButton::setText(const QString &text)
{
    if (toolButtonStyle() != Qt::ToolButtonTextUnderIcon)
    {
        QToolButton::setText(text);
    }
    else
    {
        m_text = text;
        update();
    }
}

QString ToolButton::text() const
{
    if (toolButtonStyle() != Qt::ToolButtonTextUnderIcon)
    {
        return QToolButton::text();
    }
    else
    {
        return m_text;
    }
}

void ToolButton::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

void ToolButton::setHoverPixmap(const QPixmap &pixmap)
{
    m_hoverPixmap = pixmap;
    update();
}

void ToolButton::setPressedPixmap(const QPixmap &pixmap)
{
    m_pressedPixmap = pixmap;
    update();
}

void ToolButton::setTextColor(const QColor &color)
{
    m_textColor = color;
    update();
}

void ToolButton::setTextHoverColor(const QColor &color)
{
    m_textHoverColor = color;
    update();
}

void ToolButton::setTextPressedColor(const QColor &color)
{
    m_textPressedColor = color;
    update();
}

void ToolButton::setTextPixel(int pixel)
{
    m_textPixel = pixel;
    update();
}

void ToolButton::paintEvent(QPaintEvent *event)
{
    QToolButton::paintEvent(event);

    //
    if (toolButtonStyle() != Qt::ToolButtonTextUnderIcon)
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    //
    QFont font = painter.font();
    if (m_textPixel > 0)
    {
        font.setPixelSize(m_textPixel);
    }
    QFontMetrics fm(font);

    //
    QPixmap pixmap = m_pixmap;
    if (m_isPressed)
    {
        if (!m_pressedPixmap.isNull())
        {
            pixmap = m_pressedPixmap;
        }
    }
    else if (m_isUnderMouse)
    {
        if (!m_hoverPixmap.isNull())
        {
            pixmap = m_hoverPixmap;
        }
    }
    //
    QRect pixmapRect;
    QRect textRect(0, 0, fm.width(m_text), fm.height());

    /***
     * 图标位于Button的上面3/5，图标大小为(3/5) * (2/3)
     * 字体占Button的2/5
     ***/
//    int pixmapWidth = height() < width() ? height() * 2 / 5 : width() * 2 / 5;
//    int pixmapHeight = pixmapWidth;
//    QSize pixmapSize = pixmap.size();
//    pixmapSize.scale(pixmapWidth, pixmapHeight, Qt::KeepAspectRatio);
    pixmapRect.setSize(pixmap.size());
    pixmapRect.moveCenter(QPoint(width() / 2, height() / 5 * 2));
    textRect.moveCenter(QPoint(width() / 2, height() / 5 * 4));

    //pixmap
    painter.save();
    painter.drawPixmap(pixmapRect, pixmap);
    painter.restore();

    //text
    painter.save();
    QPen pen = painter.pen();
    if (m_isPressed)
    {
        if (m_textPressedColor.isValid())
        {
            pen.setColor(m_textPressedColor);
        }
        else
        {
            pen.setColor(m_textColor);
        }
    }
    else if (m_isUnderMouse)
    {
        if (m_textHoverColor.isValid())
        {
            pen.setColor(m_textHoverColor);
        }
        else
        {
            pen.setColor(m_textColor);
        }
    }
    else
    {
        pen.setColor(m_textColor);
    }
    painter.setPen(pen);
    painter.setFont(font);
    painter.drawText(textRect, Qt::AlignCenter, m_text);
    painter.restore();
}

void ToolButton::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    if (toolButtonStyle() == Qt::ToolButtonTextBesideIcon)
    {
        setIcon(QIcon(m_pressedPixmap));
    }
    else
    {
        update();
    }
    QToolButton::mousePressEvent(event);
}

void ToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    if (toolButtonStyle() == Qt::ToolButtonTextBesideIcon)
    {
        setIcon(QIcon(m_hoverPixmap));
    }
    else
    {
        update();
    }
    QToolButton::mouseReleaseEvent(event);
}

void ToolButton::enterEvent(QEvent *event)
{
    m_isUnderMouse = true;
    if (!m_isPressed && toolButtonStyle() == Qt::ToolButtonTextBesideIcon)
    {
        setIcon(QIcon(m_hoverPixmap));
    }
    else
    {
        update();
    }
    QToolButton::enterEvent(event);
}

void ToolButton::leaveEvent(QEvent *event)
{
    m_isUnderMouse = false;
    if (toolButtonStyle() == Qt::ToolButtonTextBesideIcon)
    {
        setIcon(QIcon(m_pixmap));
    }
    else
    {
        update();
    }
    QToolButton::leaveEvent(event);
}

void ToolButton::focusInEvent(QFocusEvent *e)
{
    m_isUnderMouse = true;
    if (!m_isPressed && toolButtonStyle() == Qt::ToolButtonTextBesideIcon)
    {
        setIcon(QIcon(m_hoverPixmap));
    }
    else
    {
        update();
    }
    QToolButton::focusInEvent(e);
}

void ToolButton::focusOutEvent(QFocusEvent *e)
{
    m_isUnderMouse = false;
    if (toolButtonStyle() == Qt::ToolButtonTextBesideIcon)
    {
        setIcon(QIcon(m_pixmap));
    }
    else
    {
        update();
    }
    QToolButton::focusOutEvent(e);
}

void ToolButton::keyPressEvent(QKeyEvent *e)
{
    clicked();
    e->accept();
    //QToolButton::keyPressEvent(e);
}

