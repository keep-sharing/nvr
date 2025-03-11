#ifndef ITEMCHECKEDBUTTONDELEGATE_H
#define ITEMCHECKEDBUTTONDELEGATE_H

#include "baseitemdelegate.h"

class ItemCheckedButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    enum ButtonState
    {
        StateUnchecked,
        StateChecked,
        StateDisabledUnchecked,
        StateDisabledChecked
    };

public:
    enum ButtonType
    {
        ButtonCheckBox,
        ButtonLock,
        ButtonSwitch,
        ButtonPlay,
        ButtonNone
    };

    explicit ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonType type, QObject *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:

public slots:

private:
    ButtonState m_state = StateUnchecked;
    ButtonType m_type = ButtonNone;

    bool m_checked = false;
    QPixmap m_uncheckedPixmap;
    QPixmap m_checkedPixmap;
    QPixmap m_checkedDisabledPixmap;
    QPixmap m_uncheckedDisabledPixmap;
};

#endif // ITEMCHECKEDBUTTONDELEGATE_H
