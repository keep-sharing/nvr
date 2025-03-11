#include "PagePeopleCountingSettings.h"
#include "ui_PagePeopleCountingSettings.h"
#include "TabPeopleCountingSettings.h"
#include "TabRegionalPeopleCountingSettings.h"
#include "TabReportAutoBackupSettings.h"
#include "MsLanguage.h"
#include "TabOccupancyLiveViewSettings.h"

PagePeopleCountingSettings::PagePeopleCountingSettings(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PagePeopleCountingSettings)
{
    ui->setupUi(this);

    ui->tabBar->addTab(GET_TEXT("OCCUPANCY/74300", "People Counting Settings"), TAB_PEOPLE_COUNTING_SETTINGS);
    ui->tabBar->addTab(GET_TEXT("OCCUPANCY/74215", "Occupancy Live View Settings"), TAB_OCCUPANCY_LIVE_VIEW_SETTINGS);
    ui->tabBar->addTab(GET_TEXT("REGIONAL_PEOPLECOUNTING/103300", "Regional People Counting Settings"), TAB_REGIONAL_PEOPLE_COUNTING_SETTINGS);
    ui->tabBar->addTab(GET_TEXT("REPORTAUTOBACKUP/114009", "Report Auto Backup Settings"), TAB_REPORT_AUTO_BACKUP_SETTINGS);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
}

PagePeopleCountingSettings::~PagePeopleCountingSettings()
{
    delete ui;
}

void PagePeopleCountingSettings::initializeData()
{
    ui->tabBar->setCurrentTab(TAB_PEOPLE_COUNTING_SETTINGS);
}

void PagePeopleCountingSettings::onTabClicked(int index)
{
    m_currentTab = index;
    AbstractSettingTab *settingTab = m_tabMap.value(m_currentTab, nullptr);
    if (!settingTab) {
        switch (m_currentTab) {
        case TAB_PEOPLE_COUNTING_SETTINGS:
            settingTab = new TabPeopleCountingSettings(this);
            break;
        case TAB_OCCUPANCY_LIVE_VIEW_SETTINGS:
            settingTab = new TabOccupancyLiveViewSettings(this);
            break;
        case TAB_REGIONAL_PEOPLE_COUNTING_SETTINGS:
            settingTab = new TabRegionalPeopleCountingSettings(this);
            break;
        case TAB_REPORT_AUTO_BACKUP_SETTINGS:
            settingTab = new TabReportAutoBackupSettings(this);
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
