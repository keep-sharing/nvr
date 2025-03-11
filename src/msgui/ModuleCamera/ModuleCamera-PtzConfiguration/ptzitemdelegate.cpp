#include "ptzitemdelegate.h"
#include <QLineEdit>
#include <QPainter>
#include "presetbuttongroup.h"
#include "patrolbuttongroup.h"
#include "patternbuttongroup.h"

PtzItemDelegate::PtzItemDelegate(ItemType type, QObject *parent) :
    QStyledItemDelegate(parent),
    m_itemType(type)
{

}

void PtzItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 viewOption(option);

    const ItemState &state = (ItemState)index.data(ItemStateRole).toInt();
    QColor textColor("#4A4A4A");
    if (state == ItemDisable || state == ItemInvalidDefault)
    {
        textColor = QColor("#969696");
    }
    viewOption.palette.setColor(QPalette::Text, textColor);
    viewOption.palette.setColor(QPalette::HighlightedText, textColor);
    QStyledItemDelegate::paint(painter, viewOption, index);
}

QWidget *PtzItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    switch (index.column())
    {
    case 1:
    {
        QLineEdit *lineEdit = new QLineEdit(parent);
        //lineEdit->setMaxLength(30);
        return lineEdit;
    }
    case 2:
    {
        switch (m_itemType) {
        case PresetItem:
        {
            PresetButtonGroup *widget = new PresetButtonGroup(index.row(), parent);
            connect(widget, SIGNAL(buttonClicked(int,int)), this, SIGNAL(buttonClicked(int,int)));
            int state = index.data(ItemStateRole).toInt();
            widget->setButtonState(state);
            return widget;
        }
        case PatrolItem:
        {
            PatrolButtonGroup *widget = new PatrolButtonGroup(index.row(), parent);
            connect(widget, SIGNAL(buttonClicked(int,int)), this, SIGNAL(buttonClicked(int,int)));
            int state = index.data(ItemStateRole).toInt();
            widget->setButtonState(state);
            return widget;
        }
        case PatternItem:
        {
            PatternButtonGroup *widget = new PatternButtonGroup(index.row(), parent);
            connect(widget, SIGNAL(buttonClicked(int,int)), this, SIGNAL(buttonClicked(int,int)));
            int state = index.data(ItemStateRole).toInt();
            widget->setButtonState(state);
            return widget;
        }
        default:
            return nullptr;
        }
    }
    default:
        return nullptr;
    }
}
