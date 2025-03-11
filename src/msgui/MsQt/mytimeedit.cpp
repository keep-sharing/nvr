#include "mytimeedit.h"
#include <QWheelEvent>
#include <QMouseEvent>

MyTimeEdit::MyTimeEdit(QWidget *parent) :
    QTimeEdit(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    setDisplayFormat("HH:mm:ss");
}

void MyTimeEdit::wheelEvent(QWheelEvent *event)
{
    QTime time = dateTime().time();

    int numDegrees = event->delta();
    const Section &section = currentSection();
    switch (section) {
    case QDateTimeEdit::SecondSection:
    {
        if (time.second() == 0 && numDegrees < 0)
        {
            setTime(QTime(time.hour(), time.minute(), 59));
            return;
        }
        if (time.second()== 59 && numDegrees > 0)
        {
            setTime(QTime(time.hour(), time.minute(), 0));
            return;
        }
        break;
    }
    case QDateTimeEdit::MinuteSection:
    {
        if (time.minute() == 0 && numDegrees < 0)
        {
            setTime(QTime(time.hour(), 59, time.second()));
            return;
        }
        if (time.minute() == 59 && numDegrees > 0)
        {
            setTime(QTime(time.hour(), 0, time.second()));
            return;
        }
        break;
    }
    case QDateTimeEdit::HourSection:
    {
        if (time.hour() == 0 && numDegrees < 0)
        {
            setTime(QTime(23, time.minute(), time.second()));
            return;
        }
        if (time.hour() == 23 && numDegrees > 0)
        {
            setTime(QTime(0, time.minute(), time.second()));
            return;
        }
        break;
    }
    default:
        break;
    }

    QTimeEdit::wheelEvent(event);
}

void MyTimeEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        QWidget::mousePressEvent(event);
    }
    else
    {
        QTimeEdit::mousePressEvent(event);
    }
}
