#include "storagemode.h"
#include "ui_storagemode.h"
#include "MsLanguage.h"
#include "storagequota.h"
#include "storagegroup.h"

StorageMode::StorageMode(QWidget *parent) :
    AbstractSettingPage(parent),
    ui(new Ui::StorageMode)
{
    ui->setupUi(this);

    ui->tabBar->addTab(GET_TEXT("STORAGEMODE/95101", "Quota"), TabQuota);
    ui->tabBar->addTab(GET_TEXT("DISK/92004", "Group"), TabGroup);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
}

StorageMode::~StorageMode()
{
    delete ui;
}

void StorageMode::initializeData()
{
    ui->tabBar->setCurrentTab(TabQuota);
}

void StorageMode::onTabClicked(int index)
{
    m_currentTab = index;
    AbstractSettingTab *settingTab = m_tabMap.value(m_currentTab, nullptr);
    if (!settingTab)
    {
        switch (m_currentTab)
        {
        case TabQuota:
            settingTab = new StorageQuota(this);
            break;
        case TabGroup:
            settingTab = new StorageGroup(this);
            break;
        default:
            break;
        }
        if (settingTab)
        {
            connect(settingTab, SIGNAL(sig_back()), this, SIGNAL(sig_back()));
            m_tabMap.insert(m_currentTab, settingTab);
        }
    }
    //
    QLayoutItem *item = ui->gridLayout->itemAtPosition(0, 0);
    if (item)
    {
        QWidget *widget = item->widget();
        if (widget)
        {
            widget->hide();
        }
        ui->gridLayout->removeItem(item);
        delete item;
    }
    //
    if (settingTab)
    {
        ui->gridLayout->addWidget(settingTab, 0, 0);
        settingTab->show();
        settingTab->initializeData();
    }
}
