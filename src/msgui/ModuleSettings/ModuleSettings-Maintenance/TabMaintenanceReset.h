#ifndef TABMAINTENANCERESET_H
#define TABMAINTENANCERESET_H

#include "AbstractSettingTab.h"

namespace Ui {
class TabMaintenanceReset;
}

class TabMaintenanceReset : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabMaintenanceReset(QWidget *parent = nullptr);
    ~TabMaintenanceReset();

    void initializeData() override;

private slots:
    void onLanguageChanged();

    void on_pushButton_reset_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::TabMaintenanceReset *ui;
};

#endif // TABMAINTENANCERESET_H
