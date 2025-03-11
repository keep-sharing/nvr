#include "itemchanneldelegate.h"
#include <QPainter>

ItemChannelDelegate::ItemChannelDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void ItemChannelDelegate::setCheckedPixmap(const QString &text)
{
    m_checkedPixmap = QPixmap(text).scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void ItemChannelDelegate::setUncheckedPixmap(const QString &text)
{
    m_uncheckedPixmap = QPixmap(text).scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void ItemChannelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QStyledItemDelegate::paint(painter, option, index);
    painter->restore();

    const QString &text = index.data(ItemTextRole).toString();
    const QString &colorName = index.data(ItemColorRole).toString();
    if (index.column() == 0)
    {
        const QPoint centerPoint = option.rect.center();

        QFontMetrics fm(painter->font());
        int width = fm.width("99");

        QRect rc = option.rect;
        rc.setWidth(18 + width + 2);
        rc.moveCenter(option.rect.center());

        //pixmap
        painter->save();
        QRect pixmapRect = rc;
        pixmapRect.setSize(QSize(18, 18));
        pixmapRect.moveCenter(QPoint(centerPoint.x() - 9, centerPoint.y()));
        pixmapRect.moveLeft(rc.left());
        bool checked = index.data(ItemCheckedRole).toBool();
        if (checked)
        {
            painter->drawPixmap(pixmapRect, m_checkedPixmap);
        }
        else
        {
            painter->drawPixmap(pixmapRect, m_uncheckedPixmap);
        }
        painter->restore();

        //text
        painter->save();
        QRect textRect = rc;
        textRect.setLeft(rc.left() + 20);
        if (colorName.isEmpty())
        {
            painter->setPen(QColor("#FFFFFF"));
        }
        else
        {
            painter->setPen(QColor(colorName));
        }
        QString elidedText = QFontMetrics(painter->font()).elidedText(text, Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignCenter, elidedText);
        painter->restore();
    }
    else
    {
        painter->save();
        if (colorName.isEmpty())
        {
            painter->setPen(QColor("#FFFFFF"));
        }
        else
        {
            painter->setPen(QColor(colorName));
        }
        QString elidedText = QFontMetrics(painter->font()).elidedText(text, Qt::ElideRight, option.rect.width());
        painter->drawText(option.rect, Qt::AlignCenter, elidedText);
        painter->restore();
    }
}
