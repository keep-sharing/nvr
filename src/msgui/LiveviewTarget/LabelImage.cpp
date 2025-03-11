#include "LabelImage.h"
#include <QPainter>

LabelImage::LabelImage(QWidget *parent) :
    QLabel(parent)
{
    m_ratio = 16.0 / 9;
}

void LabelImage::setImage(const QImage &image)
{
    m_image = image;
    update();
}

void LabelImage::clearImage()
{
    m_image = QImage();
    update();
}

QImage LabelImage::image() const
{
    return m_image;
}

void LabelImage::setSmoothPixmapTransform(bool enable)
{
    m_isSmoothPixmapTransform = enable;
}

void LabelImage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (m_isSmoothPixmapTransform)
    {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
    }

    QRect rc = rect();
    if ((qreal)rc.width() / rc.height() > m_ratio)
    {
        rc.setWidth(rc.height() * m_ratio);
    }
    else
    {
        rc.setHeight(rc.width() / m_ratio);
    }
    rc.moveCenter(rect().center());
    painter.drawImage(rc, m_image);
}

void LabelImage::setRatio(const qreal &ratio)
{
    m_ratio = ratio;
}
