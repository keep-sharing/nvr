#include "PagePtzConfiguration.h"
#include "ui_PagePtzConfiguration.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "TabPtzAdvanced.h"
#include "TabPtzAutoHome.h"
#include "TabPtzAutoTracking.h"
#include "TabPtzBasic.h"
#include "TabPtzConfigClear.h"
#include "TabPtzConfiguration.h"
#include "TabPtzInitialPosition.h"
#include "TabPtzLimit.h"
#include "TabPtzPrivacyMask.h"
#include "TabPtzScheduledTasks.h"
#include "TabPtzWiper.h"
#include <QtDebug>

PagePtzConfiguration::PagePtzConfiguration(QWidget *parent)
    : AbstractCameraPage(parent)
    , ui(new Ui::PtzConfigurationManager)
{
    ui->setupUi(this);

    //tab
    ui->tabBar->setHSpacing(20);
    ui->tabBar->addTab(GET_TEXT("PTZBASIC/111000", "Basic"), PageBasic);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/166000", "Auto Home"), PageAutoHome);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/166001", "PTZ Limits"), PagePTZLimit);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/166002", "Initial Position"), PageInitialPosition);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/36051", "PTZ Privacy Mask"), PagePrivacyMask);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/166003", "Scheduled Tasks"), PageScheduledTasks);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/36052", "Auto Tracking"), PageAutoTracking);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/166004", "Config Clear"), PageConfigClear);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/166005", "Wiper"), PageWiper);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/36016", "Advanced"), PageAdvanced);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
}

PagePtzConfiguration::~PagePtzConfiguration()
{
    delete ui;
}

void PagePtzConfiguration::initializeData()
{
    PtzBasePage::setCurrentChannel(0);

    m_currentPageType = PageBasic;
    ui->tabBar->setCurrentTab(m_currentPageType);
}

void PagePtzConfiguration::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PagePtzConfiguration::onTabClicked(int index)
{
    m_currentPageType = static_cast<PageType>(index);
    PtzBasePage *page = m_pageMap.value(m_currentPageType, nullptr);
    if (!page) {
        switch (m_currentPageType) {
        case PageBasic:
            page = new TabPtzBasic(this);
            break;
        case PageAutoHome:
            page = new TabPtzAutoHome(this);
            break;
        case PagePTZLimit:
            page = new TabPtzLimit(this);
            break;
        case PageInitialPosition:
            page = new TabPtzInitialPosition(this);
            break;
        case PagePrivacyMask:
            page = new TabPtzPrivacyMask(this);
            break;
        case PageScheduledTasks:
            page = new TabPtzScheduledTasks(this);
            break;
        case PageAutoTracking:
            page = new TabPtzAutoTracking(this);
            break;
        case PageConfigClear:
            page = new TabPtzConfigClear(this);
            break;
        case PageWiper:
            page = new TabPtzWiper(this);
            break;
        case PageAdvanced:
            page = new TabPtzAdvanced(this);
            break;
        default:
            break;
        }
        if (page) {
            m_pageMap.insert(m_currentPageType, page);
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
    if (page) {
        ui->gridLayout->addWidget(page, 0, 0);
        page->show();
        page->initializeData();
    }
}
