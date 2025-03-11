#ifndef PTZTREEWIDGET_H
#define PTZTREEWIDGET_H

#include <QTreeWidget>
#include "presetitemdelegate.h"
#include "patrolitemdelegate.h"
#include "patternitemdelegate.h"
#include "ptzitemdelegate.h"

class PtzTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit PtzTreeWidget(QWidget *parent = 0);

    void setPresetName(int row, const QString &name);
    QString presetName(int row);
    void setPresetState(int row, PresetItemDelegate::ItemType type, bool refresh = false);

    void setPatrolState(int row, PatrolItemDelegate::ItemType type, bool refresh = false);
    PatrolItemDelegate::ItemType patrolState(int row);

    void setPatternState(int row, PatternItemDelegate::ItemType type, bool refresh = false);
    PatternItemDelegate::ItemType patternState(int row);

    void setPtzItemState(int row, PtzItemDelegate::ItemState state, bool refresh = false);
    PtzItemDelegate::ItemState ptzItemState(int row);

signals:

public slots:
};

#endif // PTZTREEWIDGET_H
