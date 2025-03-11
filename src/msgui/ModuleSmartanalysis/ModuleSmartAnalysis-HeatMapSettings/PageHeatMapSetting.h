#ifndef HEATMAPSETTING_H
#define HEATMAPSETTING_H

#include "AbstractSettingPage.h"

namespace Ui {
class HeatMapSetting;
}

class PageHeatMapSetting : public AbstractSettingPage
{
    Q_OBJECT
    enum SETTINGS_TAB {
        TAB_NONE,
        TAB_HEAT_MAP_SETTING,
        TAB_HEAT_MAP_AUTO_BACKUP_SETTING
    };

public:
    explicit PageHeatMapSetting(QWidget *parent = nullptr);
    ~PageHeatMapSetting() override;
    void initializeData() override;

private slots:
    void onTabClicked(int index);

private:
    Ui::HeatMapSetting *ui;
};

#endif // HEATMAPSETTING_H
