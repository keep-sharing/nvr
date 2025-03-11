#include "shadowwidget.h"
#include <QPainter>

ShadowWidget::ShadowWidget(QWidget *parent) :
    QWidget(parent)
{
    m_backgroundColor = QColor(46, 46, 46, 204);
}

void ShadowWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    qreal alphaStep = 100.0 / m_shadowWidth;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    //
    painter.save();
    painter.setBrush(Qt::NoBrush);
    QColor color(0, 0, 0, 50);
    for (int i = 0; i < m_shadowWidth; i++)
    {
        QRect rect(m_shadowWidth - i, m_shadowWidth - i, this->width() - (m_shadowWidth - i) * 2, this->height() - (m_shadowWidth - i) * 2);
        //int alpha = 150 - qSqrt(i) * 50;
        color.setAlpha(100 - i * alphaStep);
        painter.setPen(color);
        painter.drawRoundedRect(rect, i, i);
    }
    painter.restore();
    //
    painter.save();
    QRect rc(m_shadowWidth, m_shadowWidth, width() - 2 * m_shadowWidth, height() - 2 * m_shadowWidth);
    painter.setBrush(QBrush(m_backgroundColor));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rc);
    painter.restore();
}

