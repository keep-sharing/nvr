#include "storage.h"
#include "ui_storage.h"
#include "AbstractSettingPage.h"
#include "autobackup.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "PageRaidSetting.h"
#include "recordschedule.h"
#include "snapshotschedule.h"
#include "storagemode.h"
#include "MyDebug.h"
#include "PageDiskManagement.h"

Storage::Storage(QWidget *parent)
    : BaseSetting(BaseSetting::TypeStorage, parent)
    , ui(new Ui::Storage)
{
    ui->setupUi(this);
}

Storage::~Storage()
{
    delete ui;
}

void Storage::dealMessage(MessageReceive *message)
{
    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    }
    if (page) {
        page->dealMessage(message);
    }
}

QList<SettingItemInfo> Storage::itemList()
{
    QList<SettingItemInfo> list;
    bool snapshotEnable = qMsNvr->isSupportEventSnapshot();
    //热备模式移除Record Schedule、Record Settings、Group
    if (qMsNvr->isSlaveMode()) {
        list.append(SettingItemInfo(GET_TEXT("RECORDADVANCE/91026", "Video Record"), ItemVideoRecord, PERM_MODE_STORAGE, PERM_STORAGE_NONE, false));
        list.append(SettingItemInfo(GET_TEXT("SNAPSHOT/95000", "Snapshot"), ItemSnapShot, PERM_MODE_STORAGE, PERM_STORAGE_NONE, false));
        list.append(SettingItemInfo(GET_TEXT("STORAGEMODE/95110", "Disk Management"), ItemDisk, PERM_MODE_STORAGE, PERM_STORAGE_NONE));
        list.append(SettingItemInfo(GET_TEXT("DISKMANAGE/72068", "RAID"), ItemRAID, PERM_MODE_STORAGE, PERM_STORAGE_NONE, qMsNvr->isSupportRaid()));
        list.append(SettingItemInfo(GET_TEXT("STORAGEMODE/95100", "Storage Mode"), ItemStorageMode, PERM_MODE_STORAGE, PERM_STORAGE_NONE, false));
        list.append(SettingItemInfo(GET_TEXT("AUTOBACKUP/95200", "Auto Backup"), ItemAutoBackup, PERM_MODE_STORAGE, PERM_STORAGE_NONE, false));
    } else {
        list.append(SettingItemInfo(GET_TEXT("RECORDADVANCE/91026", "Video Record"), ItemVideoRecord, PERM_MODE_STORAGE, PERM_STORAGE_NONE));
        list.append(SettingItemInfo(GET_TEXT("SNAPSHOT/95000", "Snapshot"), ItemSnapShot, PERM_MODE_STORAGE, PERM_STORAGE_NONE, snapshotEnable));
        list.append(SettingItemInfo(GET_TEXT("STORAGEMODE/95110", "Disk Management"), ItemDisk, PERM_MODE_STORAGE, PERM_STORAGE_NONE));
        list.append(SettingItemInfo(GET_TEXT("DISKMANAGE/72068", "RAID"), ItemRAID, PERM_MODE_STORAGE, PERM_STORAGE_NONE, qMsNvr->isSupportRaid()));
        list.append(SettingItemInfo(GET_TEXT("STORAGEMODE/95100", "Storage Mode"), ItemStorageMode, PERM_MODE_STORAGE, PERM_STORAGE_NONE));
        list.append(SettingItemInfo(GET_TEXT("AUTOBACKUP/95200", "Auto Backup"), ItemAutoBackup, PERM_MODE_STORAGE, PERM_STORAGE_NONE));
    }
    return list;
}

void Storage::setCurrentItem(int item_id)
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
        case ItemVideoRecord:
            page = new RecordSchedule(this);
            break;
        case ItemSnapShot:
            page = new SnapshotSchedule(this);
            break;
        case ItemDisk:
            page = new PageDiskManagement(this);
            break;
        case ItemRAID:
            page = new PageRaidSetting(this);
            break;
        case ItemStorageMode:
            page = new StorageMode(this);
            break;
        case ItemAutoBackup:
            page = new AutoBackup(this);
            break;
        default:
            break;
        }
        if (page) {
            connect(page, SIGNAL(sig_back()), this, SIGNAL(sig_close()));
            connect(page, SIGNAL(updateVideoGeometry()), this, SIGNAL(updateVideoGeometry()));
            m_pageMap.insert(m_currentItemType, page);
        }
    }
    if (page) {
        ui->gridLayout->addWidget(page);
        page->show();
        page->initializeData();
    }
}

int Storage::currentItem()
{
    return m_currentItemType;
}
