#include "NetworkPageBasic.h"
#include "ui_NetworkPageBasic.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "RouterEdit.h"
#include "centralmessage.h"
#include "msuser.h"
#include "myqt.h"

extern "C" {
#include "log.h"
}

const int IPv4_LAN_MANUAL = 0;
const int IPv4_LAN_DHCP = 1;

const int IPv6_LAN_MANUAL = 0;
const int IPv6_LAN_ROUTER = 1;
const int IPv6_LAN_DHCP = 2;

NetworkPageBasic::NetworkPageBasic(QWidget *parent)
    : AbstractNetworkPage(parent)
    , ui(new Ui::NetworkPageBasic)
{
    ui->setupUi(this);

    ui->comboBoxDefaultRoute->clear();
    ui->comboBoxDefaultRoute->addItem("LAN1", 0);
    ui->comboBoxDefaultRoute->addItem("LAN2", 1);

    initNetBasicTab();
    ui->lineEdit_eth0IP->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_eth0Netmask->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_eth0Gateway->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_eth0PreDns->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_eth0AlterDns->setCheckMode(MyLineEdit::IPv4CanEmptyCheck);
    ui->lineEdit_eth0IPv6Addr->setCheckMode(MyLineEdit::IPv6CanEmptyCheck);
    ui->lineEdit_eth0IPv6GateWay->setCheckMode(MyLineEdit::IPv6CanEmptyCheck);
    ui->lineEdit_eth0IPvPreLen->setCheckMode(MyLineEdit::RangeCanEmptyCheck, 0, 128);
    ui->lineEdit_poeIPaddr->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_mtu0->setCheckMode(MyLineEdit::RangeCheck, 1200, 1500);

    ui->lineEdit_eth1IP->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_eth1Netmask->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_eth1Gateway->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_eth1PreDns->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_eth1AlterDns->setCheckMode(MyLineEdit::IPv4CanEmptyCheck);
    ui->lineEdit_eth1IPv6Addr->setCheckMode(MyLineEdit::IPv6CanEmptyCheck);
    ui->lineEdit_eth1IPv6GateWay->setCheckMode(MyLineEdit::IPv6CanEmptyCheck);
    ui->lineEdit_eth1IPvPreLen->setCheckMode(MyLineEdit::RangeCanEmptyCheck, 0, 128);
    ui->lineEdit_mtu1->setCheckMode(MyLineEdit::RangeCheck, 1200, 1500);

    onLanguageChanged();
}

NetworkPageBasic::~NetworkPageBasic()
{
    freeNetBasicTab();
    delete ui;
}

void NetworkPageBasic::initializeData()
{
    gotoNetBasicTab();
}

void NetworkPageBasic::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_TEST_IP_CONFLICT:
    case RESPONSE_FLAG_IP_CONFLICT_BY_DEV:
        ON_RESPONSE_FLAG_TEST_IP_CONFLICT(message);
        break;
    }
}

void NetworkPageBasic::initNetBasicTab()
{
    get_mac_addr(mac0, mac1);

    const struct device_info &sys_info = qMsNvr->deviceInfo();
    qDebug() << QString("NetworkPageBasic::initNetBasicTab, MaxLan: %1, PoeCount: %2").arg(sys_info.max_lan).arg(qMsNvr->poeCount());
    ui->comboBox_workingMode->clear();
    if (sys_info.max_lan == 1 || qMsNvr->isPoe()) {
        ui->comboBox_workingMode->addItem(GET_TEXT("SYSTEMNETWORK/71007", "Multi address"), 0);
        ui->groupBox_eth0Enable->setCheckable(false);

        ui->groupBox_eth1Enable->hide();

        ui->label_workingMode->hide();
        ui->comboBox_workingMode->hide();
    } else {
        ui->comboBox_workingMode->addItem(GET_TEXT("SYSTEMNETWORK/71007", "Multi address"), 0);
        ui->comboBox_workingMode->addItem(GET_TEXT("SYSTEMNETWORK/71008", "Load Balance"), 1);
        ui->comboBox_workingMode->addItem(GET_TEXT("SYSTEMNETWORK/71009", "Net Fault-tolerance"), 2);
    }

    ui->comboBox_primaryLan->clear();
    ui->comboBox_primaryLan->addItem(GET_TEXT("CAMERASEARCH/32022", "LAN1"), 0);
    ui->comboBox_primaryLan->addItem(GET_TEXT("CAMERASEARCH/32023", "LAN2"), 1);

    ui->comboBox_bond0Eth0Type->clear();
    ui->comboBox_bond0Eth0Type->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_bond0Eth0Type->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_eth1Type->clear();
    ui->comboBox_eth1Type->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_eth1Type->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_eth0IPv6Mode->clear();
    ui->comboBox_eth0IPv6Mode->addItem(GET_TEXT("SYSTEMNETWORK/71164", "Manual"), 0);
    ui->comboBox_eth0IPv6Mode->addItem(GET_TEXT("SYSTEMNETWORK/71155", "Router Advertisement"), 1);
    ui->comboBox_eth0IPv6Mode->addItem(GET_TEXT("SYSTEMNETWORK/71156", "DHCPv6"), 2);

    ui->comboBox_eth1IPv6Mode->clear();
    ui->comboBox_eth1IPv6Mode->addItem(GET_TEXT("SYSTEMNETWORK/71164", "Manual"), 0);
    ui->comboBox_eth1IPv6Mode->addItem(GET_TEXT("SYSTEMNETWORK/71155", "Router Advertisement"), 1);
    ui->comboBox_eth1IPv6Mode->addItem(GET_TEXT("SYSTEMNETWORK/71156", "DHCPv6"), 2);

    ui->pushButton_eth0details->hide();
    ui->pushButton_eth1details->hide();
    ui->label_eth0IPv6Rounter->hide();
    ui->label_eth1IPv6Rounter->hide();

    connect(ui->comboBox_workingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxWorkingModeChanged(int)));
    connect(ui->groupBox_eth0Enable, SIGNAL(clicked(bool)), this, SLOT(slotBond0Eth0Enable(bool)));
    connect(ui->groupBox_eth1Enable, SIGNAL(clicked(bool)), this, SLOT(slotEth1Enable(bool)));
    connect(ui->comboBox_bond0Eth0Type, SIGNAL(currentIndexChanged(int)), this, SLOT(slotBond0Eth0Type(int)));
    connect(ui->comboBox_eth1Type, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEth1Type(int)));
    connect(ui->comboBox_eth0IPv6Mode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEth0IPv6ModeChanged(int)));
    connect(ui->comboBox_eth1IPv6Mode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEth1IPv6ModeChanged(int)));
    connect(ui->pushButton_eth0details, SIGNAL(clicked(bool)), this, SLOT(slotShowEth0Details()));
    connect(ui->pushButton_eth1details, SIGNAL(clicked(bool)), this, SLOT(slotShowEth1Details()));
    connect(ui->comboBox_primaryLan, SIGNAL(currentIndexChanged(int)), this, SLOT(slotBondPrimaryComboBoxChanged(int)));
    connect(ui->pushButton_apply, SIGNAL(clicked(bool)), this, SLOT(onPushButtonApplyClicked()));
    connect(ui->pushButton_back, SIGNAL(clicked(bool)), this, SLOT(onPushButtonBackClicked()));

    ui->label_poeIPaddr->hide();
    ui->lineEdit_poeIPaddr->hide();
    if (qMsNvr->isPoe()) {
        ui->label_poeIPaddr->show();
        ui->lineEdit_poeIPaddr->show();
        ui->label_workingMode->hide();
        ui->comboBox_workingMode->hide();
    }

    ui->label_primaryLan->hide(); //hrz.milesight
    ui->comboBox_primaryLan->hide();

    QRegExp rx(QString("[0-9]*"));
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_eth0IPvPreLen->setValidator(validator);
    ui->lineEdit_eth1IPvPreLen->setValidator(validator);
    ui->lineEdit_eth0IPvPreLen->setMaxLength(3);
    ui->lineEdit_eth1IPvPreLen->setMaxLength(3);

    pNetworkDb = NULL;
    pNetworkDbOri = NULL;
    pNetworkDbStatic = NULL;
    pNetworkDbStatic = new struct network;
    _ip_conflict_type = -1;
    pRouterAdvDialog = NULL;
    read_network(SQLITE_FILE_NAME, pNetworkDbStatic);
}

void NetworkPageBasic::gotoNetBasicTab()
{
    if (pNetworkDb == NULL) {
        pNetworkDb = new struct network;
    }
    if (pNetworkDbOri == NULL) {
        pNetworkDbOri = new struct network;
    }
    if (pNetworkDbStatic == NULL) {
        pNetworkDbStatic = new struct network;
    }
    memset(pNetworkDb, 0, sizeof(struct network));
    memset(pNetworkDbOri, 0, sizeof(struct network));
    memset(pNetworkDbStatic, 0, sizeof(struct network));

    read_network(SQLITE_FILE_NAME, pNetworkDb);
    memcpy(pNetworkDbOri, pNetworkDb, sizeof(struct network));
    memcpy(pNetworkDbStatic, pNetworkDb, sizeof(struct network));

    ui->comboBox_workingMode->setCurrentIndex(pNetworkDb->mode);
    ui->comboBoxDefaultRoute->setCurrentIndexFromData(pNetworkDb->defaultRoute);
    readBasicConfig(pNetworkDb->mode);

    onComboBoxWorkingModeChanged(ui->comboBox_workingMode->currentIndex());
}

