#ifndef PRESETITEMDELEGATE_H
#define PRESETITEMDELEGATE_H

#include <QStyledItemDelegate>

const int ItemTypeRole = Qt::UserRole + 10;

class PresetItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    enum Theme {
        WhiteTheme,
        GrayTheme
    };

    enum ItemType {
        ItemDisable, //未设置的预置点
        ItemNormal, //已设置的预置点
        ItemValidDefault, //有效默认预置点，无法修改和删除，只能调用
        ItemLimitsDisable, //PTZ Limits未设置的预置点
        ItemLimits, //PTZ Limits已设置的预置点
        ItemInvalidDefault //无效的默认预置点，无任何功能按钮
    };

    explicit PresetItemDelegate(QObject *parent);

    void setTheme(PresetItemDelegate::Theme theme);
    void setEditable(bool enable);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void buttonClicked(int row, int index);

public slots:

private:
    Theme m_theme = WhiteTheme;

    bool m_editable = true;
};

#endif // PRESETITEMDELEGATE_H
