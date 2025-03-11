#include "itembuttondelegate.h"
#include <QPainter>

ItemButtonDelegate::ItemButtonDelegate(const QPixmap &pixmap, QObject *parent) :
    QStyledItemDelegate(parent)
{
    m_pixmap = pixmap.scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

ItemButtonDelegate::ItemButtonDelegate(const QPixmap &pixmap, const QSize &size, QObject *parent) :
    QStyledItemDelegate(parent)
{
    m_pixmap = pixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}


void ItemButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QStyledItemDelegate::paint(painter, option, index);
    painter->restore();

    if (index.data().toString().isEmpty())
    {
        QRect rc = option.rect;
        rc.setSize(m_pixmap.size());
        rc.moveCenter(option.rect.center());
        painter->drawPixmap(rc, m_pixmap);
    }
}

QWidget *ItemButtonDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(parent)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return nullptr;
}
