#ifndef PAGEMAINTENANCESETTINGS_H
#define PAGEMAINTENANCESETTINGS_H

#include "AbstractSettingPage.h"

namespace Ui {
class PageMaintenanceSettings;
}

class PageMaintenanceSettings : public AbstractSettingPage {
    Q_OBJECT

public:
    enum SETTINGS_TAB {
        TAB_NONE,
        TAB_LOCAL_UPGRADE,
        TAB_ONLINE_UPGRADE,
        TAB_CONFIGURATION,
        TAB_AUTO_REBOOT,
        TAB_RESET,
        TAB_DIAGNOSIS,
        TAB_COPYRIGHT
    };

    explicit PageMaintenanceSettings(QWidget *parent = nullptr);
    ~PageMaintenanceSettings();

    void initializeData() override;

private slots:
    void onLanguageChanged();

    void onTabBarClicked(int index);

private:
    Ui::PageMaintenanceSettings *ui;
};

#endif // PAGEMAINTENANCESETTINGS_H
