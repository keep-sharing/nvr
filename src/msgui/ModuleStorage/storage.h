#ifndef STORAGE_H
#define STORAGE_H

#include "basesetting.h"

#include <QMap>

class AbstractSettingPage;

namespace Ui {
class Storage;
}

class Storage : public BaseSetting {
    Q_OBJECT

public:
    enum ItemType {
        ItemVideoRecord,
        ItemSnapShot,
        ItemGeneralSettings,
        ItemDisk,
        ItemRAID,
        ItemStorageMode,
        ItemAutoBackup,
        ItemNone
    };

    explicit Storage(QWidget *parent = 0);
    ~Storage();

    void dealMessage(MessageReceive *message) override;
    QList<SettingItemInfo> itemList() override;
    void setCurrentItem(int item_id) override;
    int currentItem() override;

private:
    Ui::Storage *ui;

    ItemType m_currentItemType = ItemNone;
    QMap<ItemType, AbstractSettingPage *> m_pageMap;
};

#endif // STORAGE_H
