#include "ModuleStatus.h"
#include "ui_ModuleStatus.h"
#include "PageCameraStatus.h"
#include "PageDeviceInformation.h"
#include "PageDiskStatus.h"
#include "PageEventStatus.h"
#include "PageGroupStatus.h"
#include "PageLogStatus.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "PageNetworkStatus.h"
#include "PageOnlineUser.h"
#include "PagePacketCapture.h"
#include <QtDebug>

ModuleStatus::ModuleStatus(QWidget *parent)
    : BaseSetting(BaseSetting::TypeStatus, parent)
    , ui(new Ui::Status)
{
    ui->setupUi(this);
}

ModuleStatus::~ModuleStatus()
{
    delete ui;
}

void ModuleStatus::dealMessage(MessageReceive *message)
{
    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    }
    if (page) {
        page->dealMessage(message);
    }
}

QList<SettingItemInfo> ModuleStatus::itemList()
{
    QList<SettingItemInfo> list;
    //热备模式移除Group Status Event Status
    bool isSlaveMode = qMsNvr->isSlaveMode();
    list.append(SettingItemInfo(GET_TEXT("DEVICEINFO/60000", "Device Information"), ItemDevice, PERM_MODE_STATUS, PERM_STATUS_NONE ));
    list.append(SettingItemInfo(GET_TEXT("NETWORKSTATUS/61000", "Network Status"), ItemNetwork, PERM_MODE_STATUS, PERM_STATUS_NONE));
    list.append(SettingItemInfo(GET_TEXT("CAMERASTATUS/62000", "Camera Status"), ItemCamera, PERM_MODE_STATUS, PERM_STATUS_NONE));
    list.append(SettingItemInfo(GET_TEXT("CAMERASTATUS/62008", "Disk Status"), ItemDisk, PERM_MODE_STATUS, PERM_STATUS_NONE));
    list.append(SettingItemInfo(GET_TEXT("EVENTSTATUS/63000", "Event Status"), ItemEvent, PERM_MODE_STATUS, PERM_STATUS_NONE, !isSlaveMode));
    list.append(SettingItemInfo(GET_TEXT("GROUPSTATUS/65000", "Group Status"), ItemGroup, PERM_MODE_STATUS, PERM_STATUS_NONE, !isSlaveMode));
    list.append(SettingItemInfo(GET_TEXT("ONLINEUSER/74100", "Online User"), ItemOnlineUser, PERM_MODE_STATUS, PERM_STATUS_NONE));
    list.append(SettingItemInfo(GET_TEXT("PACKETCAPTURE/109000", "Packet Capture Tool"), ItemPacketCapture, PERM_MODE_STATUS, PERM_STATUS_NONE));
    list.append(SettingItemInfo(GET_TEXT("LOG/64000", "Logs"), ItemLog, PERM_MODE_STATUS, PERM_STATUS_NONE));
    return list;
}

void ModuleStatus::setCurrentItem(int item_id)
{
    //
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractSettingPage *page = m_pageMap.value(m_currentItemType);
        ui->gridLayout->removeWidget(page);
        page->hide();
    }

    //
    m_currentItemType = static_cast<ItemType>(item_id);
    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    } else {
        switch (m_currentItemType) {
        case ItemDevice:
            page = new PageDeviceInformation(this);
            break;
        case ItemNetwork:
            page = new PageNetworkStatus(this);
            break;
        case ItemCamera:
            page = new PageCameraStatus(this);
            break;
        case ItemDisk:
            page = new PageDiskStatus(this);
            break;
        case ItemEvent:
            page = new PageEventStatus(this);
            break;
        case ItemGroup:
            page = new PageGroupStatus(this);
            break;
        case ItemOnlineUser:
            page = new PageOnlineUser(this);
            break;
        case ItemPacketCapture:
            page = new PagePacketCapture(this);
            break;
        case ItemLog:
            page = new PageLogStatus(this);
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

int ModuleStatus::currentItem()
{
    return m_currentItemType;
}

bool ModuleStatus::canAutoLogout()
{
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractSettingPage *page = m_pageMap.value(m_currentItemType);
        return page->canAutoLogout();
    } else {
        return true;
    }
}
