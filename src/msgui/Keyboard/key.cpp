#include "key.h"
#include <QDebug>
#include <QMouseEvent>
#include "screencontroller.h"

Key::Key(QWidget *parent) : QToolButton(parent)
{}

bool Key::qwsEvent(QWSEvent *event)
{
    QWSMouseEvent *mouseEvent = event->asMouse();

    if (ScreenController::instance()->refreshRate() < 60 && mouseEvent && mouseEvent->simpleData.state >= 1) {
        isPressed = !isPressed;
        if (isPressed) pressCount++;
        qDebug() << "press count: " << pressCount;
    }

    return false;
}

void Key::mousePressEvent(QMouseEvent * event)
{    
    if (ScreenController::instance()->refreshRate() < 60 && Q_UNLIKELY(--pressCount < 0)) {
        qDebug() << "skip mouse press due to press count < 0";
        pressCount = 0;
        return;
    }

    QToolButton::mousePressEvent(event);
}

void Key::mouseReleaseEvent(QMouseEvent * event)
{
    QToolButton::mouseReleaseEvent(event);
}

void Key::hideEvent(QHideEvent *)
{
    isPressed = false;
    pressCount = 0;
}
