#ifndef LIVEVIEWOCCUPANCYSETTING_H
#define LIVEVIEWOCCUPANCYSETTING_H

#include "BaseShadowDialog.h"

namespace Ui {
class LiveViewOccupancySetting;
}

class LiveViewOccupancySetting : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit LiveViewOccupancySetting(QWidget *parent = 0);
    ~LiveViewOccupancySetting();

    void show();

    int currentDisplayState() const;
    int currentGroup() const;

private:
    bool checkDisplay(int display);

private slots:
    void onLanguageChanged();

    void onComboBoxGroupIndexSet(int index);
    void onDisplayStateChanged();

    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::LiveViewOccupancySetting *ui;

    int m_lastGroup = -1;
    QMap<int, int> m_displayMap;
};

#endif // LIVEVIEWOCCUPANCYSETTING_H