void NetworkPageBasic::freeNetBasicTab()
{
    if (pNetworkDb) {
        delete pNetworkDb;
        pNetworkDb = nullptr;
    }
    if (pNetworkDbOri) {
        delete pNetworkDbOri;
        pNetworkDbOri = nullptr;
    }
    if (pNetworkDbStatic) {
        delete pNetworkDbStatic;
        pNetworkDbStatic = nullptr;
    }
    if (pRouterAdvDialog) {
        delete pRouterAdvDialog;
        pRouterAdvDialog = nullptr;
    }
}

void NetworkPageBasic::readBasicConfig(int mode)
{
    const struct device_info &sys_info = qMsNvr->deviceInfo();

    char ifaddr[16] = { 0 };
    char ifmask[16] = { 0 };
    char ifgw[16] = { 0 };

    char ifaddr6[64] = { '\0' }; //hrz.milesight
    char ifprefix[16] = { 0 };
    char iftype;

    if (mode == NETMODE_MULTI) {
        if (sys_info.max_lan > 1) {
            ui->groupBox_eth1Enable->show();
        }

        //eth0
        if (sys_info.max_lan == 1 || qMsNvr->isPoe()) {
            ui->groupBox_eth0Enable->setTitle(GET_TEXT("CAMERASEARCH/32031", "LAN"));
        } else {
            ui->groupBox_eth0Enable->setTitle(QString("%1 %2").arg(GET_TEXT("CAMERASEARCH/32022", "LAN1")).arg(GET_TEXT("COMMON/1009", "Enable")));
        }

        if (pNetworkDb->lan1_enable) {
            ui->groupBox_eth0Enable->setChecked(true);
            slotBond0Eth0Enable(true);

            if (pNetworkDb->lan1_type == 1) {
                ui->comboBox_bond0Eth0Type->setCurrentIndex(1);
                //ui->checkBox_bond0Eth0Type->setChecked(true);
                slotBond0Eth0Type(true);
            } else {
                ui->comboBox_bond0Eth0Type->setCurrentIndex(0);
                //ui->checkBox_bond0Eth0Type->setChecked(false);
                slotBond0Eth0Type(false);
            }

        } else {
            /*
            ui->checkBox_bond0Eth0Enable->setChecked(false);
            checkBox_bond0Eth0EnableSlot(false);*/

            //因为enable隐藏掉了，所以界面没得选，必须得给开启
            ui->groupBox_eth0Enable->setChecked(false);
            slotBond0Eth0Enable(false);
        }
        if (pNetworkDb->lan1_type == 1) { //dhcp
            if (net_get_ifaddr("eth0", ifaddr, sizeof(ifaddr)) == 0)
                ui->lineEdit_eth0IP->setText(QString(ifaddr));
            else
                ui->lineEdit_eth0IP->setText(QString(pNetworkDb->lan1_ip_address));
            if (net_get_netmask("eth0", ifmask, sizeof(ifmask)) == 0)
                ui->lineEdit_eth0Netmask->setText(QString(ifmask));
            else
                ui->lineEdit_eth0Netmask->setText(QString(pNetworkDb->lan1_netmask));
            if (net_get_gateway("eth0", ifgw, sizeof(ifgw)) == 0)
                ui->lineEdit_eth0Gateway->setText(QString(ifgw));
            else
                ui->lineEdit_eth0Gateway->setText(QString(pNetworkDb->lan1_gateway));
        } else {
            ui->lineEdit_eth0IP->setText(QString(pNetworkDb->lan1_ip_address));
            ui->lineEdit_eth0Netmask->setText(QString(pNetworkDb->lan1_netmask));
            ui->lineEdit_eth0Gateway->setText(QString(pNetworkDb->lan1_gateway));
        }
        ui->lineEdit_eth0PreDns->setText(QString(pNetworkDb->lan1_primary_dns));
        ui->lineEdit_eth0AlterDns->setText(QString(pNetworkDb->lan1_second_dns));
        //mac
        ui->lineEdit_eth0MACShow->setText(QString(mac0));
        ui->lineEdit_mtu0->setText(QString("%1").arg(pNetworkDb->lan1_mtu));
        //...

        ui->comboBox_eth0IPv6Mode->setCurrentIndex(pNetworkDb->lan1_ip6_dhcp); //hrz.milesight
        if (pNetworkDb->lan1_ip6_dhcp == 0) {
            //static
            ui->lineEdit_eth0IPv6Addr->setText(QString(pNetworkDb->lan1_ip6_address));
            ui->lineEdit_eth0IPvPreLen->setText(QString(pNetworkDb->lan1_ip6_netmask));
            ui->lineEdit_eth0IPv6GateWay->setText(QString(pNetworkDb->lan1_ip6_gateway));
        } else {
            if (pNetworkDb->lan1_ip6_dhcp == 1 && pNetworkDb->lan1_ip6_address[0] != '\0' && pNetworkDb->lan1_ip6_netmask[0] != '\0') {
                ui->lineEdit_eth0IPv6Addr->setText(QString(pNetworkDb->lan1_ip6_address));
                ui->lineEdit_eth0IPvPreLen->setText(QString(pNetworkDb->lan1_ip6_netmask));
                ui->lineEdit_eth0IPv6GateWay->setText(QString(pNetworkDb->lan1_ip6_gateway));
            } else {

                if (pNetworkDb->lan1_ip6_dhcp == 1) //ra
                    iftype = 1;
                else //dhcpv6
                    iftype = 0;

                if (net_get_ipv6_ifaddr(iftype, "eth0", ifaddr6, sizeof(ifaddr6), ifprefix, sizeof(ifprefix)) == 0) {
                    ui->lineEdit_eth0IPv6Addr->setText(QString(ifaddr6));
                    ui->lineEdit_eth0IPvPreLen->setText(QString(ifprefix));
                } else {
                    ui->lineEdit_eth0IPv6Addr->clear();
                    ui->lineEdit_eth0IPvPreLen->clear();
                }

                if (net_get_ipv6_gateway2(0, "eth0", ifaddr6, sizeof(ifaddr6)) == 0)
                    ui->lineEdit_eth0IPv6GateWay->setText(QString(ifaddr6));
                else if (net_get_ipv6_gateway2(1, "eth0", ifaddr6, sizeof(ifaddr6)) == 0)
                    ui->lineEdit_eth0IPv6GateWay->setText(QString(ifaddr6));
                else
                    ui->lineEdit_eth0IPv6GateWay->clear();
            }
        } //end

        //eth1

        if (pNetworkDb->lan2_enable) {
            ui->groupBox_eth1Enable->setChecked(true);
            slotEth1Enable(true);

            if (pNetworkDb->lan2_type == 1) {
                ui->comboBox_eth1Type->setCurrentIndex(1);
                //ui->checkBox_eth1Type->setChecked(true);
                slotEth1Type(true);
            } else {
                ui->comboBox_eth1Type->setCurrentIndex(0);
                //ui->checkBox_eth1Type->setChecked(false);
                slotEth1Type(false);
            }
        } else {
            ui->groupBox_eth1Enable->setChecked(false);
            slotEth1Enable(false);
        }
        if (pNetworkDb->lan2_type == 1) { //dhcp
            if (net_get_ifaddr("eth1", ifaddr, sizeof(ifaddr)) == 0)
                ui->lineEdit_eth1IP->setText(QString(ifaddr));
            else
                ui->lineEdit_eth1IP->setText(QString(pNetworkDb->lan2_ip_address));
            if (net_get_netmask("eth1", ifmask, sizeof(ifmask)) == 0)
                ui->lineEdit_eth1Netmask->setText(QString(ifmask));
            else
                ui->lineEdit_eth1Netmask->setText(QString(pNetworkDb->lan2_netmask));
            if (net_get_gateway("eth1", ifgw, sizeof(ifgw)) == 0)
                ui->lineEdit_eth1Gateway->setText(QString(ifgw));
            else
                ui->lineEdit_eth1Gateway->setText(QString(pNetworkDb->lan2_gateway));
        } else {
            ui->lineEdit_eth1IP->setText(QString(pNetworkDb->lan2_ip_address));
            ui->lineEdit_eth1Netmask->setText(QString(pNetworkDb->lan2_netmask));
            ui->lineEdit_eth1Gateway->setText(QString(pNetworkDb->lan2_gateway));
        }
        ui->lineEdit_eth1PreDns->setText(QString(pNetworkDb->lan2_primary_dns));
        ui->lineEdit_eth1AlterDns->setText(QString(pNetworkDb->lan2_second_dns));

        ui->comboBox_eth1IPv6Mode->setCurrentIndex(pNetworkDb->lan2_ip6_dhcp); //hrz.milesight
        if (pNetworkDb->lan2_ip6_dhcp == 0) {
            //static
            ui->lineEdit_eth1IPv6Addr->setText(QString(pNetworkDb->lan2_ip6_address));
            ui->lineEdit_eth1IPvPreLen->setText(QString(pNetworkDb->lan2_ip6_netmask));
            ui->lineEdit_eth1IPv6GateWay->setText(QString(pNetworkDb->lan2_ip6_gateway));
        } else {
            if (pNetworkDb->lan2_ip6_dhcp == 1 && pNetworkDb->lan2_ip6_address[0] != '\0' && pNetworkDb->lan2_ip6_netmask[0] != '\0') {
                ui->lineEdit_eth1IPv6Addr->setText(QString(pNetworkDb->lan2_ip6_address));
                ui->lineEdit_eth1IPvPreLen->setText(QString(pNetworkDb->lan2_ip6_netmask));
                ui->lineEdit_eth1IPv6GateWay->setText(QString(pNetworkDb->lan2_ip6_gateway));
            } else {

                if (pNetworkDb->lan2_ip6_dhcp == 1) //ra
                    iftype = 1;
                else //dhcpv6
                    iftype = 0;

                if (net_get_ipv6_ifaddr(iftype, "eth1", ifaddr6, sizeof(ifaddr6), ifprefix, sizeof(ifprefix)) == 0) {
                    ui->lineEdit_eth1IPv6Addr->setText(QString(ifaddr6));
                    ui->lineEdit_eth1IPvPreLen->setText(QString(ifprefix));
                } else {
                    ui->lineEdit_eth1IPv6Addr->clear();
                    ui->lineEdit_eth1IPvPreLen->clear();
                }

                if (net_get_ipv6_gateway2(0, "eth1", ifaddr6, sizeof(ifaddr6)) == 0)
                    ui->lineEdit_eth1IPv6GateWay->setText(QString(ifaddr6));
                else if (net_get_ipv6_gateway2(1, "eth1", ifaddr6, sizeof(ifaddr6)) == 0)
                    ui->lineEdit_eth1IPv6GateWay->setText(QString(ifaddr6));
                else
                    ui->lineEdit_eth1IPv6GateWay->clear();
            }
        } //end

        //mac
        ui->lineEdit_eth1MACShow->setText(QString(mac1));
        ui->lineEdit_mtu1->setText(QString("%1").arg(pNetworkDb->lan2_mtu));
        //...
        if (qMsNvr->isPoe()) {
            ui->groupBox_eth1Enable->hide();
            ui->lineEdit_poeIPaddr->setText(QString(pNetworkDb->lan2_ip_address));
            ui->groupBox_eth1Enable->setCheckable(false);
        }
    } else if (mode == NETMODE_BACKUP || mode == NETMODE_LOADBALANCE) {
        ui->groupBox_eth1Enable->hide();

        //bond0
        if (pNetworkDb->bond0_enable) {
            ui->groupBox_eth0Enable->setChecked(true);
            slotBond0Eth0Enable(true);

            if (pNetworkDb->bond0_type == 1) {
                //ui->checkBox_bond0Eth0Type->setChecked(true);
                ui->comboBox_bond0Eth0Type->setCurrentIndex(1);
                slotBond0Eth0Type(true);
            } else {
                //ui->checkBox_bond0Eth0Type->setChecked(false);
                ui->comboBox_bond0Eth0Type->setCurrentIndex(0);
                slotBond0Eth0Type(false);
            }
        } else {
            ui->groupBox_eth0Enable->setChecked(false);
            slotBond0Eth0Enable(false);
        }

        ui->groupBox_eth0Enable->setChecked(true);
        slotBond0Eth0Enable(true);
        if (pNetworkDb->bond0_type == 1) {
            ui->comboBox_bond0Eth0Type->setCurrentIndex(1);
            //ui->checkBox_bond0Eth0Type->setChecked(true);
            slotBond0Eth0Type(true);
        } else {
            ui->comboBox_bond0Eth0Type->setCurrentIndex(0);
            //ui->checkBox_bond0Eth0Type->setChecked(false);
            slotBond0Eth0Type(false);
        }

        if (pNetworkDb->bond0_type == 1) { //dhcp
            if (net_get_ifaddr("bond0", ifaddr, sizeof(ifaddr)) == 0)
                ui->lineEdit_eth0IP->setText(QString(ifaddr));
            else
                ui->lineEdit_eth0IP->setText(QString(pNetworkDb->bond0_ip_address));
            if (net_get_netmask("bond0", ifmask, sizeof(ifmask)) == 0)
                ui->lineEdit_eth0Netmask->setText(QString(ifmask));
            else
                ui->lineEdit_eth0Netmask->setText(QString(pNetworkDb->bond0_netmask));
            if (net_get_gateway("bond0", ifgw, sizeof(ifgw)) == 0)
                ui->lineEdit_eth0Gateway->setText(QString(ifgw));
            else
                ui->lineEdit_eth0Gateway->setText(QString(pNetworkDb->bond0_gateway));
        } else {
            ui->lineEdit_eth0IP->setText(QString(pNetworkDb->bond0_ip_address));
            ui->lineEdit_eth0Netmask->setText(QString(pNetworkDb->bond0_netmask));
            ui->lineEdit_eth0Gateway->setText(QString(pNetworkDb->bond0_gateway));
        }

        ui->comboBox_eth0IPv6Mode->setCurrentIndex(pNetworkDb->bond0_ip6_dhcp); //hrz.milesight
        if (pNetworkDb->bond0_ip6_dhcp == 0) {
            ui->lineEdit_eth0IPv6Addr->setText(QString(pNetworkDb->bond0_ip6_address));
            ui->lineEdit_eth0IPvPreLen->setText(QString(pNetworkDb->bond0_ip6_netmask));
            ui->lineEdit_eth0IPv6GateWay->setText(QString(pNetworkDb->bond0_ip6_gateway));
        } else {
            if (pNetworkDb->bond0_ip6_dhcp == 1 && pNetworkDb->bond0_ip6_address[0] != '\0' && pNetworkDb->bond0_ip6_netmask[0] != '\0') {
                ui->lineEdit_eth0IPv6Addr->setText(QString(pNetworkDb->bond0_ip6_address));
                ui->lineEdit_eth0IPvPreLen->setText(QString(pNetworkDb->bond0_ip6_netmask));
                ui->lineEdit_eth0IPv6GateWay->setText(QString(pNetworkDb->bond0_ip6_gateway));
            } else {
                if (pNetworkDb->bond0_ip6_dhcp == 1) //ra
                    iftype = 1;
                else //dhcpv6
                    iftype = 0;

                if (net_get_ipv6_ifaddr(iftype, "bond0", ifaddr6, sizeof(ifaddr6), ifprefix, sizeof(ifprefix)) == 0) {
                    ui->lineEdit_eth0IPv6Addr->setText(QString(ifaddr6));
                    ui->lineEdit_eth0IPvPreLen->setText(QString(ifprefix));
                } else {
                    ui->lineEdit_eth0IPv6Addr->clear();
                    ui->lineEdit_eth0IPvPreLen->clear();
                }

                if (net_get_ipv6_gateway2(0, "bond0", ifaddr6, sizeof(ifaddr6)) == 0)
                    ui->lineEdit_eth0IPv6GateWay->setText(QString(ifaddr6));
                else if (net_get_ipv6_gateway2(1, "bond0", ifaddr6, sizeof(ifaddr6)) == 0)
                    ui->lineEdit_eth0IPv6GateWay->setText(QString(ifaddr6));
                else
                    ui->lineEdit_eth0IPv6GateWay->clear();
            }
        } //end

        ui->lineEdit_eth0PreDns->setText(QString(pNetworkDb->bond0_primary_dns));
        ui->lineEdit_eth0AlterDns->setText(QString(pNetworkDb->bond0_second_dns));

        ui->comboBox_primaryLan->setCurrentIndex(pNetworkDb->bond0_primary_net); //0-eth0 ,1-eth1
        //mac 根据primary
        if (mode == NETMODE_BACKUP) {
            if (ui->comboBox_primaryLan->currentIndex() == 0) {
                ui->lineEdit_eth0MACShow->setText(QString(mac0));
            } else {
                ui->lineEdit_eth0MACShow->setText(QString(mac1));
            }
        } else {
            ui->lineEdit_eth0MACShow->setText(QString(mac0));
        }
        ui->lineEdit_mtu0->setText(QString("%1").arg(pNetworkDb->bond0_mtu));
    }
}

