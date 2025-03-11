#include "SmartSearchDrawRegion.h"
#include <QMouseEvent>

SmartSearchDrawRegion::SmartSearchDrawRegion(QWidget *parent)
    : DrawMotion(parent)
{
}

void SmartSearchDrawRegion::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QWidget::mousePressEvent(event);
    } else {
        DrawMotion::mousePressEvent(event);
    }
}
