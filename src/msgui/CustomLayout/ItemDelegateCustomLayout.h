#ifndef ITEMDELEGATECUSTOMLAYOUT_H
#define ITEMDELEGATECUSTOMLAYOUT_H

#include <QStyledItemDelegate>

const int ItemPixmapRole = Qt::UserRole + 40;

class ItemDelegateCustomLayout : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ItemDelegateCustomLayout(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void editintFinished(int row, int column, const QString &oldText, const QString &newText);
};

#endif // ITEMDELEGATECUSTOMLAYOUT_H
