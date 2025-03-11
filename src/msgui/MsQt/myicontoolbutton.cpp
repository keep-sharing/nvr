#include "myicontoolbutton.h"
#include <QEvent>
#include <QPainter>
#include <QToolTip>
#include <QtDebug>

MyIconToolButton::MyIconToolButton(QWidget *parent)
    : MyToolButton(parent)
{
    setCheckable(true);
    setChecked(false);
    installEventFilter(this);
}

void MyIconToolButton::setCheckedIcon(const QString &icon)
{
    m_iconChecked = QIcon(icon);
}

void MyIconToolButton::setNormalIcon(const QString &icon)
{
    m_iconNormal = QIcon(icon);
}

void MyIconToolButton::setDisabledIcon(const QString &icon)
{
    m_iconDisabled = QIcon(icon);
}

void MyIconToolButton::paintEvent(QPaintEvent *event)
{
    if (!isEnabled()) {
        if (m_state != StateDisabled) {
            setIcon(m_iconDisabled);
            m_state = StateDisabled;
        }
    } else if (isChecked()) {
        if (m_state != StateChecked) {
            setIcon(m_iconChecked);
            m_state = StateChecked;
        }
    } else {
        if (m_state != StateNormal) {
            setIcon(m_iconNormal);
            m_state = StateNormal;
        }
    }

    //
    MyToolButton::paintEvent(event);
}

bool MyIconToolButton::eventFilter(QObject *watched, QEvent *event)
{
    if (m_isNeedToolTip && event->type() == QEvent::Enter && isVisible()) {
        QToolTip::showText(cursor().pos(), toolTip(), this);
    }
    return MyToolButton::eventFilter(watched, event);
}

void MyIconToolButton::setIsNeedToolTip(bool isNeedToolTip)
{
    m_isNeedToolTip = isNeedToolTip;
}
