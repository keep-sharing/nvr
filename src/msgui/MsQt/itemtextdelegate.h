#ifndef ITEMTEXTDELEGATE_H
#define ITEMTEXTDELEGATE_H

#include <QStyledItemDelegate>

class ItemTextDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum ItemRole
    {
        TextColorRole = Qt::UserRole + 20
    };

    ItemTextDelegate(QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // ITEMTEXTDELEGATE_H
