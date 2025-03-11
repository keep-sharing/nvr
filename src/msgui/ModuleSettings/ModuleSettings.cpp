#include "ModuleSettings.h"
#include "ui_ModuleSettings.h"
#include "AccessFilter.h"
#include "AudioPage.h"
#include "GeneralSetting.h"
#include "HolidaySetting.h"
#include "HotSpare.h"
#include "LayoutSettings.h"
#include "PageMaintenanceSettings.h"
#include "NetworkSetting.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "msuser.h"
#include "PageUserSettings.h"
#include "PageUserSettingOperator.h"
#include <QtDebug>

ModuleSettings::ModuleSettings(QWidget *parent)
    : BaseSetting(BaseSetting::TypeSettings, parent)
    , ui(new Ui::Setting)
{
    ui->setupUi(this);
}

ModuleSettings::~ModuleSettings()
{
    delete ui;
}

void ModuleSettings::dealMessage(MessageReceive *message)
{
    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    }
    if (page) {
        page->dealMessage(message);
    }
}

QList<SettingItemInfo> ModuleSettings::itemList()
{
    QList<SettingItemInfo> list;
    //热备模式移除Holiday、Auto Reboot、Access Filter
    bool isSlaveMode = qMsNvr->isSlaveMode();
    list.append(SettingItemInfo(GET_TEXT("SYSTEMGENERAL/70001", "General"), ItemGeneral, PERM_MODE_SETTINGS, PERM_SETTINGS_NONE));
    list.append(SettingItemInfo(GET_TEXT("MENU/10001", "Layout"), ItemLayout, PERM_MODE_SETTINGS, PERM_SETTINGS_NONE));
    list.append(SettingItemInfo(GET_TEXT("SYSTEMGENERAL/70002", "Network"), ItemNetwork, PERM_MODE_SETTINGS, PERM_SETTINGS_NONE));
    //是否添加音频
    const struct device_info &sys_info = qMsNvr->deviceInfo();
    if  (sys_info.max_audio_out != 0) {
        list.append(SettingItemInfo(GET_TEXT("AUDIOFILE/117000", "Audio File Manager"), ItemAudio, PERM_MODE_SETTINGS, PERM_SETTINGS_NONE));
    }
    list.append(SettingItemInfo(GET_TEXT("COMMON/1031", "Holiday"), ItemHoliday, PERM_MODE_SETTINGS, PERM_SETTINGS_NONE, !isSlaveMode));
    list.append(SettingItemInfo(GET_TEXT("LOG/64008", "User"), ItemUser, PERM_MODE_SETTINGS, PERM_SETTINGS_ALL));
    list.append(SettingItemInfo(GET_TEXT("ACCESSFILTER/81000", "Access Filter"), ItemAccessFilter, PERM_MODE_SETTINGS, PERM_SETTINGS_NONE, !isSlaveMode));
    list.append(SettingItemInfo(GET_TEXT("SYSTEMGENERAL/70008", "Maintenance"), ItemMaintenance, PERM_MODE_SETTINGS, PERM_SETTINGS_NONE));

    bool hotSpareEnable = true;
    bool is3798 = qMsNvr->is3798();
    bool is3536c = qMsNvr->is3536c();
    double version = qMsNvr->hardwareVersionEx();
    qDebug() << "isSlaveMode:" << isSlaveMode << "is3798:" << is3798 << " version:" << version;
    if (is3798 || is3536c) {
        hotSpareEnable = false;
    }
    if (qMsNvr->isPoe()) {
        hotSpareEnable = false;
    }
    if (!gMsUser.isAdmin()) {
        hotSpareEnable = false;
    }
#if defined(_NT98323_) || defined(_NT98633_)
    hotSpareEnable = false;
#endif
    list.append(SettingItemInfo(GET_TEXT("DISKMANAGE/72064", "Hot Spare"), ItemHotSpare, PERM_MODE_SETTINGS, PERM_SETTINGS_ALL, hotSpareEnable));

    return list;
}

void ModuleSettings::setCurrentItem(int item_id)
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
        case ItemGeneral:
            page = new GeneralSetting(this);
            break;
        case ItemLayout:
            page = new LayoutSettings(this);
            break;
        case ItemNetwork:
            page = new NetworkSetting(this);
            break;
        case ItemAudio:
            page = new AudioPage(this);
            break;
        case ItemHoliday:
            page = new HolidaySetting(this);
            break;
        case ItemUser:
            if (gMsUser.isAdmin()) {
                page = new PageUserSettings(this);
            } else {
                page = new PageUserSettingOperator(this);
            }
            break;
        case ItemAccessFilter:
            page = new AccessFilter(this);
            break;
        case ItemMaintenance:
            page = new PageMaintenanceSettings(this);
            break;
        case ItemHotSpare:
            page = new HotSpare(this);
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

int ModuleSettings::currentItem()
{
    return m_currentItemType;
}

bool ModuleSettings::canAutoLogout()
{
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractSettingPage *page = m_pageMap.value(m_currentItemType);
        return page->canAutoLogout();
    } else {
        return true;
    }
}
