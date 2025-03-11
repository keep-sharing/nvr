#include "TabNetwork.h"
#include "ui_TabNetwork.h"
#include "centralmessage.h"
#include "MsDevice.h"
#include "MsLanguage.h"

extern "C" {
#include "msdb.h"
}

TabNetwork::TabNetwork(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabNetwork)
{
    ui->setupUi(this);
    ui->groupBox_pppoe->hide();

    onLanguageChanged();
}

TabNetwork::~TabNetwork()
{
    delete ui;
}

void TabNetwork::initializeData()
{
    const device_info &sys_info = qMsNvr->deviceInfo();

    ui->groupBox_pppoe->hide();

    struct network db_network;
    memset(&db_network, 0, sizeof(struct network));
    read_network(SQLITE_FILE_NAME, &db_network);
    if (db_network.mode == NETMODE_MULTI) {
        if (sys_info.max_lan == 1) {
            ui->groupBox_lan2->hide();
        }
    } else {
        ui->groupBox_lan2->hide();
    }

    sendMessage(REQUEST_FLAG_GET_NETWORK_INFO, (void *)NULL, 0);
}

void TabNetwork::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_NETWORK_INFO:
        ON_RESPONSE_FLAG_GET_NETWORK_INFO(message);
        break;
    }
}

void TabNetwork::ON_RESPONSE_FLAG_GET_NETWORK_INFO(MessageReceive *message)
{
    if (message->isNull()) {
        return;
    }
    struct resp_network_info *network_info_list = (struct resp_network_info *)message->data;
    int count = message->size() / sizeof(struct resp_network_info);
    for (int i = 0; i < count; ++i) {
        const resp_network_info &network_info = network_info_list[i];
        if (network_info.netif_id == NETIF_LAN0 || network_info.netif_id == NETIF_BOND0) {
            //connected
            if (network_info.conn_stat == 1) {
                ui->lineEdit_connection->setText("Link is up");
                //mode
                if (network_info.duplex == 0) {
                    //half-duplex
                    ui->lineEdit_mode->setText(QString("%1 Mbps HALF").arg(network_info.speed_mode));
                } else {
                    //duplex
                    ui->lineEdit_mode->setText(QString("%1 Mbps FULL").arg(network_info.speed_mode));
                }
            } else {
                //disconnected
                ui->lineEdit_connection->setText("Link is down");
                ui->lineEdit_mode->clear();
            }
            //dhcp
            if (network_info.dhcp_enable) {
                ui->lineEdit_ipv4DHCP->setText("Enable");
            } else {
                ui->lineEdit_ipv4DHCP->setText("Disable");
            }
            //ipv4 address
            ui->lineEdit_ipv4Address->setText(QString(network_info.ip));
            //ipv4 mask
            ui->lineEdit_ipv4SubnetMask->setText(QString(network_info.netmask));
            //ipv4 gateway
            ui->lineEdit_ipv4Gateway->setText(QString(network_info.gateway));
            //preferred dns
            ui->lineEdit_preferredDNS->setText(QString(network_info.primary_dns));
            //alternate dns
            ui->lineEdit_alternateDNS->setText(QString(network_info.second_dns));
            //ipv6 mode
            if (network_info.ip6_mode == 0) {
                ui->lineEdit_ipv6Mode->setText(QString("Manual"));
            } else if (network_info.ip6_mode == 1) {
                ui->lineEdit_ipv6Mode->setText(QString("Router Advertisement"));
            } else {
                ui->lineEdit_ipv6Mode->setText(QString("DHCPv6"));
            }
            //ipv6 address
            ui->lineEdit_ipv6Address->setText(network_info.ip6_address);
            //ipv6 prefix length
            if (QString(network_info.ip6_address).isEmpty() && network_info.ip6_prefix == 0) {
                ui->lineEdit_ipv6PrefixLength->clear();
            } else {
                ui->lineEdit_ipv6PrefixLength->setText(QString("%1").arg(network_info.ip6_prefix));
            }
            //ipv6 gateway
            ui->lineEdit_ipv6Gateway->setText(network_info.ip6_gateway);
            //mtu
            ui->lineEdit_mtu->setText(QString("%1").arg(network_info.mtu));
            //mac
            ui->lineEdit_mac->setText(QString(network_info.mac).toUpper());
        } else if (network_info.netif_id == NETIF_LAN1) {
            //connected
            if (network_info.conn_stat == 1) {
                ui->lineEdit_connection_2->setText("Link is up");
                //mode
                if (network_info.duplex == 0) {
                    //half-duplex
                    ui->lineEdit_mode_2->setText(QString("%1 Mbps HALF").arg(network_info.speed_mode));
                } else {
                    //duplex
                    ui->lineEdit_mode_2->setText(QString("%1 Mbps FULL").arg(network_info.speed_mode));
                }
            } else {
                //disconnected
                ui->lineEdit_connection_2->setText("Link is down");
                ui->lineEdit_mode_2->clear();
            }
            //dhcp
            if (network_info.dhcp_enable) {
                ui->lineEdit_ipv4DHCP_2->setText("Enable");
            } else {
                ui->lineEdit_ipv4DHCP_2->setText("Disable");
            }
            //ipv4 address
            ui->lineEdit_ipv4Address_2->setText(QString(network_info.ip));
            //ipv4 mask
            ui->lineEdit_ipv4SubnetMask_2->setText(QString(network_info.netmask));
            //ipv4 gateway
            ui->lineEdit_ipv4Gateway_2->setText(QString(network_info.gateway));
            //preferred dns
            ui->lineEdit_preferredDNS_2->setText(QString(network_info.primary_dns));
            //alternate dns
            ui->lineEdit_alternateDNS_2->setText(QString(network_info.second_dns));
            //ipv6 mode
            if (network_info.ip6_mode == 0) {
                ui->lineEdit_ipv6Mode_2->setText(QString("Manual"));
            } else if (network_info.ip6_mode == 1) {
                ui->lineEdit_ipv6Mode_2->setText(QString("Router Advertisement"));
            } else {
                ui->lineEdit_ipv6Mode_2->setText(QString("DHCPv6"));
            }
            //ipv6 address
            ui->lineEdit_ipv6Address_2->setText(network_info.ip6_address);
            //ipv6 prefix length
            if (QString(network_info.ip6_address).isEmpty() && network_info.ip6_prefix == 0) {
                ui->lineEdit_ipv6PrefixLength_2->clear();
            } else {
                ui->lineEdit_ipv6PrefixLength_2->setText(QString("%1").arg(network_info.ip6_prefix));
            }
            //ipv6 gateway
            ui->lineEdit_ipv6Gateway_2->setText(network_info.ip6_gateway);
            //mtu
            ui->lineEdit_mtu_2->setText(QString("%1").arg(network_info.mtu));
            //mac
            ui->lineEdit_mac_2->setText(QString(network_info.mac).toUpper());
        } else if (network_info.netif_id == NETIF_PPPOE) {
            if (network_info.enable) {
                ui->groupBox_pppoe->show();
            }
            ui->lineEdit_ipAddress->setText(network_info.ip);
            ui->lineEdit_netmask->setText(network_info.netmask);
            ui->lineEdit_gateway->setText(network_info.gateway);
            //connected
            if (network_info.conn_stat == 1) {
                ui->lineEdit_connection_3->setText("Link is up");
                ui->pushButton_connect->setText("Disconnect");
            } else {
                //disconnected
                ui->lineEdit_connection_3->setText("Link is down");
                ui->pushButton_connect->setText("Connect");
            }
        }
    }
}

