#include "playbacktoolbutton.h"
#include <QPainter>

PlaybackToolButton::PlaybackToolButton(QWidget *parent) :
    MyToolButton(parent)
{

}

void PlaybackToolButton::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    update();
}

void PlaybackToolButton::paintEvent(QPaintEvent *event)
{
    MyToolButton::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_backgroundColor);
    painter.drawRect(rect());
}
