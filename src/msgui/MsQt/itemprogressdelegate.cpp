#include "itemprogressdelegate.h"
#include <QPainter>

ItemProgressDelegate::ItemProgressDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void ItemProgressDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    const QString &text = index.data(ItemProgressTextRole).toString();
    const int &value = index.data(ItemProgressValueRole).toInt();
    const QString &error = index.data(ItemProgressErrorRole).toString();

    //背景
    QRect backgroundRect = option.rect;
    backgroundRect.setRight(backgroundRect.right() - 1);
    painter->save();
    painter->setPen(QPen(QColor("#a7a7a7"), 1));
    painter->setBrush(Qt::white);
    painter->drawRect(backgroundRect);
    painter->restore();
    //进度
    QRect rc = option.rect;
    rc.setRight(value / 100.0 * option.rect.right());
    rc.setLeft(rc.left() + 1);
    rc.setTop(rc.top() + 1);
    rc.setBottom(rc.bottom());
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#09a8e2"));
    painter->drawRect(rc);
    painter->restore();
    //text
    QRect textRect = option.rect;
    textRect.setLeft(textRect.left() + 15);
    textRect.setRight(textRect.right() - 50);
    painter->save();
    painter->setPen(QColor("#4A4A4A"));
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    if (error.isEmpty())
    {
        //value
        painter->drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, QString("%1%").arg(value));
        painter->restore();
    }
    else
    {
        //error
        painter->drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, error);
        painter->restore();
    }
}
