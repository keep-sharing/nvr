#ifndef PATROLEDITDELEGATE_H
#define PATROLEDITDELEGATE_H

#include <QStyledItemDelegate>

class PatrolEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit PatrolEditDelegate(QObject *parent = nullptr);
    ~PatrolEditDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setChannel(int channel);

signals:
    void activated(int row);

private slots:
    //
    void onCommitData();

private:
    mutable int m_currentRow = -1;
    mutable int m_channel = -1;
};

#endif // PATROLEDITDELEGATE_H
