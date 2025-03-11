#include "NetworkSetting.h"
#include "ui_NetworkSetting.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "NetworkPageBasic.h"
#include "NetworkPageCloud.h"
#include "NetworkPageDDNS.h"
#include "NetworkPageEmail.h"
#include "NetworkPageMore.h"
#include "NetworkPageP2P.h"
#include "NetworkPagePPPoE.h"
#include "NetworkPageSNMP.h"
#include "NetworkPageUPnP.h"
#include "NetWorkPageMulticast.h"
NetworkSetting::NetworkSetting(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::NetworkSetting)
{
    ui->setupUi(this);

    ui->tabBar->setHSpacing(45);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));

    onLanguageChanged();
}

NetworkSetting::~NetworkSetting()
{
    delete ui;
}

void NetworkSetting::initializeData()
{
    ui->tabBar->setCurrentTab(Network_Basic);
}

void NetworkSetting::onTabBarClicked(int index)
{
    m_currentTab = static_cast<NetWorkTab>(index);

    AbstractNetworkPage *page = m_tabMap.value(m_currentTab, nullptr);
    if (!page) {
        switch (m_currentTab) {
        case Network_Basic:
            page = new NetworkPageBasic(this);
            break;
        case Network_UPnP:
            page = new NetworkPageUPnp(this);
            break;
        case Network_DDNS:
            page = new NetworkPageDDNS(this);
            break;
        case Network_Email:
            page = new NetworkPageEmail(this);
            break;
        case Network_P2P:
            page = new NetworkPageP2P(this);
            break;
        case Network_Cloud:
            page = new NetworkPageCloud(this);
            break;
        case Network_PPPoE:
            page = new NetworkPagePPPoe(this);
            break;
        case Network_SNMP:
            page = new NetworkPageSNMP(this);
            break;
        case NetWork_Multicast:
            page = new NetWorkPageMulticast(this);
            break;
        case Network_More:
            page = new NetworkPageMore(this);
            break;
        default:
            break;
        }
        if (page) {
            connect(page, SIGNAL(sig_back()), this, SIGNAL(sig_back()));
            m_tabMap.insert(m_currentTab, page);
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

void NetworkSetting::onLanguageChanged()
{
    ui->tabBar->clear();
    QMap<NetWorkTab, QString> tabMap;
    tabMap.insert(Network_Basic, GET_TEXT("SYSTEMNETWORK/71000", "Basic"));
    tabMap.insert(Network_UPnP, GET_TEXT("SYSTEMNETWORK/71142", "UPnP"));
    tabMap.insert(Network_DDNS, GET_TEXT("SYSTEMNETWORK/71002", "DDNS"));
    tabMap.insert(Network_Email, GET_TEXT("SYSTEMNETWORK/71003", "Email"));
    tabMap.insert(Network_P2P, GET_TEXT("SYSTEMNETWORK/71004", "P2P"));
    tabMap.insert(Network_Cloud, "Milesight Cloud");
    tabMap.insert(Network_PPPoE, GET_TEXT("SYSTEMNETWORK/71006", "PPPoE"));
    tabMap.insert(Network_SNMP, GET_TEXT("SYSTEMNETWORK/71187", "SNMP"));
    tabMap.insert(NetWork_Multicast, GET_TEXT("NETWORKSTATUS/144000", "Multicast"));
    tabMap.insert(Network_More, GET_TEXT("SYSTEMNETWORK/71005", "More"));

    if (qMsNvr->isSlaveMode()) {
        //只保留Basic和More
        tabMap.remove(Network_UPnP);
        tabMap.remove(Network_DDNS);
        tabMap.remove(Network_Email);
        tabMap.remove(Network_P2P);
        tabMap.remove(Network_Cloud);
        tabMap.remove(Network_PPPoE);
        tabMap.remove(Network_SNMP);
        tabMap.remove(NetWork_Multicast);
    }
    if (qMsNvr->isMilesight()) {
        tabMap.remove(Network_P2P);
    } else {
        tabMap.remove(Network_Cloud);
    }

    for (auto iter = tabMap.constBegin(); iter != tabMap.constEnd(); ++iter) {
        ui->tabBar->addTab(iter.value(), iter.key());
    }
}
