#ifndef PTZLIMITSPANEL_H
#define PTZLIMITSPANEL_H

#include <QWidget>

extern "C" {
#include "msg.h"
}

class MessageReceive;
class QTreeWidgetItem;
class PresetItemDelegate;

namespace Ui {
class PTZLimitsPanel;
}

class PTZLimitsPanel : public QWidget {
    Q_OBJECT

public:
    explicit PTZLimitsPanel(QWidget *parent = nullptr);
    ~PTZLimitsPanel();

    void initializeData(const int mask, const int mode, const int supportSpeed);

signals:
    void setLimitControl(PTZ_LIMITS_TYPE type, int value);

public slots:
    void onLanguageChanged();

private slots:
    void itemButtonClicked(int row, int index);
    void on_pushButtonClearAll_clicked();

private:
    Ui::PTZLimitsPanel *ui;
    int m_mode = 0;
    int m_count = 0;
    PresetItemDelegate *m_itemDelegate;
};

#endif // PTZLIMITSPANEL_H
