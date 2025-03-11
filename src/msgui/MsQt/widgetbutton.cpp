#include "widgetbutton.h"
#include <QPainter>
#include <QtDebug>

WidgetButton::WidgetButton(QWidget *parent) :
    QWidget(parent)
{

}

void WidgetButton::enterEvent(QEvent *)
{
    qDebug() << objectName() << "enterEvent";
    m_state = StateHover;
    update();
}

void WidgetButton::leaveEvent(QEvent *)
{
    qDebug() << objectName() << "leaveEvent";
    m_state = StateNormal;
    update();
}

void WidgetButton::focusInEvent(QFocusEvent *)
{
    qDebug() << objectName() << "focusInEvent";
}

void WidgetButton::focusOutEvent(QFocusEvent *)
{
    qDebug() << objectName() << "focusOutEvent";
}

void WidgetButton::mousePressEvent(QMouseEvent *)
{
    m_state = StatePressed;
    update();
}

void WidgetButton::mouseReleaseEvent(QMouseEvent *)
{
    m_state = StateHover;
    update();

    emit clicked();
}

void WidgetButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    switch (m_state) {
    case StateNormal:
        return;
    case StateHover:
        painter.setBrush(QColor(44, 122, 158, 220));
        break;
    case StatePressed:
        painter.setBrush(QColor(44, 122, 158, 150));
        break;
    }
    painter.drawRect(rect());
}
