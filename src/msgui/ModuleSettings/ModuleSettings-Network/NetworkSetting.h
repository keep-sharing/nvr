#ifndef NETWORKSETTING_H
#define NETWORKSETTING_H

#include "AbstractSettingPage.h"

class AbstractNetworkPage;

namespace Ui {
class NetworkSetting;
}

class NetworkSetting : public AbstractSettingPage {
    Q_OBJECT

    enum NetWorkTab {
        Network_Basic,
        Network_UPnP,
        Network_DDNS,
        Network_Email,
        Network_P2P,
        Network_Cloud,
        Network_PPPoE,
        Network_SNMP,
        NetWork_Multicast,
        Network_More,
        Network_None
    };

public:
    explicit NetworkSetting(QWidget *parent = nullptr);
    ~NetworkSetting();

    void initializeData() override;

private slots:
    void onTabBarClicked(int index);
    void onLanguageChanged() override;

private:
    Ui::NetworkSetting *ui;

    NetWorkTab m_currentTab = Network_None;
    QMap<NetWorkTab, AbstractNetworkPage *> m_tabMap;
};

#endif // NETWORKSETTING_H
