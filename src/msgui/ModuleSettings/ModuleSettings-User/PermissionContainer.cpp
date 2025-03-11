#include "PermissionContainer.h"
#include <QPainter>

PermissionContainer::PermissionContainer(QWidget *parent)
    : QWidget(parent)
{

}

void PermissionContainer::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setPen(QPen(QColor(170,170,170)));
    painter.setBrush(QBrush(QColor(190,190,190)));
    painter.drawRect(rect());
}
