#ifndef MASKPRESET_H
#define MASKPRESET_H

#include <QWidget>

extern "C"
{
#include "msg.h"
}

class MessageReceive;
class QTreeWidgetItem;
class PresetItemDelegate;

namespace Ui {
class MaskPreset;
}

class MaskPreset : public QWidget
{
    Q_OBJECT

public:
    explicit MaskPreset(QWidget *parent = nullptr);
    ~MaskPreset();

    void initializeData(const resp_ptz_preset *preset_array, int count = PRESET_MAX);
    void clear();

private:

signals:
    //index: 0-299
    void sendPresetControl(int action, int index, const QString &name);

public slots:
    void onLanguageChanged();

private slots:
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void itemButtonClicked(int row, int index);

private:
    Ui::MaskPreset *ui;

    PresetItemDelegate *m_itemDelegate;
};

#endif // MASKPRESET_H
