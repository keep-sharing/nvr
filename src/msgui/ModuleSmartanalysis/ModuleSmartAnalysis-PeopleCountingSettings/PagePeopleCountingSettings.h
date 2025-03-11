#ifndef PAGEPEOPLECOUNTINGSETTINGS_H
#define PAGEPEOPLECOUNTINGSETTINGS_H

#include "AbstractSettingPage.h"

namespace Ui {
class PagePeopleCountingSettings;
}

class PagePeopleCountingSettings : public AbstractSettingPage {
    Q_OBJECT

    enum SETTINGS_TAB {
        TAB_NONE,
        TAB_PEOPLE_COUNTING_SETTINGS,
        TAB_OCCUPANCY_LIVE_VIEW_SETTINGS,
        TAB_REGIONAL_PEOPLE_COUNTING_SETTINGS,
        TAB_REPORT_AUTO_BACKUP_SETTINGS
    };

public:
    explicit PagePeopleCountingSettings(QWidget *parent = 0);
    ~PagePeopleCountingSettings();

    void initializeData() override;

private slots:
    void onTabClicked(int index);

private:
    Ui::PagePeopleCountingSettings *ui;
};

#endif // PAGEPEOPLECOUNTINGSETTINGS_H
