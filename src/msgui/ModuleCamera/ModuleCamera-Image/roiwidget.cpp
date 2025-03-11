#include "roiwidget.h"
#include <QPainter>

RoiWidget::RoiWidget(QWidget *parent) :
    MaskWidget(parent)
{

}

void RoiWidget::setIndex(int index)
{
    m_index = index;
}

void RoiWidget::paintEvent(QPaintEvent *event)
{
    MaskWidget::paintEvent(event);
#if 0
    if (m_index >= 0)
    {
        QPainter painter(this);
        painter.setPen(QPen(QColor("#FEFE00")));
        QPoint point(m_realRect.left() + 5, m_realRect.top() + 15);
        painter.drawText(point, QString::number(m_index + 1));
    }
#endif
}
