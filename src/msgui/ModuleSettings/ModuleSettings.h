#ifndef MODULESETTINGS_H
#define MODULESETTINGS_H

#include "basesetting.h"

#include <QMap>

class AbstractSettingPage;
class GeneralSetting;
class NetworkSetting;
class HolidaySetting;
class PageUserSettings;
class PageMaintenanceSettings;

namespace Ui {
class Setting;
}

class ModuleSettings : public BaseSetting {
    Q_OBJECT

public:
    enum ItemType {
        ItemGeneral,
        ItemLayout,
        ItemNetwork,
        ItemAudio,
        ItemHoliday,
        ItemUser,
        ItemAccessFilter,
        ItemMaintenance,
        ItemHotSpare,
        ItemNone
    };

    explicit ModuleSettings(QWidget *parent = 0);
    ~ModuleSettings();

    void dealMessage(MessageReceive *message) override;
    QList<SettingItemInfo> itemList() override;
    void setCurrentItem(int item_id) override;
    int currentItem() override;

    bool canAutoLogout() override;

private:
    Ui::Setting *ui;

    ItemType m_currentItemType = ItemNone;
    QMap<ItemType, AbstractSettingPage *> m_pageMap;
};

#endif // MODULESETTINGS_H
