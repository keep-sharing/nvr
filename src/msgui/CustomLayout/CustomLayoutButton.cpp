#include "CustomLayoutButton.h"
#include <QPainter>

CustomLayoutButton::CustomLayoutButton(QWidget *parent)
    : MyToolButton(parent)
{
}

void CustomLayoutButton::setTrianglePosition(CustomLayoutButton::TrianglePosition position)
{
    m_position = position;
    update();
}

void CustomLayoutButton::paintEvent(QPaintEvent *event)
{
    MyToolButton::paintEvent(event);

    if (isEnabled()) {
        QPainter painter(this);
        switch (m_position) {
        case Right:
            drawRightTriangle(&painter);
            break;
        case BottomRight:
            drawBottomRightTriangle(&painter);
            break;
        default:
            break;
        }
    }
}

void CustomLayoutButton::drawRightTriangle(QPainter *painter)
{
    int marginRight = 0;
    int w = 5;
    int h = 8;
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::white);
    painter->setBrush(Qt::white);
    static const QPoint points[7] = {
        QPoint(width() - marginRight, height() / 2),
        QPoint(width() - marginRight - w, (height() - h) / 2),
        QPoint(width() - marginRight - w, (height() + h) / 2)
    };
    painter->drawPolygon(points, 3);
}

void CustomLayoutButton::drawBottomRightTriangle(QPainter *painter)
{
    int marginRight = 2;
    int marginBottom = 8;
    int w = 8;
    int h = 5;
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(QColor(112, 112, 112));
    painter->setBrush(QColor(112, 112, 112));
    static const QPoint points[7] = {
        QPoint(width() - marginRight - w, height() - marginBottom - h),
        QPoint(width() - marginRight, height() - marginBottom - h),
        QPoint(width() - marginRight - w / 2, height() - marginBottom)
    };
    painter->drawPolygon(points, 3);
}
