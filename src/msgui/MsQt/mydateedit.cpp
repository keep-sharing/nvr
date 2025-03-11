#include "mydateedit.h"
#include "mycalendarwidget.h"
#include <QMouseEvent>
#include <QtDebug>

MyDateEdit::MyDateEdit(QWidget *parent) :
    QDateEdit(parent)
{
    setCalendarPopup(true);
    m_calendarWidget = new MyCalendarWidget(this);
    m_calendarWidget->setMinimumSize(300, 300);
    m_calendarWidget->setMaximumSize(300, 300);
    setCalendarWidget(m_calendarWidget);

    setContextMenuPolicy(Qt::NoContextMenu);

    setDisplayFormat("yyyy-MM-dd");
    setCalendarPopup(true);
    setProperty("style", "white");
}

bool MyDateEdit::setProperty(const char *name, const QVariant &value)
{
    if (QString(name) == QString("style"))
    {
        const QString &strType = value.toString();
        //白色主题
        if (strType == QString("white"))
        {
            m_calendarWidget->setWhiteTheme();
        }
        else if (strType == QString("black"))
        {
            m_calendarWidget->setBlackTheme();
        }
    }
    return QDateEdit::setProperty(name, value);
}

void MyDateEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        QWidget::mousePressEvent(event);
    }
    else
    {
        QDateEdit::mousePressEvent(event);
    }
}
