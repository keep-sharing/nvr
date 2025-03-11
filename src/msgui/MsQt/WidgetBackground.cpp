#include "WidgetBackground.h"
#include <QPainter>

WidgetBackground::WidgetBackground(QWidget *parent)
    : QWidget(parent)
{

}

void WidgetBackground::paintEvent(QPaintEvent *)
{
#ifdef _NT98323_
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(46, 46, 46, 200));
    painter.drawRoundedRect(rect(), 5, 5);
#endif
}