void NetworkPageBasic::saveBasicConfig()
{
    pNetworkDb->mode = (NETMODE)ui->comboBox_workingMode->currentIndex();
    if (ui->comboBoxDefaultRoute->isVisible()) {
        pNetworkDb->defaultRoute = ui->comboBoxDefaultRoute->currentData().toInt();
    }
    if (pNetworkDb->mode == NETMODE_MULTI) {
        //eth0
        if ((ui->groupBox_eth0Enable->isCheckable() && ui->groupBox_eth0Enable->isChecked()) || !ui->groupBox_eth0Enable->isCheckable()) {
            pNetworkDb->lan1_enable = 1;
        } else {
            pNetworkDb->lan1_enable = 0;
        }
        if (qMsNvr->isPoe() || qMsNvr->deviceInfo().max_lan == 1) {
            pNetworkDb->lan1_enable = 1;
        }
        //if(ui->checkBox_bond0Eth0Type->isChecked()){
        if (ui->comboBox_bond0Eth0Type->currentIndex() == 1) {
            pNetworkDb->lan1_type = 1;
        } else {
            pNetworkDb->lan1_type = 0;
        }

        if (ui->lineEdit_eth0IP->text().trimmed() == "...") {
            snprintf(pNetworkDb->lan1_ip_address, sizeof(pNetworkDb->lan1_ip_address), "%s", "");
        } else {
            snprintf(pNetworkDb->lan1_ip_address, sizeof(pNetworkDb->lan1_ip_address), "%s", ui->lineEdit_eth0IP->text().trimmed().toStdString().c_str());
        }

        if (ui->lineEdit_eth0Netmask->text().trimmed() == "...") {
            snprintf(pNetworkDb->lan1_netmask, sizeof(pNetworkDb->lan1_netmask), "%s", "");
        } else {
            snprintf(pNetworkDb->lan1_netmask, sizeof(pNetworkDb->lan1_netmask), "%s", ui->lineEdit_eth0Netmask->text().trimmed().toStdString().c_str());
        }

        if (ui->lineEdit_eth0Gateway->text().trimmed() == "...") {
            snprintf(pNetworkDb->lan1_gateway, sizeof(pNetworkDb->lan1_gateway), "%s", "");
        } else {
            snprintf(pNetworkDb->lan1_gateway, sizeof(pNetworkDb->lan1_gateway), "%s", ui->lineEdit_eth0Gateway->text().trimmed().toStdString().c_str());
        }

        if (ui->lineEdit_eth0PreDns->text().trimmed() == "...") {
            snprintf(pNetworkDb->lan1_primary_dns, sizeof(pNetworkDb->lan1_primary_dns), "%s", "");
        } else {
            snprintf(pNetworkDb->lan1_primary_dns, sizeof(pNetworkDb->lan1_primary_dns), "%s", ui->lineEdit_eth0PreDns->text().trimmed().toStdString().c_str());
        }

        if (ui->lineEdit_eth0AlterDns->text().trimmed() == "...") {
            snprintf(pNetworkDb->lan1_second_dns, sizeof(pNetworkDb->lan1_second_dns), "%s", "");
        } else {
            snprintf(pNetworkDb->lan1_second_dns, sizeof(pNetworkDb->lan1_second_dns), "%s", ui->lineEdit_eth0AlterDns->text().trimmed().toStdString().c_str());
        }
        pNetworkDb->lan1_mtu = ui->lineEdit_mtu0->text().toInt();

        pNetworkDb->lan1_ip6_dhcp = ui->comboBox_eth0IPv6Mode->currentIndex(); //hrz.milesight
        if (pNetworkDb->lan1_ip6_dhcp == 2) {
            memset(pNetworkDb->lan1_ip6_address, 0x0, sizeof(pNetworkDb->lan1_ip6_address));
            memset(pNetworkDb->lan1_ip6_netmask, 0x0, sizeof(pNetworkDb->lan1_ip6_netmask));
        } else {
            snprintf(pNetworkDb->lan1_ip6_address, sizeof(pNetworkDb->lan1_ip6_address), "%s", ui->lineEdit_eth0IPv6Addr->text().toStdString().c_str());
            snprintf(pNetworkDb->lan1_ip6_netmask, sizeof(pNetworkDb->lan1_ip6_netmask), "%s", ui->lineEdit_eth0IPvPreLen->text().toStdString().c_str());
        }
        if (pNetworkDb->lan1_ip6_dhcp != 2)
            snprintf(pNetworkDb->lan1_ip6_gateway, sizeof(pNetworkDb->lan1_ip6_gateway), "%s", ui->lineEdit_eth0IPv6GateWay->text().toStdString().c_str());

        //eth1
        if (qMsNvr->isPoe()) {
            pNetworkDb->lan2_enable = 1;
            if (ui->lineEdit_poeIPaddr->text().trimmed() == "...") {
                snprintf(pNetworkDb->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address), "%s", "");
            } else {
                snprintf(pNetworkDb->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address), "%s", ui->lineEdit_poeIPaddr->text().trimmed().toStdString().c_str());
            }
        } else {
            if (ui->groupBox_eth1Enable->isChecked()) {
                pNetworkDb->lan2_enable = 1;
            } else {
                pNetworkDb->lan2_enable = 0;
            }

            if (ui->comboBox_eth1Type->currentIndex() == 1) {
                pNetworkDb->lan2_type = 1;
            } else {
                pNetworkDb->lan2_type = 0;
            }

            if (ui->lineEdit_eth1IP->text().trimmed() == "...") {
                snprintf(pNetworkDb->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address), "%s", "");
            } else {
                snprintf(pNetworkDb->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address), "%s", ui->lineEdit_eth1IP->text().trimmed().toStdString().c_str());
            }

            if (ui->lineEdit_eth1Netmask->text().trimmed() == "...") {
                snprintf(pNetworkDb->lan2_netmask, sizeof(pNetworkDb->lan2_netmask), "%s", "");
            } else {
                snprintf(pNetworkDb->lan2_netmask, sizeof(pNetworkDb->lan2_netmask), "%s", ui->lineEdit_eth1Netmask->text().trimmed().toStdString().c_str());
            }

            if (ui->lineEdit_eth1Gateway->text().trimmed() == "...") {
                snprintf(pNetworkDb->lan2_gateway, sizeof(pNetworkDb->lan2_gateway), "%s", "");
            } else {
                snprintf(pNetworkDb->lan2_gateway, sizeof(pNetworkDb->lan2_gateway), "%s", ui->lineEdit_eth1Gateway->text().trimmed().toStdString().c_str());
            }

            if (ui->lineEdit_eth1PreDns->text().trimmed() == "...") {
                snprintf(pNetworkDb->lan2_primary_dns, sizeof(pNetworkDb->lan2_primary_dns), "%s", "");
            } else {
                snprintf(pNetworkDb->lan2_primary_dns, sizeof(pNetworkDb->lan2_primary_dns), "%s", ui->lineEdit_eth1PreDns->text().trimmed().toStdString().c_str());
            }

            if (ui->lineEdit_eth1AlterDns->text().trimmed() == "...") {
                snprintf(pNetworkDb->lan2_second_dns, sizeof(pNetworkDb->lan2_second_dns), "%s", "");
            } else {
                snprintf(pNetworkDb->lan2_second_dns, sizeof(pNetworkDb->lan2_second_dns), "%s", ui->lineEdit_eth1AlterDns->text().trimmed().toStdString().c_str());
            }
            pNetworkDb->lan2_mtu = ui->lineEdit_mtu1->text().toInt();

            pNetworkDb->lan2_ip6_dhcp = ui->comboBox_eth1IPv6Mode->currentIndex(); //hrz.milesight

            if (pNetworkDb->lan2_ip6_dhcp == 2) {
                memset(pNetworkDb->lan2_ip6_address, 0x0, sizeof(pNetworkDb->lan2_ip6_address));
                memset(pNetworkDb->lan2_ip6_netmask, 0x0, sizeof(pNetworkDb->lan2_ip6_netmask));
            } else {
                snprintf(pNetworkDb->lan2_ip6_address, sizeof(pNetworkDb->lan2_ip6_address), "%s", ui->lineEdit_eth1IPv6Addr->text().toStdString().c_str());
                snprintf(pNetworkDb->lan2_ip6_netmask, sizeof(pNetworkDb->lan2_ip6_netmask), "%s", ui->lineEdit_eth1IPvPreLen->text().toStdString().c_str());
            }

            if (pNetworkDb->lan2_ip6_dhcp != 2)
                snprintf(pNetworkDb->lan2_ip6_gateway, sizeof(pNetworkDb->lan2_ip6_gateway), "%s", ui->lineEdit_eth1IPv6GateWay->text().toStdString().c_str());
        }
    } else {
        //bond0
        if ((ui->groupBox_eth0Enable->isCheckable() && ui->groupBox_eth0Enable->isChecked()) || !ui->groupBox_eth0Enable->isCheckable()) {
            pNetworkDb->bond0_enable = 1;
        } else {
            pNetworkDb->bond0_enable = 0;
        }

        //if(ui->checkBox_bond0Eth0Type->isChecked()){
        if (ui->comboBox_bond0Eth0Type->currentIndex() == 1) {
            pNetworkDb->bond0_type = 1;
        } else {
            pNetworkDb->bond0_type = 0;
        }

        if (ui->lineEdit_eth0IP->text().trimmed() == "...") {
            snprintf(pNetworkDb->bond0_ip_address, sizeof(pNetworkDb->bond0_ip_address), "%s", "");
        } else {
            snprintf(pNetworkDb->bond0_ip_address, sizeof(pNetworkDb->bond0_ip_address), "%s", ui->lineEdit_eth0IP->text().trimmed().toStdString().c_str());
        }

        if (ui->lineEdit_eth0Netmask->text().trimmed() == "...") {
            snprintf(pNetworkDb->bond0_netmask, sizeof(pNetworkDb->bond0_netmask), "%s", "");
        } else {
            snprintf(pNetworkDb->bond0_netmask, sizeof(pNetworkDb->bond0_netmask), "%s", ui->lineEdit_eth0Netmask->text().trimmed().toStdString().c_str());
        }

        if (ui->lineEdit_eth0Gateway->text().trimmed() == "...") {
            snprintf(pNetworkDb->bond0_gateway, sizeof(pNetworkDb->bond0_gateway), "%s", "");
        } else {
            snprintf(pNetworkDb->bond0_gateway, sizeof(pNetworkDb->bond0_gateway), "%s", ui->lineEdit_eth0Gateway->text().trimmed().toStdString().c_str());
        }

        if (ui->lineEdit_eth0PreDns->text().trimmed() == "...") {
            snprintf(pNetworkDb->bond0_primary_dns, sizeof(pNetworkDb->bond0_primary_dns), "%s", "");
        } else {
            snprintf(pNetworkDb->bond0_primary_dns, sizeof(pNetworkDb->bond0_primary_dns), "%s", ui->lineEdit_eth0PreDns->text().trimmed().toStdString().c_str());
        }

        if (ui->lineEdit_eth0AlterDns->text().trimmed() == "...") {
            snprintf(pNetworkDb->bond0_second_dns, sizeof(pNetworkDb->bond0_second_dns), "%s", "");
        } else {
            snprintf(pNetworkDb->bond0_second_dns, sizeof(pNetworkDb->bond0_second_dns), "%s", ui->lineEdit_eth0AlterDns->text().trimmed().toStdString().c_str());
        }

        pNetworkDb->bond0_mtu = ui->lineEdit_mtu0->text().toInt();
        pNetworkDb->bond0_primary_net = ui->comboBox_primaryLan->currentIndex();
        if (ui->comboBox_primaryLan->currentIndex() == 0) {
            //mac
        } else {
        }

        pNetworkDb->bond0_ip6_dhcp = ui->comboBox_eth0IPv6Mode->currentIndex(); //hrz.milesight

        if (pNetworkDb->bond0_ip6_dhcp == 2) {
            memset(pNetworkDb->bond0_ip6_address, 0x0, sizeof(pNetworkDb->bond0_ip6_address));
            memset(pNetworkDb->bond0_ip6_netmask, 0x0, sizeof(pNetworkDb->bond0_ip6_netmask));
        } else {
            snprintf(pNetworkDb->bond0_ip6_address, sizeof(pNetworkDb->bond0_ip6_address), "%s", ui->lineEdit_eth0IPv6Addr->text().toStdString().c_str());
            snprintf(pNetworkDb->bond0_ip6_netmask, sizeof(pNetworkDb->bond0_ip6_netmask), "%s", ui->lineEdit_eth0IPvPreLen->text().toStdString().c_str());
        }
        if (pNetworkDb->bond0_ip6_dhcp != 2)
            snprintf(pNetworkDb->bond0_ip6_gateway, sizeof(pNetworkDb->bond0_ip6_gateway), "%s", ui->lineEdit_eth0IPv6GateWay->text().toStdString().c_str());
    }
}

