#include "presetitemdelegate.h"
#include <QLineEdit>
#include <QPainter>
#include "presetitembutton.h"

PresetItemDelegate::PresetItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void PresetItemDelegate::setTheme(PresetItemDelegate::Theme theme)
{
    m_theme = theme;
}

void PresetItemDelegate::setEditable(bool enable)
{
    m_editable = enable;
}

void PresetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 viewOption(option);

    ItemType type = (ItemType)index.data(ItemTypeRole).toInt();
    QColor textColor;;
    switch (m_theme)
    {
    case WhiteTheme:
        textColor = QColor("#FFFFFF");
        break;
    case GrayTheme:
        textColor = QColor("#4A4A4A");
        break;
    }

    if (type == ItemDisable || type == ItemInvalidDefault || type == ItemLimitsDisable)
    {
        textColor = QColor("#A4A4A4");
    }
    viewOption.palette.setColor(QPalette::Text, textColor);
    viewOption.palette.setColor(QPalette::HighlightedText, textColor);
    QStyledItemDelegate::paint(painter, viewOption, index);
}

QWidget *PresetItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    switch (index.column())
    {
    case 1:
    {
        if (m_editable)
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
            lineEdit->setMaxLength(30);
            return lineEdit;
        }
        break;
    }
    case 2:
    {
        PresetItemButton *widget = new PresetItemButton(index.row(), parent);
        widget->setTheme(m_theme);
        connect(widget, SIGNAL(buttonClicked(int,int)), this, SIGNAL(buttonClicked(int,int)));
        //
        int type = index.data(ItemTypeRole).toInt();
        widget->setButtonType(type);
        return widget;
    }
    default:
        return nullptr;
    }
    return nullptr;
}

void PresetItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//    if (index.column() == 0)
//    {
//        editor->setGeometry(option.rect - QMargins(1, 1, 1, 1));
//    }

    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

