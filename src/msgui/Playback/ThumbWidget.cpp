#include "ThumbWidget.h"
#include <QPainter>

ThumbWidget::ThumbWidget(QWidget *parent) :
    QWidget(parent)
{

}

void ThumbWidget::setImage(const QImage &image)
{
    m_image = image;
    update();
}

void ThumbWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.drawImage(rect(), m_image);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(rect());
}
