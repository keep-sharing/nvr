#include "PageCameraStatus.h"
#include "ui_PageCameraStatus.h"
#include "MsDevice.h"
#include "MsLanguage.h"

PageCameraStatus::PageCameraStatus(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::CameraStatus)
{
    ui->setupUi(this);

    ui->widget_tab->addTab(GET_TEXT("CAMERASTATUS/62001", "Channel Status"));
    ui->widget_tab->addTab(GET_TEXT("CAMERASTATUS/62014", "PoE Port Status"));
    connect(ui->widget_tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
    ui->widget_tab->setCurrentTab(0);

    if (!qMsNvr->isPoe()) {
        ui->widget_tab->hideTab(1);
    }

    onLanguageChanged();
}

PageCameraStatus::~PageCameraStatus()
{
    delete ui;
}

void PageCameraStatus::initializeData()
{
    ui->widget_tab->setCurrentTab(0);
}

bool PageCameraStatus::canAutoLogout()
{
    return false;
}

void PageCameraStatus::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PageCameraStatus::onLanguageChanged()
{
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PageCameraStatus::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageCameraStatus::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);

    switch (index) {
    case 0:
        ui->page_channel->initializeData();
        break;
    case 1:
        ui->page_poe->initializeData();
        break;
    default:
        break;
    }
}
