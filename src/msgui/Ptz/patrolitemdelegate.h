#ifndef PATROLITEMDELEGATE_H
#define PATROLITEMDELEGATE_H

#include <QStyledItemDelegate>

const int ItemPatrolRole = Qt::UserRole + 10;

class PatrolItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum ItemType
    {
        ItemDisable,        //未设置的预置点
        ItemEnable,         //已设置的预置点
        ItemPlaying
    };

    explicit PatrolItemDelegate(QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void buttonClicked(int row, int index);

private:
};

#endif // PATROLITEMDELEGATE_H