void NetworkPageBasic::ON_RESPONSE_FLAG_TEST_IP_CONFLICT(MessageReceive *message)
{
    //closeWait();
    if (!message->data) {
        qWarning() << "NetworkPageBasic::ON_RESPONSE_FLAG_TEST_IP_CONFLICT, data is null.";
        return;
    }
    int ret = *((int *)message->data);
    struct ip_conflict conflict;
    memset(&conflict, 0, sizeof(struct ip_conflict));
    int mode = (NETMODE)ui->comboBox_workingMode->currentIndex();
    _ipConflict = 0;
    if (_ip_conflict_type == 0) //bond0 ip conflict check
    {
        if (ret == -1) {
            //冲突  ，进行相关操作
            if (MessageBox::Yes == MessageBox::question(this, GET_TEXT("SYSTEMNETWORK/71060", "Detect Bond0 IP Conflict, Continue?"))) {
                //不管冲突，任性修改，滴滴  重启吧
                _ipConflict = 1;
                writeBasicConfig(mode);
                return;
            } else {
                //冲突，修改吧
                return;
            }
        } else {
            //不冲突  跳转页面
            writeBasicConfig(mode);
            return;
        }
    } else if (_ip_conflict_type == 1) //lan1 ip conflict check
    {
        if (ret == -1) {
            if (MessageBox::Yes == MessageBox::question(this, GET_TEXT("SYSTEMNETWORK/71061", "Detect LAN1 IP Conflict, Continue?"))) {
                //冲突，任性
                _ipConflict = 1;
                if (pNetworkDb->lan2_enable && !qMsNvr->isPoe()) {
                    if (memcmp(pNetworkDb->lan2_ip_address, pNetworkDbOri->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address)) != 0) {
                        snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_ETH1);
                        snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->lan2_ip_address);
                        sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                        _ip_conflict_type = 2;
                        //showWait();
                        return;
                    } else {
                        writeBasicConfig(mode);
                        return;
                    }
                } else {
                    writeBasicConfig(mode);
                    return;
                }
            } else {
                //想修改
                return;
            }
        } else {
            if (pNetworkDb->lan2_enable && !qMsNvr->isPoe()) {
                if (memcmp(pNetworkDb->lan2_ip_address, pNetworkDbOri->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address)) != 0) {
                    //guiSendMsg(SOCKET_TYPE_CORE, REQUEST_FLAG_TEST_IP_CONFLICT, sizeof(pNetworkDb->lan2_ip_address), (void *)&pNetworkDb->lan2_ip_address);
                    snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_ETH1);
                    snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->lan2_ip_address);
                    sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                    _ip_conflict_type = 2;
                    //showWait();
                    return;
                } else {
                    writeBasicConfig(mode);
                    return;
                }
            } else {
                writeBasicConfig(mode);
                return;
            }
        }
    } else if (_ip_conflict_type == 2) //lan2 ip conflict check
    {
        if (ret == -1) {
            //冲突  ，进行相关操作
            QString strMessage;
            if (qMsNvr->isPoe())
                strMessage = GET_TEXT("SYSTEMNETWORK/71114", "Detect PoE NIC IPv4 Address Conflict, Continue?");
            else
                strMessage = GET_TEXT("SYSTEMNETWORK/71062", "Detect LAN2 IP Conflict, Continue?");
            if (MessageBox::Yes == MessageBox::question(this, strMessage)) {
                //不管冲突，修改
                _ipConflict = 1;
                writeBasicConfig(mode);
                return;
            } else {
                //冲突，修改
                return;
            }
        } else {
            //不冲突  跳转页面
            writeBasicConfig(mode);
            return;
        }
    }
}

