#include "itemtextdelegate.h"

ItemTextDelegate::ItemTextDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void ItemTextDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 viewOption(option);

    QString strColor = index.data(TextColorRole).toString();
    if (strColor.isEmpty())
    {
        strColor = "#FFFFFF";
    }
    QColor textColor(strColor);
    viewOption.palette.setColor(QPalette::Text, textColor);
    viewOption.palette.setColor(QPalette::HighlightedText, textColor);

    QStyledItemDelegate::paint(painter, viewOption, index);
}
