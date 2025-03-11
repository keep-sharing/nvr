#include "key.h"
#include <QDebug>
#include <QMouseEvent>

Key::Key(QWidget *parent)
    : QToolButton(parent)
{
}

bool Key::qwsEvent(QWSEvent *event)
{
    return QToolButton::qwsEvent(event);
}

void Key::mousePressEvent(QMouseEvent *event)
{
    QToolButton::mousePressEvent(event);
}

void Key::mouseReleaseEvent(QMouseEvent *event)
{
    QToolButton::mouseReleaseEvent(event);
}

void Key::hideEvent(QHideEvent *event)
{
    QToolButton::hideEvent(event);
}
