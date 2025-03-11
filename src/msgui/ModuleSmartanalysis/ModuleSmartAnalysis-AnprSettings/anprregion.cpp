#include "anprregion.h"
#include <QPainter>

AnprRegion::AnprRegion(QWidget *parent) :
    MaskWidget(parent)
{

}

AnprRegion::AnprRegion(int index, QWidget *parent) :
    MaskWidget(parent),
    m_index(index)
{

}

void AnprRegion::setIndex(int index)
{
    m_index = index;
}

int AnprRegion::index() const
{
    return m_index;
}

void AnprRegion::setName(const QString &name)
{
    m_name = name;
}

QString AnprRegion::name()
{
    if (m_name.isEmpty())
    {
        m_name = QString("ROI_%1").arg(m_index + 1);
    }
    return m_name;
}

QRect AnprRegion::realRect() const
{
    QRect rc;
    rc.setLeft(pos().x() + m_realRect.left());
    rc.setTop(pos().y() + m_realRect.top());
    rc.setWidth(m_realRect.width());
    rc.setHeight(m_realRect.height());
    return rc;
}

void AnprRegion::setRealRect(const QRect &rc)
{
    QRect rect;
    rect.setLeft(rc.x() - m_margin);
    rect.setTop(rc.y() - m_margin);
    rect.setRight(rc.right() + m_margin);
    rect.setBottom(rc.bottom() + m_margin);
    setGeometry(rect);
}

void AnprRegion::paintEvent(QPaintEvent *event)
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
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_realRect);
    painter.restore();

    //
//    QRect textRect = m_realRect;
//    textRect.setLeft(m_realRect.left() + 5);
//    painter.save();
//    painter.setPen(Qt::red);
//    painter.drawText(textRect, QString("%1 %2*%3").arg(m_index + 1).arg(m_realRect.width()).arg(m_realRect.height()), QTextOption(Qt::AlignTop | Qt::AlignLeft));
//    painter.restore();
    //

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
