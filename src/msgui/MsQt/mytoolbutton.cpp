#include "mytoolbutton.h"
#include <QtDebug>

MyToolButton::MyToolButton(QWidget *parent)
    : QToolButton(parent)
{
}

void MyToolButton::clearUnderMouse()
{
    setAttribute(Qt::WA_UnderMouse, false);
}

void MyToolButton::setData(const QVariant &value, int role)
{
    m_dataMap.insert(role, value);
}

QVariant MyToolButton::data(int role) const
{
    return m_dataMap.value(role);
}

void MyToolButton::focusOutEvent(QFocusEvent *event)
{
    QToolButton::focusOutEvent(event);
}

void MyToolButton::enterEvent(QEvent *event)
{
    emit mouseEnter();
    QToolButton::enterEvent(event);
}

void MyToolButton::leaveEvent(QEvent *event)
{
    QToolButton::leaveEvent(event);
}

void MyToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    QToolButton::mouseReleaseEvent(event);
}
