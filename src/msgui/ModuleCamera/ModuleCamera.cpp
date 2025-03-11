#include "ModuleCamera.h"
#include "ui_ModuleCamera.h"
#include "AdvancedSettings.h"
#include "CameraAudio.h"
#include "CameraManagement.h"
#include "DeviceSearch.h"
#include "MsLanguage.h"
#include "TabHeatMapSettings.h"
#include "abstractcamerapage.h"
#include "PageCameraMaintenance.h"
#include "imagesetting.h"
#include "PagePtzConfiguration.h"
#include <QtDebug>

ModuleCamera::ModuleCamera(QWidget *parent)
    : BaseSetting(BaseSetting::TypeCamera, parent)
    , ui(new Ui::Camera)
{
    ui->setupUi(this);
}

ModuleCamera::~ModuleCamera()
{
    delete ui;
}

void ModuleCamera::dealMessage(MessageReceive *message)
{
    AbstractCameraPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    }
    if (page) {
        page->dealMessage(message);
    }
}

QList<SettingItemInfo> ModuleCamera::itemList()
{
    QList<SettingItemInfo> list;
    list.append(SettingItemInfo(GET_TEXT("CHANNELMANAGE/30001", "Camera Management"), ItemManagenment, PERM_MODE_CAMERA, PERM_CAMERA_NONE));
    list.append(SettingItemInfo(GET_TEXT("CHANNELMANAGE/30002", "Device Search"), ItemDeviceSearch, PERM_MODE_CAMERA, PERM_CAMERA_NONE));
    list.append(SettingItemInfo(GET_TEXT("CHANNELMANAGE/30004", "PTZ Configuration"), ItemPtzConfiguration, PERM_MODE_CAMERA, PERM_CAMERA_NONE));
    list.append(SettingItemInfo(GET_TEXT("IMAGE/37000", "Image"), ItemImage, PERM_MODE_CAMERA, PERM_CAMERA_NONE));
    list.append(SettingItemInfo(GET_TEXT("CAMERAAUDIO/107001", "Audio"), ItemAudio, PERM_MODE_CAMERA, PERM_CAMERA_NONE));
    list.append(SettingItemInfo(GET_TEXT("CAMERAADVANCE/106000", "Advanced"), ItemAdvanced, PERM_MODE_CAMERA, PERM_CAMERA_NONE));
    list.append(SettingItemInfo(GET_TEXT("CAMERAMAINTENANCE/38000", "Camera Maintenance"), ItemCameraMaintenance, PERM_MODE_CAMERA, PERM_CAMERA_NONE));
    return list;
}

void ModuleCamera::setCurrentItem(int item_id)
{
    //
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractCameraPage *page = m_pageMap.value(m_currentItemType);
        ui->gridLayout->removeWidget(page);
        page->hide();
    }

    //
    m_currentItemType = static_cast<ItemType>(item_id);
    AbstractCameraPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    } else {
        switch (m_currentItemType) {
        case ItemManagenment:
            page = new CameraManagement(this);
            break;
        case ItemDeviceSearch:
            page = new DeviceSearch(this);
            break;
        case ItemPtzConfiguration:
            page = new PagePtzConfiguration(this);
            break;
        case ItemImage:
            page = new ImageSetting(this);
            break;
        case ItemAudio:
            page = new CameraAudio(this);
            break;
        case ItemAdvanced:
            page = new AdvancedSettings(this);
            break;
        case ItemCameraMaintenance:
            page = new PageCameraMaintenance(this);
            break;
        default:
            break;
        }
        if (page) {
            connect(page, SIGNAL(sig_back()), this, SIGNAL(sig_close()));
            m_pageMap.insert(m_currentItemType, page);
        }
    }
    if (page) {
        ui->gridLayout->addWidget(page);
        page->show();
        page->initializeData();
    }
}

int ModuleCamera::currentItem()
{
    return m_currentItemType;
}

bool ModuleCamera::isCloseable()
{
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractCameraPage *page = m_pageMap.value(m_currentItemType);
        return page->isCloseable();
    }
    return true;
}

bool ModuleCamera::isChangeable()
{
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractCameraPage *page = m_pageMap.value(m_currentItemType);
        return page->isChangeable();
    }
    return true;
}

bool ModuleCamera::canAutoLogout()
{
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractCameraPage *page = m_pageMap.value(m_currentItemType);
        return page->canAutoLogout();
    }
    return true;
}
