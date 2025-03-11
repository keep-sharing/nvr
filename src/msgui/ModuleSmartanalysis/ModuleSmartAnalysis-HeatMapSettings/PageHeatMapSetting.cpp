#include "PageHeatMapSetting.h"
#include "ui_PageHeatMapSetting.h"
#include "MsLanguage.h"
#include "TabHeatMapSettings.h"
#include "TabHeatMapAutoBackup.h"

PageHeatMapSetting::PageHeatMapSetting(QWidget *parent) :
    AbstractSettingPage(parent),
    ui(new Ui::HeatMapSetting)
{
    ui->setupUi(this);
    ui->tabBar->addTab(GET_TEXT("HEATMAP/104000", "Heat Map"), TAB_HEAT_MAP_SETTING);
    ui->tabBar->addTab(GET_TEXT("REPORTAUTOBACKUP/114009", "Report Auto Backup Settings"), TAB_HEAT_MAP_AUTO_BACKUP_SETTING);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
}

PageHeatMapSetting::~PageHeatMapSetting()
{
    delete ui;
}

void PageHeatMapSetting::initializeData()
{
    ui->tabBar->setCurrentTab(TAB_HEAT_MAP_SETTING);
}

void PageHeatMapSetting::onTabClicked(int index)
{
    m_currentTab = index;
    AbstractSettingTab *settingTab = m_tabMap.value(m_currentTab, nullptr);
    if (!settingTab) {
        switch (m_currentTab) {
        case TAB_HEAT_MAP_SETTING:
            settingTab = new TabHeatMapSettings(this);
            break;
        case TAB_HEAT_MAP_AUTO_BACKUP_SETTING:
            settingTab = new TabHeatMapAutoBackup(this);
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
    QLayoutItem *item = ui->gridLayout->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->hide();
        }
        ui->gridLayout->removeItem(item);
        delete item;
    }
    //
    if (settingTab) {
        ui->gridLayout->addWidget(settingTab, 0, 0);
        settingTab->show();
        settingTab->initializeData();
    }
}
