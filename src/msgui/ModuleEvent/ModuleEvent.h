#ifndef MODULEEVENT_H
#define MODULEEVENT_H

#include "basesetting.h"

class AbstractSettingPage;

namespace Ui {
class ModuleEvent;
}

class ModuleEvent : public BaseSetting {
    Q_OBJECT

public:
    enum ItemType {
        ItemMotion,
        ItemAudioAlarm,
        ItemVideoLoss,
        ItemAlarmInput,
        ItemAlarmOutput,
        ItemException,
        ItemSmartEvent,
        ItemAnpr,
        ItemNone
    };

    explicit ModuleEvent(QWidget *parent = 0);
    ~ModuleEvent();

    void dealMessage(MessageReceive *message) override;
    QList<SettingItemInfo> itemList() override;
    void setCurrentItem(int item_id) override;
    int currentItem() override;

private:
    Ui::ModuleEvent *ui;

    ItemType m_currentItemType = ItemNone;
    QMap<ItemType, AbstractSettingPage *> m_pageMap;
};

#endif // MODULEEVENT_H