void NetworkPageBasic::writeBasicConfig(int mode)
{
    struct log_data log_data;
    int failover = 0;
    if (!dealConfirmLoadBalance(mode)) {
        if (failoverMode == 1 && isHotspareWorking(mode)) {
            QString strMessage = GET_TEXT("HOTSPARE/79041", "After the IP is modified, you need to reconfigure the Slave IP of each Master. Continue?");
            if (MessageBox::Cancel == MessageBox::question(this, strMessage)) {
                return;
            }
            failover = 1;
        }

        QString strMessage = GET_TEXT("SYSTEMNETWORK/71063", "The modifications will take effect after reboot. Do you want to reboot now?");
        if (MessageBox::Yes == MessageBox::question(this, strMessage)) {
            memcpy(pNetworkDbOri, pNetworkDb, sizeof(struct network));
            write_network(SQLITE_FILE_NAME, pNetworkDb);

            struct req_set_sysconf timeconf;
            memset(&timeconf, 0, sizeof(timeconf));
            snprintf(timeconf.arg, sizeof(timeconf.arg), "%s", "network"); //network
            if (failover)
            {
                sendMessageOnly(REQUEST_FLAG_FAILOVER_CHANGE_IP, (void *)&failover, sizeof(int));
            }
            sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&timeconf, sizeof(timeconf));

            //write ip conflict log
            if (_ipConflict == 1) {
                memset(&log_data, 0, sizeof(struct log_data));
                snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
                log_data.log_data_info.subType = SUB_EXCEPT_IPC_CONFLICT;
                log_data.log_data_info.parameter_type = SUB_PARAM_NETWORK;
                MsWriteLog(log_data);
            }

            sendMessageOnly(REQUEST_FLAG_SET_NETWORK_RESTART, (void *)&timeconf, sizeof(timeconf));
            ////showWait();
        }
    }
}

int NetworkPageBasic::isHotspareWorking(int mode)
{
    //slave mode and change ip
    QString ip1, ip2, old_ip1, old_ip2, bond_ip1, old_bond_ip1;

    ip1 = QString(pNetworkDb->lan1_ip_address);
    ip2 = QString(pNetworkDb->lan2_ip_address);
    bond_ip1 = QString(pNetworkDb->bond0_ip_address);
    old_ip1 = QString(pNetworkDbOri->lan1_ip_address);
    old_ip2 = QString(pNetworkDbOri->lan2_ip_address);
    old_bond_ip1 = QString(pNetworkDbOri->bond0_ip_address);

    if (mode == NETMODE_MULTI) {
        if (pNetworkDb->lan1_enable == 1 && pNetworkDb->lan1_type == 1) {
            //lan1 dhcp
            return 1;
        } else if (pNetworkDb->lan2_enable == 1 && pNetworkDb->lan2_type == 1) {
            //lan2 dhcp
            return 1;
        } else if (pNetworkDb->lan1_enable != pNetworkDbOri->lan1_enable) {
            return 1;
        } else if (pNetworkDb->lan2_enable != pNetworkDbOri->lan2_enable) {
            return 1;
        } else if (ip1 != old_ip1 || ip2 != old_ip2) {
            return 1;
        }
    } else {
        if (pNetworkDb->bond0_enable == 1 && pNetworkDb->bond0_type == 1) {
            return 1;
        } else if (bond_ip1 != old_bond_ip1) {
            return 1;
        }
    }

    return 0;
}

