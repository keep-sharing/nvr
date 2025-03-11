#include "itemcheckedbuttondelegate.h"
#include "itemicondelegate.h"
#include <QPainter>
ItemCheckedButtonDelegate::ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonType type, QObject *parent) :
    QStyledItemDelegate(parent),
    m_type(type)
{
    switch (type) {
    case ButtonCheckBox:
        m_uncheckedPixmap = QPixmap(":/common/common/checkbox-unchecked.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_checkedPixmap = QPixmap(":/common/common/checkbox-checked.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_checkedDisabledPixmap = QPixmap(":/common/common/checkbox-checked-disable.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_uncheckedDisabledPixmap = QPixmap(":/common/common/checkbox-unchecked-disable.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        break;
    case ButtonLock:
        m_uncheckedPixmap = QPixmap(":/retrieve/retrieve/unlock.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_checkedPixmap = QPixmap(":/retrieve/retrieve/lock.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        break;
    case ButtonSwitch:
        m_uncheckedPixmap = QPixmap(":/common/common/switch_off.png").scaled(40, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_checkedPixmap = QPixmap(":/common/common/switch_on.png").scaled(40, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        break;
    case ButtonPlay:
        m_uncheckedPixmap = QPixmap(":/retrieve/retrieve/audioPlay.png").scaled(20, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_checkedPixmap = QPixmap(":/retrieve/retrieve/audioStop.png").scaled(20, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        break;
    default:
        break;
    }
}

void ItemCheckedButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QStyledItemDelegate::paint(painter, option, index);
    painter->restore();

    if (!index.data().toString().isEmpty())
    {
        return;
    }

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
    } else {
        QRect rc = option.rect;
        rc.setSize(m_uncheckedPixmap.size());
        rc.moveCenter(option.rect.center());
        int enabled = index.data(ItemEnabledRole).toInt();
        if (enabled == -12345) {
            bool checked = index.data(ItemCheckedRole).toBool();
            if (checked) {
                painter->drawPixmap(rc, m_checkedDisabledPixmap);
            }
            else {
                painter->drawPixmap(rc, m_uncheckedDisabledPixmap);
            }
        }
        else {
            bool checked = index.data(ItemCheckedRole).toBool();
            if (checked) {
                painter->drawPixmap(rc, m_checkedPixmap);
            }
            else {
                painter->drawPixmap(rc, m_uncheckedPixmap);
            }
        }
    }

}
