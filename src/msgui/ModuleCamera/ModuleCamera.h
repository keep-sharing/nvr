#ifndef MODULECAMERA_H
#define MODULECAMERA_H

#include "basesetting.h"
#include <QMap>

class AbstractCameraPage;

namespace Ui {
class Camera;
}

class ModuleCamera : public BaseSetting
{
    Q_OBJECT

public:
    enum ItemType
    {
        ItemManagenment,
        ItemDeviceSearch,
        ItemPtzConfiguration,
        ItemImage,
        ItemAudio,
        ItemAdvanced,
        ItemCameraMaintenance,
        ItemNone
    };

    explicit ModuleCamera(QWidget *parent = 0);
    ~ModuleCamera();

    void dealMessage(MessageReceive *message) override;
    QList<SettingItemInfo> itemList() override;
    void setCurrentItem(int item_id) override;
    int currentItem() override;

    bool isCloseable() override;
    bool isChangeable() override;
    bool canAutoLogout() override;

private:
    Ui::Camera *ui;

    ItemType m_currentItemType = ItemNone;
    QMap<ItemType, AbstractCameraPage *> m_pageMap;
};

#endif // MODULECAMERA_H