bool NetworkPageBasic::isValidatorInput()
{
    char realIPv6[64] = { 0 };
    char tmpIPv6[64] = { 0 };

    bool valid = true;
    int mode = (NETMODE)ui->comboBox_workingMode->currentIndex();

    QString eth0Ip = ui->lineEdit_eth0IP->text().trimmed();
    if (ui->lineEdit_eth0IP->isEnabled()) {
        valid = ui->lineEdit_eth0IP->checkValid();
        if (valid) {
            QStringList ipList = eth0Ip.split(".");
            if (ipList.size() != 4) {
                qMsWarning() << "invalid ip:" << eth0Ip;
            } else {
                int ip1 = ipList.first().toInt();
                if (ip1 < 1 || ip1 > 223) {
                    ui->lineEdit_eth0IP->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
                    valid = false;
                }
            }
        }
    }

    if (ui->lineEdit_mtu0->isEnabled() && !ui->lineEdit_mtu0->checkValid()) {
        valid = false;
    }

    if (ui->lineEdit_eth0Netmask->isEnabled() && !ui->lineEdit_eth0Netmask->checkValid()) {
        valid = false;
    }

    if (ui->lineEdit_eth0Gateway->isEnabled() && !ui->lineEdit_eth0Gateway->checkValid()) {
        valid = false;
    }
    if (ui->lineEdit_eth0PreDns->isEnabled() && !ui->lineEdit_eth0PreDns->checkValid()) {
        valid = false;
    }
    if (!ui->lineEdit_eth0AlterDns->text().trimmed().isEmpty()) {
        qDebug() << "[david debug] AlterDns" << ui->lineEdit_eth0AlterDns->text();
        if (ui->lineEdit_eth0AlterDns->isEnabled() && !ui->lineEdit_eth0AlterDns->checkValid()) {
            valid = false;
        }
    }

    if ((ui->groupBox_eth0Enable->isCheckable() && ui->groupBox_eth0Enable->isChecked()) || !ui->groupBox_eth0Enable->isCheckable()) {
        if (ui->comboBox_eth0IPv6Mode->currentIndex() == 0) {
            //manual
            if (!ui->lineEdit_eth0IPv6Addr->text().isEmpty()) {
                //ipv4 transfor ipv6
                //::ffff:192.168.89.9 --> 0000:0000:0000:0000:0000:ffff:c0a8:5909 -->::ffff:c0a8:5909
                snprintf(tmpIPv6, sizeof(tmpIPv6), "%s", ui->lineEdit_eth0IPv6Addr->text().toStdString().c_str());
                if (net_ipv4_trans_ipv6(tmpIPv6, realIPv6, sizeof(realIPv6)) == 0) {
                    ui->lineEdit_eth0IPv6Addr->setText(QString(realIPv6));
                }

                if (!ui->lineEdit_eth0IPv6Addr->checkValid()) {
                    valid = false;
                }
            }

            if (ui->lineEdit_eth0IPvPreLen->text().isEmpty()) {
                if (!ui->lineEdit_eth0IPv6Addr->text().isEmpty()) {
                    ui->lineEdit_eth0IPvPreLen->setCustomValid(false, GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                    valid = false;
                }
            } else if (!ui->lineEdit_eth0IPvPreLen->checkValid()) {
                valid = false;
            }

            if (!ui->lineEdit_eth0IPv6GateWay->text().isEmpty()) {
                snprintf(tmpIPv6, sizeof(tmpIPv6), "%s", ui->lineEdit_eth0IPv6GateWay->text().toStdString().c_str());
                if (net_ipv4_trans_ipv6(tmpIPv6, realIPv6, sizeof(realIPv6)) == 0) {
                    ui->lineEdit_eth0IPv6GateWay->setText(QString(realIPv6));
                }

                if (!ui->lineEdit_eth0IPv6GateWay->checkValid()) {
                    valid = false;
                }
            }
        }
    }
    if (qMsNvr->isPoe()) {
        if (ui->lineEdit_poeIPaddr->isEnabled() && !ui->lineEdit_poeIPaddr->checkValid()) {
            valid = false;
        }
    }
    if (mode == NETMODE_MULTI && !qMsNvr->isPoe()) {
        QString eth0Ip = ui->lineEdit_eth1IP->text().trimmed();
        if (ui->lineEdit_eth1IP->isEnabled()) {
            if (!ui->lineEdit_eth1IP->checkValid()) {
                valid = false;
            }
            if (ui->lineEdit_eth1IP->checkValid()) {
                QStringList ipList = eth0Ip.split(".");
                if (ipList.size() != 4) {
                    qMsWarning() << "invalid ip:" << eth0Ip;
                } else {
                    int ip1 = ipList.first().toInt();
                    if (ip1 < 1 || ip1 > 223) {
                        ui->lineEdit_eth1IP->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
                        valid = false;
                    }
                }
            }
        }
        if (ui->lineEdit_mtu1->isEnabled() && !ui->lineEdit_mtu1->checkValid()) {
            valid = false;
        }

        if (ui->lineEdit_eth1Netmask->isEnabled() && !ui->lineEdit_eth1Netmask->checkValid()) {
            valid = false;
        }

        if (ui->lineEdit_eth1Gateway->isEnabled() && !ui->lineEdit_eth1Gateway->checkValid()) {
            valid = false;
        }
        if (ui->lineEdit_eth1PreDns->isEnabled() && !ui->lineEdit_eth1PreDns->checkValid()) {
            valid = false;
        }
        if (!ui->lineEdit_eth1AlterDns->text().trimmed().isEmpty()) {
            if (ui->lineEdit_eth1AlterDns->isEnabled() && !ui->lineEdit_eth1AlterDns->checkValid()) {
                valid = false;
            }
        }

        if (ui->groupBox_eth1Enable->isChecked()) //hrz.milesight here check ipv6
        {
            if (ui->comboBox_eth1IPv6Mode->currentIndex() == 0) {
                //manual
                if (!ui->lineEdit_eth1IPv6Addr->text().isEmpty()) {
                    snprintf(tmpIPv6, sizeof(tmpIPv6), "%s", ui->lineEdit_eth1IPv6Addr->text().toStdString().c_str());
                    if (net_ipv4_trans_ipv6(tmpIPv6, realIPv6, sizeof(realIPv6)) == 0) {
                        ui->lineEdit_eth1IPv6Addr->setText(QString(realIPv6));
                    }

                    if (!ui->lineEdit_eth1IPv6Addr->checkValid()) {
                        valid = false;
                    }
                }

                if (ui->lineEdit_eth1IPvPreLen->text().isEmpty()) {
                    if (!ui->lineEdit_eth1IPv6Addr->text().isEmpty()) {
                        ui->lineEdit_eth1IPvPreLen->setCustomValid(false, GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                        valid = false;
                    }
                } else if (!ui->lineEdit_eth1IPvPreLen->checkValid()) {
                    valid = false;
                }

                if (!ui->lineEdit_eth1IPv6GateWay->text().isEmpty()) {
                    snprintf(tmpIPv6, sizeof(tmpIPv6), "%s", ui->lineEdit_eth1IPv6GateWay->text().toStdString().c_str());
                    if (net_ipv4_trans_ipv6(tmpIPv6, realIPv6, sizeof(realIPv6)) == 0) {
                        ui->lineEdit_eth1IPv6GateWay->setText(QString(realIPv6));
                    }
                    if (!ui->lineEdit_eth1IPv6GateWay->checkValid()) {
                        valid = false;
                    }
                }
            }
        }
    }

    return valid;
}

void NetworkPageBasic::getNetBasicInfo(void *data)
{
    struct network *info;
    char iftype;

    if (data) {
        info = (struct network *)data;
        read_network(SQLITE_FILE_NAME, info);

        if (info->mode == NETMODE_MULTI) {
            if (info->lan1_ip6_dhcp != 0) {
                if (info->lan1_ip6_dhcp == 1 && info->lan1_ip6_address[0] != '\0' && info->lan1_ip6_netmask[0] != '\0') {
                    ;
                } else {

                    if (info->lan1_ip6_dhcp == 1) //ra
                        iftype = 1;
                    else //dhcpv6
                        iftype = 0;

                    net_get_ipv6_ifaddr(iftype, "eth0", info->lan1_ip6_address, sizeof(info->lan1_ip6_address),
                                        info->lan1_ip6_netmask, sizeof(info->lan1_ip6_netmask));
                    net_get_ipv6_gateway2(1, "eth0", info->lan1_ip6_gateway, sizeof(info->lan1_ip6_gateway));
                }
            }

            if (info->lan2_ip6_dhcp != 0) {
                if (info->lan2_ip6_dhcp == 1 && info->lan2_ip6_address[0] != '\0' && info->lan2_ip6_netmask[0] != '\0') {
                    ;
                } else {
                    if (info->lan2_ip6_dhcp == 1) //ra
                        iftype = 1;
                    else //dhcpv6
                        iftype = 0;

                    net_get_ipv6_ifaddr(iftype, "eth1", info->lan2_ip6_address, sizeof(info->lan2_ip6_address),
                                        info->lan2_ip6_netmask, sizeof(info->lan2_ip6_netmask));
                    //dhcpv6和路由公告网关均为路由器网关[fe80::2a10:fbff:feee:ba7a]
                    net_get_ipv6_gateway2(1, "eth1", info->lan2_ip6_gateway, sizeof(info->lan2_ip6_gateway));
                }
            }

        } else {
            //NETMODE_LOADBALANCE,
            //NETMODE_BACKUP

            if (info->bond0_ip6_dhcp != 0) {
                if (info->bond0_ip6_dhcp == 1 && info->bond0_ip6_address[0] != '\0' && info->bond0_ip6_netmask[0] != '\0') {
                    ;
                } else {

                    if (info->bond0_ip6_dhcp == 1) //ra
                        iftype = 1;
                    else //dhcpv6
                        iftype = 0;

                    net_get_ipv6_ifaddr(iftype, "bond0", info->bond0_ip6_address, sizeof(info->bond0_ip6_address),
                                        info->bond0_ip6_netmask, sizeof(info->bond0_ip6_netmask));
                    //dhcpv6和路由公告网关均为路由器网关[fe80::2a10:fbff:feee:ba7a]
                    net_get_ipv6_gateway2(1, "bond0", info->bond0_ip6_gateway, sizeof(info->bond0_ip6_gateway));
                }
            }
        }
    }
}

int NetworkPageBasic::dealConfirmLoadBalance(int mode)
{
    int ip6type = ui->comboBox_eth0IPv6Mode->currentIndex();
    int confirm;
    if (mode == NETMODE_LOADBALANCE) {

        if (ip6type == 0 && pNetworkDb->bond0_ip6_address[0] == '\0')
            confirm = 0;
        else
            confirm = 1;

        //need confirm
        if (confirm == 1) {
            QString strMessage = GET_TEXT("SYSTEMNETWORK/71070", "The switch should support Link Aggrengation Group feature when IPv6 is set to work in Load Banlance mode. Sure to Continue?");
            if (MessageBox::Cancel == MessageBox::question(this, strMessage)) {
                return -1;
            }
        }
    }

    return 0;
}

void NetworkPageBasic::onLanguageChanged()
{
    ui->label_workingMode->setText(GET_TEXT("WIZARD/11023", "Working Mode"));

    ui->labelDefaultRoute->setText(GET_TEXT("WIZARD/11086", "Default Route"));

    ui->label_bond0Eth0Type->setText(GET_TEXT("SYSTEMNETWORK/71157", "IPv4 DHCP"));
    ui->label_eth0IP->setText(GET_TEXT("SYSTEMNETWORK/71158", "IPv4 Address"));
    ui->label_eho0Netmask->setText(GET_TEXT("SYSTEMNETWORK/71159", "IPv4 Subnet Mask"));
    ui->label_eth0Gateway->setText(GET_TEXT("SYSTEMNETWORK/71160", "IPv4 Gateway"));
    ui->label_eth0PreDns->setText(GET_TEXT("WIZARD/11027", "Preferred DNS Server"));
    ui->label_eth0AlterDns->setText(GET_TEXT("WIZARD/11028", "Alternate DNS Server"));
    ui->label_mtu0->setText(GET_TEXT("NETWORKSTATUS/61007", "MTU(Byte)"));
    ui->label_eth0MAC->setText(GET_TEXT("CHANNELMANAGE/30027", "MAC"));
    ui->label_poeIPaddr->setText(GET_TEXT("WIZARD/11030", "PoE NIC IPv4 Address"));

    ui->groupBox_eth1Enable->setTitle(QString("%1 %2").arg(GET_TEXT("CAMERASEARCH/32023", "LAN2")).arg(GET_TEXT("COMMON/1009", "Enable")));
    ui->label_eth1Type->setText(GET_TEXT("SYSTEMNETWORK/71157", "IPv4 DHCP"));
    ui->label_eth1IP->setText(GET_TEXT("SYSTEMNETWORK/71158", "IPv4 Address"));
    ui->label_eth1Netmask->setText(GET_TEXT("SYSTEMNETWORK/71159", "IPv4 Subnet Mask"));
    ui->label_eth1Gateway->setText(GET_TEXT("SYSTEMNETWORK/71160", "IPv4 Gateway"));
    ui->label_eth1PreDns->setText(GET_TEXT("WIZARD/11027", "Preferred DNS Server"));
    ui->label_eth1AlterDns->setText(GET_TEXT("WIZARD/11028", "Alternate DNS Server"));
    ui->label_mtu1->setText(GET_TEXT("NETWORKSTATUS/61007", "MTU(Byte)"));
    ui->label_eth1MAC->setText(GET_TEXT("CHANNELMANAGE/30027", "MAC"));
    ui->label_primaryLan->setText(GET_TEXT("NETWORKSTATUS/61003", "Main NIC"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->label_eth0IPv6Mode->setText(GET_TEXT("SYSTEMNETWORK/71151", "IPv6 Mode"));
    ui->label_eth0IPv6Addr->setText(GET_TEXT("SYSTEMNETWORK/71152", "IPv6 Address"));
    ui->label_eth0IPvPreLen->setText(GET_TEXT("SYSTEMNETWORK/71153", "IPv6 Prefix Length"));
    ui->label_eth0IPv6GateWay->setText(GET_TEXT("SYSTEMNETWORK/71154", "IPv6 Gateway"));
    ui->label_eth0IPv6Rounter->setText(GET_TEXT("SYSTEMNETWORK/71155", "Router Advertisement"));
    ui->pushButton_eth0details->setText(GET_TEXT("SYSTEMNETWORK/71161", "Details"));

    ui->comboBox_eth0IPv6Mode->setItemText(0, GET_TEXT("SYSTEMNETWORK/71164", "Manual"));
    ui->comboBox_eth0IPv6Mode->setItemText(1, GET_TEXT("SYSTEMNETWORK/71155", "Router Advertisement"));
    ui->comboBox_eth0IPv6Mode->setItemText(2, GET_TEXT("SYSTEMNETWORK/71156", "DHCPv6"));

    ui->label_eth1IPv6Mode->setText(GET_TEXT("SYSTEMNETWORK/71151", "IPv6 Mode"));
    ui->label_eth1IPv6Addr->setText(GET_TEXT("SYSTEMNETWORK/71152", "IPv6 Address"));
    ui->label_eth1IPvPreLen->setText(GET_TEXT("SYSTEMNETWORK/71153", "IPv6 Prefix Length"));
    ui->label_eth1IPv6GateWay->setText(GET_TEXT("SYSTEMNETWORK/71154", "IPv6 Gateway"));
    ui->label_eth1IPv6Rounter->setText(GET_TEXT("SYSTEMNETWORK/71155", "Router Advertisement"));
    ui->pushButton_eth1details->setText(GET_TEXT("SYSTEMNETWORK/71161", "Details"));

    ui->comboBox_eth1IPv6Mode->setItemText(0, GET_TEXT("SYSTEMNETWORK/71164", "Manual"));
    ui->comboBox_eth1IPv6Mode->setItemText(1, GET_TEXT("SYSTEMNETWORK/71155", "Router Advertisement"));
    ui->comboBox_eth1IPv6Mode->setItemText(2, GET_TEXT("SYSTEMNETWORK/71156", "DHCPv6"));

    if (qMsNvr->deviceInfo().max_lan == 1 || qMsNvr->isPoe()) {
        ui->comboBox_workingMode->setItemText(0, GET_TEXT("SYSTEMNETWORK/71007", "Multi address"));
    } else {
        ui->comboBox_workingMode->setItemText(0, GET_TEXT("SYSTEMNETWORK/71007", "Multi address"));
        ui->comboBox_workingMode->setItemText(1, GET_TEXT("SYSTEMNETWORK/71008", "Load Balance"));
        ui->comboBox_workingMode->setItemText(2, GET_TEXT("SYSTEMNETWORK/71009", "Net Fault-tolerance"));
    }

    ui->comboBox_primaryLan->setItemText(0, GET_TEXT("CAMERASEARCH/32022", "LAN1"));
    ui->comboBox_primaryLan->setItemText(1, GET_TEXT("CAMERASEARCH/32023", "LAN2"));

    ui->comboBox_bond0Eth0Type->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_bond0Eth0Type->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));

    ui->comboBox_eth1Type->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_eth1Type->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));

    if (pRouterAdvDialog != NULL) {
        //pRouterAdvDialog->slotTranslateUi();
    }
}

