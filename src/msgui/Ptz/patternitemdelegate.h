#ifndef PATTERNITEMDELEGATE_H
#define PATTERNITEMDELEGATE_H

#include <QStyledItemDelegate>

const int ItemPatternRole = Qt::UserRole + 10;

class PatternItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum ItemType
    {
        ItemDisable,        //未设置的预置点
        ItemEnable,         //已设置的预置点
        ItemPlaying,
        ItemRecording
    };

    explicit PatternItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void buttonClicked(int row, int index);
};

#endif // PATTERNITEMDELEGATE_H
