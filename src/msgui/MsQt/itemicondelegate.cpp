#include "itemicondelegate.h"
#include <QPainter>
#include <QtDebug>

ItemIconDelegate::ItemIconDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

ItemIconDelegate::~ItemIconDelegate()
{

}

void ItemIconDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    //图标居中
    //option->decorationAlignment = Qt::AlignCenter;
    //option->decorationPosition = QStyleOptionViewItem::Top;
}

void ItemIconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    const QPixmap &pixmap = index.data(PixmapRole).value<QPixmap>();
    if (!pixmap.isNull())
    {
        painter->save();
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        QRect rc = option.rect;
        rc.setSize(QSize(24, 24));
        rc.moveCenter(option.rect.center());
        painter->drawPixmap(rc, pixmap);
        painter->restore();
    }
}
