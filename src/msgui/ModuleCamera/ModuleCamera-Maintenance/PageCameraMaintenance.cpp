#include "PageCameraMaintenance.h"
#include "ui_PageCameraMaintenance.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "TabCameraDiagnosisInformation.h"
#include "TabCameraExportConfiguration.h"
#include "TabCameraLocalUpgrade.h"
#include "TabCameraOnlineUpgrade.h"
#include "TabCameraReboot.h"
#include "TabCameraReset.h"
#include "TabCameraLogs.h"
#include <QtDebug>

PageCameraMaintenance::PageCameraMaintenance(QWidget *parent)
    : AbstractCameraPage(parent)
    , ui(new Ui::CameraMaintenance)
{
    ui->setupUi(this);

    ui->tab->addTab(GET_TEXT("UPGRADE/75011", "Local Upgrade"), TabLocal);
    ui->tab->addTab(GET_TEXT("UPGRADE/75012", "Online Upgrade"), TabOnline);
    ui->tab->addTab(GET_TEXT("SYSTEMGENERAL/70007", "Import/Export Configuration"), TabConfiguration);
    ui->tab->addTab(GET_TEXT("USER/74045", "Reboot"), TabReboot);
    ui->tab->addTab(GET_TEXT("OCCUPANCY/74221", "Reset"), TabReset);
    ui->tab->addTab(GET_TEXT("MAINTENANCE/77005", "Diagnosis Information"), TabDiagnosisInfomation);
    ui->tab->addTab(GET_TEXT("LOG/64000", "Logs"), TabLog);
    if (qMsNvr->OEMType() && qMsNvr->deviceInfo().oemupdateonline == 0 && QString(qMsNvr->deviceInfo().company) != QString("Milesight")) {
        ui->tab->hideTab(TabOnline);
    }
    connect(ui->tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
}

PageCameraMaintenance::~PageCameraMaintenance()
{
    delete ui;
}

void PageCameraMaintenance::initializeData()
{
    ui->tab->setCurrentTab(TabLocal);
}

void PageCameraMaintenance::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

bool PageCameraMaintenance::isCloseable()
{
    AbstractSettingTab *page = currentPage();
    if (page) {
        return page->isCloseable();
    } else {
        return true;
    }
}

bool PageCameraMaintenance::isChangeable()
{
    AbstractSettingTab *page = currentPage();
    if (page) {
        return page->isChangeable();
    } else {
        return true;
    }
}

bool PageCameraMaintenance::canAutoLogout()
{
    AbstractSettingTab *page = currentPage();
    if (page) {
        return page->canAutoLogout();
    } else {
        return true;
    }
}

void PageCameraMaintenance::onLanguageChanged()
{
}

AbstractSettingTab *PageCameraMaintenance::currentPage()
{
    AbstractSettingTab *page = nullptr;
    if (m_tabMap.contains(m_currentTab)) {
        page = m_tabMap.value(m_currentTab);
    }
    return page;
}

void PageCameraMaintenance::onTabClicked(int index)
{
    if (!isChangeable()) {
        qDebug() << "CameraMaintenance::onTabClicked";
        ////closeWait();
        ui->tab->editCurrentTab(m_currentTab);
        return;
    }

    m_currentTab = index;

    QLayoutItem *item = ui->gridLayout->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->hide();
        }
        ui->gridLayout->removeItem(item);
        delete item;
    }

    AbstractSettingTab *page = currentPage();
    if (!page) {
        switch (m_currentTab) {
        case TabLocal:
            page = new TabCameraLocalUpgrade(this);
            break;
        case TabOnline:
            page = new TabCameraOnlineUpgrade(this);
            break;
        case TabConfiguration:
            page = new TabCameraExportConfiguration(this);
            break;
        case TabReboot:
            page = new TabCameraReboot(this);
            break;
        case TabReset:
            page = new TabCameraReset(this);
            break;
        case TabDiagnosisInfomation:
            page = new TabCameraDiagnosisInformation(this);
            break;
        case TabLog:
            page = new TabCameraLogs(this);
            break;
        default:
            break;
        }
        if (page) {
            connect(page, SIGNAL(sig_back()), this, SIGNAL(sig_back()));
            m_tabMap.insert(m_currentTab, page);
        }
    }
    if (page) {
        ui->gridLayout->addWidget(page, 0, 0);
        page->initializeData();
        page->show();
    }
}
