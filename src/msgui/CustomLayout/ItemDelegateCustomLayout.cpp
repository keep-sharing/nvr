#include "ItemDelegateCustomLayout.h"
#include "lineeditdelegate.h"
#include <QPainter>

ItemDelegateCustomLayout::ItemDelegateCustomLayout(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void ItemDelegateCustomLayout::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    const QPixmap &pixmap = index.data(ItemPixmapRole).value<QPixmap>();
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

QWidget *ItemDelegateCustomLayout::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    switch (index.column()) {
    case 1:
    {
        QString text = index.data(Qt::DisplayRole).toString();
        if (text.isEmpty()) {
            return nullptr;
        }
        LineEditDelegate *lineEdit = new LineEditDelegate(parent);
        connect(lineEdit, SIGNAL(sigEditintFinished(int,int,QString,QString)), this, SIGNAL(editintFinished(int,int,QString,QString)));
        lineEdit->setRow(index.row());
        lineEdit->setColumn(index.column());
        lineEdit->setOldText(text);
        lineEdit->setMaxLength(16);
        return lineEdit;
    }
    default:
        return nullptr;
    }
}

void ItemDelegateCustomLayout::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}
