#include "PageDiskManagement.h"
#include "ui_PageDiskManagement.h"
#include "MsLanguage.h"
#include "MsDevice.h"

PageDiskManagement::PageDiskManagement(QWidget *parent) :
    AbstractSettingPage(parent),
    ui(new Ui::PageDiskManagement)
{
    ui->setupUi(this);
    ui->widget_tab->addTab(GET_TEXT("STORAGEMODE/95110", "Disk Management"));
    if (!qMsNvr->isSlaveMode()) {
        ui->widget_tab->addTab(GET_TEXT("USER/74034", "General Settings"));
    }

    connect(ui->widget_tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
    connect(ui->pageDiskSetting, SIGNAL(sig_back()), this, SIGNAL(sig_back()));
    connect(ui->pageStorageGeneralSetting, SIGNAL(sig_back()), this, SIGNAL(sig_back()));

    onLanguageChanged();
}

PageDiskManagement::~PageDiskManagement()
{
    delete ui;
}

void PageDiskManagement::initializeData()
{
    ui->widget_tab->setCurrentTab(0);
}

void PageDiskManagement::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PageDiskManagement::onLanguageChanged()
{

}

void PageDiskManagement::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);

    switch (index) {
    case 0:
        ui->pageDiskSetting->initializeData();
        break;
    case 1:
        ui->pageStorageGeneralSetting->initializeData();
        break;
    default:
        break;
    }
}