void NetworkPageBasic::onComboBoxWorkingModeChanged(int index)
{
    ui->lineEdit_eth0IP->setValid(true);
    ui->lineEdit_eth0Netmask->setValid(true);
    ui->lineEdit_eth0Gateway->setValid(true);
    ui->lineEdit_eth0PreDns->setValid(true);
    ui->lineEdit_eth0AlterDns->setValid(true);
    ui->lineEdit_eth0IPv6Addr->setValid(true);
    ui->lineEdit_eth0IPv6GateWay->setValid(true);
    ui->lineEdit_eth0IPvPreLen->setValid(true);
    ui->lineEdit_poeIPaddr->setValid(true);
    ui->lineEdit_mtu0->setValid(true);

    const struct device_info &sys_info = qMsNvr->deviceInfo();
    if (index == NETMODE_MULTI) {
        ui->groupBox_eth0Enable->show();
        ui->groupBox_eth0Enable->setCheckable(true);

        ui->label_primaryLan->hide();
        ui->comboBox_primaryLan->hide();

        if (sys_info.max_lan == 1 || qMsNvr->isPoe()) {
            ui->groupBox_eth0Enable->setTitle(GET_TEXT("CAMERASEARCH/32031", "LAN"));
        } else {
            ui->groupBox_eth0Enable->setTitle(QString("%1 %2").arg(GET_TEXT("CAMERASEARCH/32022", "LAN1")).arg(GET_TEXT("COMMON/1009", "Enable")));
        }
    } else if (index == NETMODE_BACKUP) {
        ui->groupBox_eth0Enable->setCheckable(false);
        ui->groupBox_eth1Enable->hide();

        ui->label_primaryLan->show();
        ui->comboBox_primaryLan->show();
    } else {
        ui->groupBox_eth0Enable->setCheckable(false);
        ui->groupBox_eth1Enable->hide();
        ui->groupBox_eth0Enable->setTitle(GET_TEXT("NETWORKSTATUS/61014", "Bond0"));

        ui->label_primaryLan->hide();
        ui->comboBox_primaryLan->hide();
    }

    //
    if (index != NETMODE_MULTI || sys_info.max_lan == 1 || qMsNvr->isPoe()) {
        ui->labelDefaultRoute->hide();
        ui->comboBoxDefaultRoute->hide();
    } else {
        ui->labelDefaultRoute->show();
        ui->comboBoxDefaultRoute->show();
    }

    readBasicConfig(index);
}

void NetworkPageBasic::slotBond0Eth0Enable(bool state)
{
    Q_UNUSED(state);
}

void NetworkPageBasic::slotEth1Enable(bool state)
{
    Q_UNUSED(state);
}

