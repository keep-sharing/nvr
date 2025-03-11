#ifndef MODULERETRIEVE_H
#define MODULERETRIEVE_H

#include "basesetting.h"

class AbstractSettingPage;

namespace Ui {
class Retrieve;
}

class ModuleRetrieve : public BaseSetting
{
    Q_OBJECT

public:
    enum ItemType
    {
        ItemCommonBackup = 0,
        ItemEventBackup,
        ItemPictureBackup,
        ItemNone
    };

    explicit ModuleRetrieve(QWidget *parent = 0);
    ~ModuleRetrieve();

    NetworkResult dealNetworkCommond(const QString &commond) override;

    void dealMessage(MessageReceive *message) override;
    QList<SettingItemInfo> itemList() override;
    void setCurrentItem(int item_id) override;

    bool isCloseable() override;
    void closeSetting() override;

    bool canAutoLogout() override;
    int currentItem() override;

protected:
    bool isAddToVisibleList() override;

private:
    Ui::Retrieve *ui;

    ItemType m_currentItemType = ItemNone;
    QMap<ItemType, AbstractSettingPage *> m_pageMap;
};

#endif // MODULERETRIEVE_H
