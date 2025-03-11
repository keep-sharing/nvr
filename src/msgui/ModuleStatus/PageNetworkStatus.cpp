#include "PageNetworkStatus.h"
#include "ui_PageNetworkStatus.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QtDebug>

PageNetworkStatus::PageNetworkStatus(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::NetworkStatus)
{
    ui->setupUi(this);

    ui->widget_tab->addTab(GET_TEXT("NETWORKSTATUS/61000", "Network Status"));
    ui->widget_tab->addTab(GET_TEXT("NETWORKSTATUS/61001", "Bandwidth Status"));
    ui->widget_tab->addTab(GET_TEXT("PING/116002", "Network Test"));
    ui->widget_tab->setCurrentTab(0);
    connect(ui->widget_tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
    connect(ui->page_network, SIGNAL(sig_back()), this, SIGNAL(sig_back()));
    connect(ui->page_bandwidth, SIGNAL(sig_back()), this, SIGNAL(sig_back()));
    connect(ui->page_networkTest, SIGNAL(sig_back()), this, SIGNAL(sig_back()));

    ui->stackedWidget->setCurrentIndex(0);
    onLanguageChanged();
}

PageNetworkStatus::~PageNetworkStatus()
{
    delete ui;
}

void PageNetworkStatus::initializeData()
{
    ui->widget_tab->setCurrentTab(0);
}

bool PageNetworkStatus::canAutoLogout()
{
    return false;
}

void PageNetworkStatus::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PageNetworkStatus::onLanguageChanged()
{
}

void PageNetworkStatus::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);

    switch (index) {
    case 0:
        ui->page_network->initializeData();
        break;
    case 1:
        ui->page_bandwidth->initializeData();
        break;
    case 2:
        ui->page_networkTest->initializeData();

        break;
    default:
        break;
    }
}