void NetworkPageBasic::slotBond0Eth0Type(int state)
{
    if (state == 1) {
        //ui->bond0_eth0_typeFrame->setEnabled(false);
        ui->lineEdit_eth0IP->setEnabled(false);
        ui->lineEdit_eth0Netmask->setEnabled(false);
        ui->lineEdit_eth0Gateway->setEnabled(false);
        ui->lineEdit_eth0PreDns->setEnabled(false);
        ui->lineEdit_eth0AlterDns->setEnabled(false);
        ui->lineEdit_mtu0->setEnabled(false);
        ui->lineEdit_poeIPaddr->setEnabled(true);
    } else {
        //ui->bond0_eth0_typeFrame->setEnabled(true);
        ui->lineEdit_eth0IP->setEnabled(true);
        ui->lineEdit_eth0Netmask->setEnabled(true);
        ui->lineEdit_eth0Gateway->setEnabled(true);
        ui->lineEdit_eth0PreDns->setEnabled(true);
        ui->lineEdit_eth0AlterDns->setEnabled(true);
        ui->lineEdit_mtu0->setEnabled(true);
        ui->lineEdit_poeIPaddr->setEnabled(true);
    }
}

void NetworkPageBasic::slotEth1Type(int state)
{
    if (state == 1) {
        //ui->eth1_typeFrame->setEnabled(false);
        ui->lineEdit_eth1IP->setEnabled(false);
        ui->lineEdit_eth1Netmask->setEnabled(false);
        ui->lineEdit_eth1Gateway->setEnabled(false);
        ui->lineEdit_eth1PreDns->setEnabled(false);
        ui->lineEdit_eth1AlterDns->setEnabled(false);
        ui->lineEdit_mtu1->setEnabled(false);
    } else {
        //ui->eth1_typeFrame->setEnabled(true);
        ui->lineEdit_eth1IP->setEnabled(true);
        ui->lineEdit_eth1Netmask->setEnabled(true);
        ui->lineEdit_eth1Gateway->setEnabled(true);
        ui->lineEdit_eth1PreDns->setEnabled(true);
        ui->lineEdit_eth1AlterDns->setEnabled(true);
        ui->lineEdit_mtu1->setEnabled(true);
    }
}

void NetworkPageBasic::slotBondPrimaryComboBoxChanged(int index)
{
    if (index == 0) {
        ui->lineEdit_eth0MACShow->setText(QString(mac0));
    } else {
        ui->lineEdit_eth0MACShow->setText(QString(mac1));
    }
}

void NetworkPageBasic::slotEth0IPv6ModeChanged(int index)
{
    if (index == 1) {
        ui->pushButton_eth0details->show();
        ui->label_eth0IPv6Rounter->show();
    } else {
        ui->pushButton_eth0details->hide();
        ui->label_eth0IPv6Rounter->hide();
    }

    if (index == 0) {
        ui->lineEdit_eth0IPv6Addr->setEnabled(true);
        ui->lineEdit_eth0IPvPreLen->setEnabled(true);
        ui->lineEdit_eth0IPv6GateWay->setEnabled(true);
    } else {
        ui->lineEdit_eth0IPv6Addr->setEnabled(false);
        ui->lineEdit_eth0IPvPreLen->setEnabled(false);
        ui->lineEdit_eth0IPv6GateWay->setEnabled(false);
    }
}

void NetworkPageBasic::slotEth1IPv6ModeChanged(int index)
{
    if (index == 1) {
        ui->pushButton_eth1details->show();
        ui->label_eth1IPv6Rounter->show();
    } else {
        ui->pushButton_eth1details->hide();
        ui->label_eth1IPv6Rounter->hide();
    }

    if (index == 0) {
        ui->lineEdit_eth1IPv6Addr->setEnabled(true);
        ui->lineEdit_eth1IPvPreLen->setEnabled(true);
        ui->lineEdit_eth1IPv6GateWay->setEnabled(true);
    } else {
        ui->lineEdit_eth1IPv6Addr->setEnabled(false);
        ui->lineEdit_eth1IPvPreLen->setEnabled(false);
        ui->lineEdit_eth1IPv6GateWay->setEnabled(false);
    }
}

void NetworkPageBasic::slotShowEth0Details()
{
    if (!pRouterAdvDialog ) {
        pRouterAdvDialog = new RouterEdit(this);
    }
    if (pRouterAdvDialog) {
        struct network info;
        memset(&info, 0x0, sizeof(struct network));
        int mode = (NETMODE)ui->comboBox_workingMode->currentIndex();
        if (mode == NETMODE_MULTI)
            mode = 0; //eth0
        else
            mode = 2; //bond0

        getNetBasicInfo(&info);
        pRouterAdvDialog->setNetinfo(&info);
        pRouterAdvDialog->updateInfo(mode);
        pRouterAdvDialog->show();
        clearFocus();
        pRouterAdvDialog->setFocus();
    }
}

void NetworkPageBasic::slotShowEth1Details()
{
    if (!pRouterAdvDialog ) {
        pRouterAdvDialog = new RouterEdit(this);
    }

    if (pRouterAdvDialog) {
        int mode = 1;
        struct network info;
        memset(&info, 0x0, sizeof(struct network));
        getNetBasicInfo(&info);

        pRouterAdvDialog->setNetinfo(&info);
        pRouterAdvDialog->updateInfo(mode);
        pRouterAdvDialog->show();
        clearFocus();
        pRouterAdvDialog->setFocus();
    }
}

void NetworkPageBasic::onPushButtonApplyClicked()
{
    ui->pushButton_apply->clearUnderMouse();

    const struct device_info &sys_info = qMsNvr->deviceInfo();
    int mode = (NETMODE)ui->comboBox_workingMode->currentIndex();
    if (sys_info.max_lan > 1 && mode == NETMODE_MULTI) {
        if (ui->groupBox_eth0Enable->isCheckable() && !ui->groupBox_eth0Enable->isChecked() && ui->groupBox_eth1Enable->isCheckable() && !ui->groupBox_eth1Enable->isChecked()) {
            ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71073", "At last one lan should be enabled."));
            return;
        }
    }

    if (!isValidatorInput()) {
        return;
    }

    struct ip_conflict conflict;
    memset(&conflict, 0, sizeof(ip_conflict));
    saveBasicConfig();
    if (memcmp(pNetworkDb, pNetworkDbOri, sizeof(struct network)) != 0) {
        if (!pNetworkDbStatic) {
            return;
        }

        //只修改路由不用重启
        network temp;
        memset(&temp, 0, sizeof(temp));
        memcpy(&temp, pNetworkDbOri, sizeof(temp));
        temp.defaultRoute = pNetworkDb->defaultRoute;
        if (memcmp(&temp, pNetworkDb, sizeof(temp)) == 0) {
            qMsDebug() << "set default route:" << pNetworkDb->defaultRoute;
            write_network(SQLITE_FILE_NAME, pNetworkDb);
            memcpy(pNetworkDbOri, pNetworkDb, sizeof(struct network));
            sendMessage(REQUEST_FLAG_SET_DEFAULT_ROUTE, &pNetworkDb->defaultRoute, sizeof(pNetworkDb->defaultRoute));
            return;
        }

        //
        if (mode == NETMODE_MULTI) { //多址模式
            //ms_strcmp_two_ip_same_net is in same net?
            if (pNetworkDb->lan1_enable && pNetworkDb->lan2_enable) {
                int ret = 0;
                //qDebug() << "check same network segment" << ret;
                if (!ret) {
                    QString strMessage;
                    if (qMsNvr->isPoe()) {
                        strMessage = GET_TEXT("SYSTEMNETWORK/71115", "IP Conflict between the LAN1 and NVR Internal IPv4, please modify again.");
                        ShowMessageBox(strMessage);
                        return;
                    } else {
                        if (pNetworkDb->lan1_type == 1 && pNetworkDb->lan2_type == 1) {
                            //DHCP不检测冲突
                        } else {
                            strMessage = GET_TEXT("SYSTEMNETWORK/71075", "The two IP Address belong to the same network segment, please modify again.");
                            ShowMessageBox(strMessage);
                            return;
                        }
                    }
                }
            }
            if ((pNetworkDb->lan1_enable && !pNetworkDb->lan1_type) && memcmp(pNetworkDb->lan1_ip_address, pNetworkDbStatic->lan1_ip_address, sizeof(pNetworkDb->lan1_ip_address)) != 0) {
                snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_ETH0);
                snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->lan1_ip_address);
                sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                _ip_conflict_type = 1;
                //showWait();
                return;
            } else if ((pNetworkDb->lan2_enable && !pNetworkDb->lan2_type) && memcmp(pNetworkDb->lan2_ip_address, pNetworkDbStatic->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address)) != 0 && !qMsNvr->isPoe()) {
                snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_ETH1);
                snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->lan2_ip_address);
                sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                _ip_conflict_type = 2;
                //showWait();
                return;
            } else {
                writeBasicConfig(mode);
                return;
            }
        } else { //容错模式  或者负载平衡
            if (memcmp(pNetworkDb->bond0_ip_address, pNetworkDbStatic->bond0_ip_address, sizeof(pNetworkDb->bond0_ip_address)) != 0) {
                //guiSendMsg(SOCKET_TYPE_CORE, REQUEST_FLAG_TEST_IP_CONFLICT, sizeof(pNetworkDb->bond0_ip_address), (void *)&pNetworkDb->bond0_ip_address);
                snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_BOND);
                snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->bond0_ip_address);
                sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                _ip_conflict_type = 0;
                //showWait();
                return;
            } else {
                writeBasicConfig(mode);
                return;
            }
        }
    }
}

void NetworkPageBasic::onPushButtonBackClicked()
{
    freeNetBasicTab();
    emit sig_back();
}
