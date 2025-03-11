#ifndef ITEMICONDELEGATE_H
#define ITEMICONDELEGATE_H

#include <QStyledItemDelegate>

const int PixmapRole = Qt::UserRole + 40;

class ItemIconDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ItemIconDelegate(QObject *parent = nullptr);
    ~ItemIconDelegate();

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // ITEMICONDELEGATE_H
