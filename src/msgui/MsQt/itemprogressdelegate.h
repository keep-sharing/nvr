#ifndef ITEMPROGRESSDELEGATE_H
#define ITEMPROGRESSDELEGATE_H

#include "QStyledItemDelegate"

const int ItemProgressTextRole = Qt::UserRole + 30;
const int ItemProgressValueRole = Qt::UserRole + 31;
const int ItemProgressErrorRole = Qt::UserRole + 32;

class ItemProgressDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ItemProgressDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:

public slots:
};

#endif // ITEMPROGRESSDELEGATE_H
