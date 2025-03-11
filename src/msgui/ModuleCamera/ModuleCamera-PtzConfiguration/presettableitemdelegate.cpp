#include "presettableitemdelegate.h"
#include <QPainter>
#include <QLineEdit>

PresetTableItemDelegate::PresetTableItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void PresetTableItemDelegate::setEditable(bool enable)
{
    m_editable = enable;
}

void PresetTableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    const ItemType &type = static_cast<ItemType>(index.data(ItemStateRole).toInt());
    switch (index.column()) {
    case PresetColumnNumber:
    {
        QStyleOptionViewItemV4 viewOption(option);
        QColor textColor("#4A4A4A");
        if (type == ItemDisable || type == ItemInvalidDefault)
        {
            textColor = QColor("#A4A4A4");
        }
        viewOption.palette.setColor(QPalette::Text, textColor);
        viewOption.palette.setColor(QPalette::HighlightedText, textColor);
        QStyledItemDelegate::paint(painter, viewOption, index);
        break;
    }
    case PresetColumnPlay:
    {
        if (type == ItemNormal || type == ItemValidDefault)
        {
            static QPixmap pixmapPlay = QPixmap(":/ptz/ptz/patrolBegin_gray.png").scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QRect rc;
            rc.setSize(QSize(18, 18));
            rc.moveCenter(option.rect.center());
            painter->save();
            painter->drawPixmap(rc, pixmapPlay);
            painter->restore();
        }
        else
        {
            painter->save();
            painter->drawText(option.rect, Qt::AlignCenter, "-");
            painter->restore();
        }
        break;
    }
    case PresetColumnSave:
    {
        if (type == ItemNormal || type == ItemDisable)
        {
            static QPixmap pixmapSave = QPixmap(":/ptz/ptz/save_gray.png").scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QRect rc;
            rc.setSize(QSize(18, 18));
            rc.moveCenter(option.rect.center());
            painter->save();
            painter->drawPixmap(rc, pixmapSave);
            painter->restore();
        }
        else
        {
            painter->save();
            painter->drawText(option.rect, Qt::AlignCenter, "-");
            painter->restore();
        }
        break;
    }
    case PresetColumnDelete:
    {
        if (type == ItemNormal)
        {
            static QPixmap pixmapDelete = QPixmap(":/common/common/delete.png").scaled(26, 26, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QRect rc;
            rc.setSize(QSize(26, 26));
            rc.moveCenter(option.rect.center());
            painter->save();
            painter->drawPixmap(rc, pixmapDelete);
            painter->restore();
        }
        else
        {
            painter->save();
            painter->drawText(option.rect, Qt::AlignCenter, "-");
            painter->restore();
        }
        break;
    }
    default:
        break;
    }
}

QWidget *PresetTableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    switch (index.column())
    {
    case PresetColumnNumber:
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
    default:
        return nullptr;
    }
    return nullptr;
}

void PresetTableItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}
