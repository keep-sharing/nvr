#include "patrolitemdelegate.h"
#include <QPainter>
#include "patrolitembutton.h"

PatrolItemDelegate::PatrolItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void PatrolItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 viewOption(option);

    ItemType type = (ItemType)index.data(ItemPatrolRole).toInt();
    QColor textColor("#FFFFFF");
    if (type == ItemDisable)
    {
        textColor = QColor("#A4A4A4");
    }
    viewOption.palette.setColor(QPalette::Text, textColor);
    viewOption.palette.setColor(QPalette::HighlightedText, textColor);
    QStyledItemDelegate::paint(painter, viewOption, index);
}

QWidget *PatrolItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    switch (index.column())
    {
    case 2:
    {
        PatrolItemButton *widget = new PatrolItemButton(index.row(), parent);
        connect(widget, SIGNAL(buttonClicked(int,int)), this, SIGNAL(buttonClicked(int,int)));
        //
        int type = index.data(ItemPatrolRole).toInt();
        widget->setButtonType(type);
        return widget;
    }
    default:
        return nullptr;
    }
}

void PatrolItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}
