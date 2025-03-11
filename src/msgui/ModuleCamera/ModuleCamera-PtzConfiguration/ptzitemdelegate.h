#ifndef PTZITEMDELEGATE_H
#define PTZITEMDELEGATE_H

#include <QStyledItemDelegate>

class PtzItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum ItemType
    {
        PresetItem,
        PatrolItem,
        PatternItem
    };
    enum ItemState
    {
        ItemDisable,        //未设置的预置点
        ItemEnable,         //已设置的预置点
        ItemValidDefault,   //有效默认预置点，无法修改和删除，只能调用
        ItemInvalidDefault, //无效的默认预置点，无任何功能按钮
        ItemPatrolPlaying,  //
        ItemPatternPlaying,
        ItemPatternRecording
    };
    enum ItemRole
    {
        ItemStateRole = Qt::UserRole + 10
    };

    explicit PtzItemDelegate(ItemType type, QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void buttonClicked(int row, int index);

public slots:

private:
    ItemType m_itemType;
};

#endif // PTZITEMDELEGATE_H
