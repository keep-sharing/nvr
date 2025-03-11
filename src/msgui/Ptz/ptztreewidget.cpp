#include "ptztreewidget.h"
#include <QHeaderView>

PtzTreeWidget::PtzTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    header()->setVisible(false);
    header()->setStretchLastSection(true);
    setIndentation(0);
}

void PtzTreeWidget::setPresetName(int row, const QString &name)
{
    QTreeWidgetItem *item = topLevelItem(row);
    if (item)
    {
        item->setText(1, name);
    }
}

QString PtzTreeWidget::presetName(int row)
{
    QString name;
    QTreeWidgetItem *item = topLevelItem(row);
    if (item)
    {
        name = item->text(1);
    }
    return name;
}

void PtzTreeWidget::setPresetState(int row, PresetItemDelegate::ItemType type, bool refresh)
{
    QTreeWidgetItem *item = topLevelItem(row);
    if (item)
    {
        item->setData(0, ItemTypeRole, type);
        item->setData(1, ItemTypeRole, type);
        item->setData(2, ItemTypeRole, type);
        if (refresh)
        {
            closePersistentEditor(item, 2);
            openPersistentEditor(item, 2);
        }
    }
}

void PtzTreeWidget::setPatrolState(int row, PatrolItemDelegate::ItemType type, bool refresh)
{
    QTreeWidgetItem *item = topLevelItem(row);
    if (item)
    {
        item->setData(0, ItemPatrolRole, type);
        item->setData(1, ItemPatrolRole, type);
        item->setData(2, ItemPatrolRole, type);
        if (refresh)
        {
            closePersistentEditor(item, 2);
            openPersistentEditor(item, 2);
        }
    }
}

PatrolItemDelegate::ItemType PtzTreeWidget::patrolState(int row)
{
    PatrolItemDelegate::ItemType type = PatrolItemDelegate::ItemEnable;
    QTreeWidgetItem *item = topLevelItem(row);
    if (item)
    {
        type = static_cast<PatrolItemDelegate::ItemType>(item->data(0, ItemPatrolRole).toInt());
    }
    return type;
}

void PtzTreeWidget::setPatternState(int row, PatternItemDelegate::ItemType type, bool refresh)
{
    QTreeWidgetItem *item = topLevelItem(row);
    if (item)
    {
        item->setData(0, ItemPatternRole, type);
        item->setData(1, ItemPatternRole, type);
        item->setData(2, ItemPatternRole, type);
        if (refresh)
        {
            closePersistentEditor(item, 2);
            openPersistentEditor(item, 2);
        }
    }
}

PatternItemDelegate::ItemType PtzTreeWidget::patternState(int row)
{
    PatternItemDelegate::ItemType type = PatternItemDelegate::ItemEnable;
    QTreeWidgetItem *item = topLevelItem(row);
    if (item)
    {
        type = static_cast<PatternItemDelegate::ItemType>(item->data(0, ItemPatternRole).toInt());
    }
    return type;
}

void PtzTreeWidget::setPtzItemState(int row, PtzItemDelegate::ItemState state, bool refresh)
{
    QTreeWidgetItem *item = topLevelItem(row);
    if (item)
    {
        item->setData(0, PtzItemDelegate::ItemStateRole, state);
        item->setData(1, PtzItemDelegate::ItemStateRole, state);
        item->setData(2, PtzItemDelegate::ItemStateRole, state);
        if (refresh)
        {
            closePersistentEditor(item, 2);
            openPersistentEditor(item, 2);
        }
    }
}

PtzItemDelegate::ItemState PtzTreeWidget::ptzItemState(int row)
{
    PtzItemDelegate::ItemState state = PtzItemDelegate::ItemEnable;
    QTreeWidgetItem *item = topLevelItem(row);
    if (item)
    {
        state = static_cast<PtzItemDelegate::ItemState>(item->data(0, PtzItemDelegate::ItemStateRole).toInt());
    }
    return state;
}

