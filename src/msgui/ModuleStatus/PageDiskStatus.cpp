#include "PageDiskStatus.h"
#include "ui_PageDiskStatus.h"
#include "MsDevice.h"
#include "MsLanguage.h"

PageDiskStatus::PageDiskStatus(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::DiskStatus)
{
    ui->setupUi(this);

    ui->widget_tab->addTab(GET_TEXT("CAMERASTATUS/62008", "Disk Status"));
    ui->widget_tab->addTab("S.M.A.R.T");
    if (qMsNvr->is3536a()) {
        ui->widget_tab->addTab(GET_TEXT("STATUS/177000", "Disk Health Detection"));
    }
    connect(ui->widget_tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));

    onLanguageChanged();
}

PageDiskStatus::~PageDiskStatus()
{
    delete ui;
}

void PageDiskStatus::initializeData()
{
    ui->widget_tab->setCurrentTab(0);
}

bool PageDiskStatus::canAutoLogout()
{
    return false;
}

void PageDiskStatus::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PageDiskStatus::onLanguageChanged()
{
    ui->pushButton_refresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonSave->setText(GET_TEXT("COMMON/1003", "Apply"));
}

void PageDiskStatus::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);

    switch (index) {
    case 0:
        ui->page_disk->initializeData();
        ui->pushButton_refresh->show();
        ui->pushButtonSave->hide();
        break;
    case 1:
        ui->page_smart->initializeData(ui->page_disk->localDiskList());
        ui->pushButton_refresh->hide();
        ui->pushButtonSave->hide();
        break;
    case 2:
        ui->page_health->initializeData();
        ui->pushButton_refresh->show();
        ui->pushButtonSave->show();
        break;
    default:
        break;
    }
}

void PageDiskStatus::on_pushButton_refresh_clicked()
{
    if (ui->stackedWidget->currentIndex() == 0) {
        ui->page_disk->refreshData();
    } else if (ui->stackedWidget->currentIndex() == 2) {
        ui->page_health->refreshData();
    }
}

void PageDiskStatus::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageDiskStatus::on_pushButtonSave_clicked()
{
    if (ui->stackedWidget->currentIndex() == 2) {
        ui->page_health->saveData();
    }
}
