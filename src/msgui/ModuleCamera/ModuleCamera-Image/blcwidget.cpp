#include "blcwidget.h"
#include <QPainter>

BlcWidget::BlcWidget(QWidget *parent) :
    MaskWidget(parent)
{

}

void BlcWidget::setBlcType(BlcType type)
{
    m_blcType = type;
    update();
}

void BlcWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    //
    //    painter.save();
    //    painter.setPen(Qt::NoPen);
    //    painter.setBrush(Qt::red);
    //    painter.drawRect(rect());
    //    painter.restore();
    //
    painter.save();
    painter.setPen(QPen(QColor("#0AA9E3"), 1));
    switch (m_blcType) {
    case BlcCustomize:
        painter.setBrush(QColor(10, 169, 227, 80));
        break;
    case BlcCentre:
        painter.setBrush(m_color);
        break;
    default:
        break;
    }
    painter.drawRect(m_realRect);
    painter.restore();

    //
    if (m_isSelected)
    {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#0AA9E3"));
        auto anchorIter = m_anchorMap.constBegin();
        for (; anchorIter != m_anchorMap.constEnd(); ++anchorIter)
        {
            painter.drawEllipse(anchorIter.value());
        }
        painter.restore();
    }
}

void BlcWidget::mousePressEvent(QMouseEvent *event)
{
    switch (m_blcType) {
    case BlcCustomize:
        MaskWidget::mousePressEvent(event);
        break;
    case BlcCentre:
        break;
    default:
        break;
    }
}

void BlcWidget::mouseReleaseEvent(QMouseEvent *event)
{
    switch (m_blcType) {
    case BlcCustomize:
        MaskWidget::mouseReleaseEvent(event);
        break;
    case BlcCentre:
        break;
    default:
        break;
    }
}

void BlcWidget::mouseMoveEvent(QMouseEvent *event)
{
    switch (m_blcType) {
    case BlcCustomize:
        MaskWidget::mouseMoveEvent(event);
        break;
    case BlcCentre:
        break;
    default:
        break;
    }
}
