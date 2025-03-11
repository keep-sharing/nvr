#ifndef MODULESTATUS_H
#define MODULESTATUS_H

#include "AbstractSettingPage.h"
#include "basesetting.h"

namespace Ui {
class Status;
}

class ModuleStatus : public BaseSetting {
    Q_OBJECT

public:
    enum ItemType {
        ItemDevice = 0,
        ItemNetwork,
        ItemCamera,
        ItemDisk,
        ItemEvent,
        ItemGroup,
        ItemOnlineUser,
        ItemPacketCapture,
        ItemLog,
        ItemNone
    };

    explicit ModuleStatus(QWidget *parent = 0);
    ~ModuleStatus();

    void dealMessage(MessageReceive *message) override;
    QList<SettingItemInfo> itemList() override;
    void setCurrentItem(int item_id) override;
    int currentItem() override;

    bool canAutoLogout() override;

private:
    Ui::Status *ui;

    ItemType m_currentItemType = ItemNone;
    QMap<ItemType, AbstractSettingPage *> m_pageMap;
};

#endif // MODULESTATUS_H
