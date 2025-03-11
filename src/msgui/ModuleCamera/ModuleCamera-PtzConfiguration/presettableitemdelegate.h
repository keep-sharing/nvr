#ifndef PRESETTABLEITEMDELEGATE_H
#define PRESETTABLEITEMDELEGATE_H

#include <QStyledItemDelegate>

class PresetTableItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum PresetColumn
    {
        PresetColumnCheck,
        PresetColumnNumber,
        PresetColumnPlay,
        PresetColumnSave,
        PresetColumnDelete
    };
    enum ItemType
    {
        ItemDisable,        //未设置的预置点
        ItemNormal,         //已设置的预置点
        ItemValidDefault,   //有效默认预置点，无法修改和删除，只能调用
        ItemInvalidDefault  //无效的默认预置点，无任何功能按钮
    };
    enum ItemRole
    {
        ItemStateRole = Qt::UserRole + 10
    };

    explicit PresetTableItemDelegate(QObject *parent = nullptr);

    void setEditable(bool enable);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:

public slots:

private:
    bool m_editable = true;
};

#endif // PRESETTABLEITEMDELEGATE_H
