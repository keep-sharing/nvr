#ifndef PRESETWIDGET_H
#define PRESETWIDGET_H

#include "MsWidget.h"

extern "C" {
#include "msg.h"
}

class QTreeWidgetItem;
class PresetItemDelegate;

namespace Ui {
class PresetWidget;
}

class PresetWidget : public MsWidget {
    Q_OBJECT

public:
    explicit PresetWidget(QWidget *parent = 0);
    ~PresetWidget();

    void initializeData(const resp_ptz_preset *preset_array, int count = PRESET_MAX);
    void clear();

private:
    void setFisheyePresetName(int index, const QString &name);

signals:
    //index: 0-254
    void sendPresetControl(int action, int index, const QString &name);

private slots:
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void itemButtonClicked(int row, int index);

private:
    Ui::PresetWidget *ui;

    PresetItemDelegate *m_itemDelegate;
};

#endif // PRESETWIDGET_H
