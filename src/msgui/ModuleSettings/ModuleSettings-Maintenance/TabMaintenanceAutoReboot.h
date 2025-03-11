#ifndef TABMAINTENANCEAUTOREBOOT_H
#define TABMAINTENANCEAUTOREBOOT_H

#include "AbstractSettingTab.h"
#include <QWidget>

extern "C" {
#include "msdb.h"
}

namespace Ui {
class TabMaintenanceAutoReboot;
}

class TabMaintenanceAutoReboot : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabMaintenanceAutoReboot(QWidget *parent = nullptr);
    ~TabMaintenanceAutoReboot();

    void initializeData() override;

private slots:
    void onLanguageChanged();

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::TabMaintenanceAutoReboot *ui;

    reboot_conf m_reboot_conf;
};

#endif // TABMAINTENANCEAUTOREBOOT_H
