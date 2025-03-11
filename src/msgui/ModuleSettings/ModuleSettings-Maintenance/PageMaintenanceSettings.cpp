#include "PageMaintenanceSettings.h"
#include "ui_PageMaintenanceSettings.h"
#include "TabMaintenanceAutoReboot.h"
#include "TabMaintenanceConfiguration.h"
#include "TabMaintenanceDiagnosis.h"
#include "TabMaintenanceOnlineUpgrade.h"
#include "TabMaintenanceReset.h"
#include "TabMaintenanceLocalUpgrade.h"
#include "TabCopyrightNotice.h"
#include "MsDevice.h"
#include "MsLanguage.h"

PageMaintenanceSettings::PageMaintenanceSettings(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PageMaintenanceSettings)
{
    ui->setupUi(this);

    //
    ui->tabBar->addTab(GET_TEXT("UPGRADE/75011", "Local Upgrade"), TAB_LOCAL_UPGRADE);
    bool isOEM = qMsNvr->OEMType();
    bool isSlaveMode = qMsNvr->isSlaveMode();
    if (!(isOEM && qMsNvr->deviceInfo().oemupdateonline == 0 && QString(qMsNvr->deviceInfo().company) != QString("Milesight"))) {
        ui->tabBar->addTab(GET_TEXT("UPGRADE/75012", "Online Upgrade"), TAB_ONLINE_UPGRADE);
    }
    if (!isSlaveMode) {
        ui->tabBar->addTab(GET_TEXT("SYSTEMGENERAL/70007", "Import/Export Configuration"), TAB_CONFIGURATION);
        ui->tabBar->addTab(GET_TEXT("AUTOREBOOT/78000", "Auto Reboot"), TAB_AUTO_REBOOT);
    }
    ui->tabBar->addTab(GET_TEXT("COMMON/1057", "Reset"), TAB_RESET);
    ui->tabBar->addTab(GET_TEXT("MAINTENANCE/77005", "Diagnosis Information"), TAB_DIAGNOSIS);
    ui->tabBar->addTab(GET_TEXT("MAINTENANCE/179000", "Copyright Notice"), TAB_COPYRIGHT);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));
}

PageMaintenanceSettings::~PageMaintenanceSettings()
{
    delete ui;
}

void PageMaintenanceSettings::initializeData()
{
    ui->tabBar->setCurrentTab(TAB_LOCAL_UPGRADE);
}

void PageMaintenanceSettings::onLanguageChanged()
{
    ui->tabBar->setTabText(TAB_LOCAL_UPGRADE, GET_TEXT("UPGRADE/75011", "Local Upgrade"));
    ui->tabBar->setTabText(TAB_ONLINE_UPGRADE, GET_TEXT("UPGRADE/75012", "Online Upgrade"));
    ui->tabBar->setTabText(TAB_CONFIGURATION, GET_TEXT("SYSTEMGENERAL/70007", "Import/Export Configuration"));
    ui->tabBar->setTabText(TAB_AUTO_REBOOT, GET_TEXT("AUTOREBOOT/78000", "Auto Reboot"));
    ui->tabBar->setTabText(TAB_RESET, GET_TEXT("COMMON/1057", "Reset"));
    ui->tabBar->setTabText(TAB_DIAGNOSIS, GET_TEXT("MAINTENANCE/77005", "Diagnosis Information"));
    ui->tabBar->setTabText(TAB_COPYRIGHT, GET_TEXT("MAINTENANCE/179000", "Copyright Notice"));
}

void PageMaintenanceSettings::onTabBarClicked(int index)
{
    m_currentTab = index;
    AbstractSettingTab *settingTab = m_tabMap.value(m_currentTab, nullptr);
    if (!settingTab) {
        switch (m_currentTab) {
        case TAB_LOCAL_UPGRADE:
            settingTab = new TabMaintenanceLocalUpgrade(this);
            break;
        case TAB_ONLINE_UPGRADE:
            settingTab = new TabMaintenanceOnlineUpgrade(this);
            break;
        case TAB_CONFIGURATION:
            settingTab = new TabMaintenanceConfiguration(this);
            break;
        case TAB_AUTO_REBOOT:
            settingTab = new TabMaintenanceAutoReboot(this);
            break;
        case TAB_RESET:
            settingTab = new TabMaintenanceReset(this);
            break;
        case TAB_DIAGNOSIS:
            settingTab = new TabMaintenanceDiagnosis(this);
            break;
        case TAB_COPYRIGHT:
            settingTab = new TabCopyrightNotice(this);
            break;
        default:
            break;
        }
        if (settingTab) {
            connect(settingTab, SIGNAL(sig_back()), this, SIGNAL(sig_back()));
            m_tabMap.insert(m_currentTab, settingTab);
        }
    }
    //
    QLayoutItem *item = ui->gridLayoutContainer->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->hide();
        }
        ui->gridLayoutContainer->removeItem(item);
        delete item;
    }
    //
    if (settingTab) {
        ui->gridLayoutContainer->addWidget(settingTab, 0, 0);
        settingTab->show();
        settingTab->initializeData();
    }
}