void TabNetwork::onLanguageChanged()
{
    ui->groupBox_lan1->setTitle(GET_TEXT("CAMERASEARCH/32022", "LAN1"));
    ui->groupBox_lan2->setTitle(GET_TEXT("CAMERASEARCH/32023", "LAN2"));

    ui->label_connection->setText(GET_TEXT("NETWORKSTATUS/61004", "Connection"));
    ui->label_connection_2->setText(GET_TEXT("NETWORKSTATUS/61004", "Connection"));
    ui->label_ipv4DHCP->setText(GET_TEXT("SYSTEMNETWORK/71157", "IPv4 DHCP"));
    ui->label_ipv4DHCP_2->setText(GET_TEXT("SYSTEMNETWORK/71157", "IPv4 DHCP"));
    ui->label_ipv4Address->setText(GET_TEXT("SYSTEMNETWORK/71158", "IPv4 Address"));
    ui->label_ipv4Address_2->setText(GET_TEXT("SYSTEMNETWORK/71158", "IPv4 Address"));
    ui->label_ipv4SubnetMask->setText(GET_TEXT("SYSTEMNETWORK/71159", "IPv4 Subnet Mask"));
    ui->label_ipv4SubnetMask_2->setText(GET_TEXT("SYSTEMNETWORK/71159", "IPv4 Subnet Mask"));
    ui->label_ipv4Gateway->setText(GET_TEXT("SYSTEMNETWORK/71160", "IPv4 Gateway"));
    ui->label_ipv4Gateway_2->setText(GET_TEXT("SYSTEMNETWORK/71160", "IPv4 Gateway"));
    ui->label_preferredDNS->setText(GET_TEXT("WIZARD/11027", "Preferred DNS Server"));
    ui->label_preferredDNS_2->setText(GET_TEXT("WIZARD/11027", "Preferred DNS Server"));
    ui->label_alternateDNS->setText(GET_TEXT("WIZARD/11028", "Alternate DNS Server"));
    ui->label_alternateDNS_2->setText(GET_TEXT("WIZARD/11028", "Alternate DNS Server"));

    ui->label_mode->setText(GET_TEXT("NETWORKSTATUS/61005", "Mode"));
    ui->label_mode_2->setText(GET_TEXT("NETWORKSTATUS/61005", "Mode"));
    ui->label_ipv6Mode->setText(GET_TEXT("SYSTEMNETWORK/71151", "IPv6 Mode"));
    ui->label_ipv6Mode_2->setText(GET_TEXT("SYSTEMNETWORK/71151", "IPv6 Mode"));
    ui->label_ipv6Address->setText(GET_TEXT("SYSTEMNETWORK/71152", "IPv6 Address"));
    ui->label_ipv6Address_2->setText(GET_TEXT("SYSTEMNETWORK/71152", "IPv6 Address"));
    ui->label_ipv6PrefixLength->setText(GET_TEXT("SYSTEMNETWORK/71153", "IPv6 Prefix Length"));
    ui->label_ipv6PrefixLength_2->setText(GET_TEXT("SYSTEMNETWORK/71153", "IPv6 Prefix Length"));
    ui->label_ipv6Gateway->setText(GET_TEXT("SYSTEMNETWORK/71154", "IPv6 Gateway"));
    ui->label_ipv6Gateway_2->setText(GET_TEXT("SYSTEMNETWORK/71154", "IPv6 Gateway"));
    ui->label_mtu->setText(GET_TEXT("NETWORKSTATUS/61007", "MTU(B)"));
    ui->label_mtu_2->setText(GET_TEXT("NETWORKSTATUS/61007", "MTU(B)"));
    ui->label_mac->setText(GET_TEXT("CHANNELMANAGE/30027", "MAC"));
    ui->label_mac_2->setText(GET_TEXT("CHANNELMANAGE/30027", "MAC"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabNetwork::on_pushButtonBack_clicked()
{
    emit sig_back();
}
