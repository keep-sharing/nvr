#include "wizardpagenetwork.h"
#include "ui_wizardpagenetwork.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "mswizard.h"
#include "myqt.h"

WizardPageNetwork::WizardPageNetwork(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPageNetwork)
{
    ui->setupUi(this);

    initNetWizardPage();
    ui->ipaddr_lineEdit->setCheckMode(MyLineEdit::WizardNetCheck);
    ui->netmask_lineEdit->setCheckMode(MyLineEdit::EmptyCheck);
    ui->gateway_lineEdit->setCheckMode(MyLineEdit::WizardNetCheck);
    ui->dns1_lineEdit->setCheckMode(MyLineEdit::WizardNetCheck);
    ui->dns2_lineEdit->setCheckMode(MyLineEdit::WizardNetCanEmptyCheck);
    ui->poeIPaddr_lineEdit->setCheckMode(MyLineEdit::WizardNetCanEmptyCheck);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

WizardPageNetwork::~WizardPageNetwork()
{
    delete ui;
}

void WizardPageNetwork::initializeData()
{
    initializePage();
}

void WizardPageNetwork::saveSetting()
{
}

void WizardPageNetwork::previousPage()
{
    showWizardPage(Wizard_Time);
}

void WizardPageNetwork::nextPage()
{
    bool result = validatePage();
    if (result) {
        showWizardPage(Wizard_Disk);
    }
}

void WizardPageNetwork::skipWizard()
{
    AbstractWizardPage::skipWizard();
}

void WizardPageNetwork::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_TEST_IP_CONFLICT:
        ON_RESPONSE_FLAG_TEST_IP_CONFLICT(message);
        break;
    case RESPONSE_FLAG_IP_CONFLICT_BY_DEV:
        ON_RESPONSE_FLAG_IP_CONFLICT_BY_DEV(message);
        break;
    }
}

int WizardPageNetwork::ON_RESPONSE_FLAG_TEST_IP_CONFLICT(MessageReceive *message)
{
    //m_waitting->//closeWait();

    int ret = 0;
    if (!message->data) {
        qWarning() << "WizardPageNetwork::ON_RESPONSE_FLAG_TEST_IP_CONFLICT, data is null.";
    } else {
        ret = *((int *)message->data);
    }
    int respon = 0;
    struct ip_conflict conflict;
    memset(&conflict, 0, sizeof(struct ip_conflict));
    if (ipConflictCheckType == 0) //bond0 ip conflict check
    {
        if (ret == -1) {
            //冲突  ，进行相关操作
            QString strMessage = GET_TEXT("SYSTEMNETWORK/71060", "Detect Bond0 IP Conflict, Continue?");
            if (MessageBox::Yes == MessageBox::question(this, strMessage)) //不管冲突，任性修改，滴滴  重启吧
            {
                //modify by tyler for reboot not goto diskpage
                respon = nextOrReboot(pNetworkDbOri->bond0_ip_address, 1);
                return respon;
            } else {
                //冲突，修改吧
                return 0;
            }
        } else {
            //不冲突  跳转页面
            //modify by tyler for reboot not goto diskpage
            respon = nextOrReboot(pNetworkDbOri->bond0_ip_address, 1);
            return respon;
        }
    } else if (ipConflictCheckType == 1) //lan1 ip conflict check
    {
        if (ret == -1) {
            QString strMessage = GET_TEXT("SYSTEMNETWORK/71061", "Detect LAN1 IP Conflict, Continue?");
            if (MessageBox::Yes == MessageBox::question(this, strMessage)) //冲突，任性
            {
                if (pNetworkDb->lan2_enable && !qMsNvr->isPoe()) {
                    if (memcmp(pNetworkDb->lan2_ip_address, pNetworkDbOri->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address)) != 0) {
                        snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_ETH1);
                        snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->lan2_ip_address);
                        sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                        ipConflictCheckType = 2;
                        //m_waitting->//showWait();
                        return 0;
                    } else {
                        //modify by tyler for reboot not goto diskpage
                        respon = nextOrReboot(pNetworkDbOri->lan1_ip_address, 1);
                        return respon;
                    }
                } else {
                    //modify by tyler for reboot not goto diskpage
                    respon = nextOrReboot(pNetworkDbOri->lan1_ip_address, 1);
                    return respon;
                }
            } else {
                //想修改
                return 0;
            }
        } else {
            if (pNetworkDb->lan2_enable && !qMsNvr->isPoe()) {
                if (memcmp(pNetworkDb->lan2_ip_address, pNetworkDbOri->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address)) != 0) {
                    //guiSendMsg(SOCKET_TYPE_CORE, REQUEST_FLAG_TEST_IP_CONFLICT, sizeof(pNetworkDb->lan2_ip_address), (void *)&pNetworkDb->lan2_ip_address);
                    snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_ETH1);
                    snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->lan2_ip_address);
                    sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                    ipConflictCheckType = 2;
                    //m_waitting->//showWait();
                    return 0;
                } else {
                    //modify by tyler for reboot not goto diskpage
                    respon = nextOrReboot(pNetworkDbOri->lan1_ip_address, 1);
                    return respon;
                }
            } else {
                //modify by tyler for reboot not goto diskpage
                respon = nextOrReboot(pNetworkDbOri->lan1_ip_address, 1);
                return respon;
            }
        }
    } else if (ipConflictCheckType == 2) //lan2 ip conflict check
    {
        if (ret == -1) {
            //冲突，进行相关操作
            QString strMessage;
            if (qMsNvr->isPoe())
                strMessage = GET_TEXT("SYSTEMNETWORK/71114", "Detect PoE NIC IPv4 Address Conflict, Continue?");
            else
                strMessage = GET_TEXT("SYSTEMNETWORK/71062", "Detect LAN2 IP Conflict, Continue?");
            if (MessageBox::Yes == MessageBox::question(this, strMessage)) //不管冲突，任性修改，滴滴  重启吧
            {
                //modify by tyler for reboot not goto diskpage
                respon = nextOrReboot(pNetworkDbOri->lan2_ip_address, 2);
                return respon;
            } else {
                //冲突，修改吧
                return 0;
            }
        } else {
            //不冲突  跳转页面
            //modify by tyler for reboot not goto diskpage
            respon = nextOrReboot(pNetworkDbOri->lan2_ip_address, 2);
            return respon;
        }
    }
    return 0;
}

void WizardPageNetwork::ON_RESPONSE_FLAG_IP_CONFLICT_BY_DEV(MessageReceive *message)
{
    if (ON_RESPONSE_FLAG_TEST_IP_CONFLICT(message) == 1) {
        showWizardPage(Wizard_Disk);
    }
}

void WizardPageNetwork::initNetWizardPage()
{
    const device_info &sys_info = qMsNvr->deviceInfo();

    ui->comboBox_workMode->clear();
    if (sys_info.max_lan == 1 || qMsNvr->isPoe()) {
        ui->comboBox_workMode->addItem(GET_TEXT("SYSTEMNETWORK/71007", "Multi address"), 0);
        ui->label_workMode->hide();
        ui->comboBox_workMode->hide();
        ui->enableLAN_checkBox->hide();
    } else {
        ui->comboBox_workMode->addItem(GET_TEXT("SYSTEMNETWORK/71007", "Multi address"), NETMODE_MULTI);
        ui->comboBox_workMode->addItem(GET_TEXT("SYSTEMNETWORK/71008", "Load Balance"), NETMODE_LOADBALANCE);
        ui->comboBox_workMode->addItem(GET_TEXT("SYSTEMNETWORK/71009", "Net Fault-tolerance"), NETMODE_BACKUP);
    }

    ui->comboBoxDefaultRoute->clear();
    ui->comboBoxDefaultRoute->addItem("LAN1", 0);
    ui->comboBoxDefaultRoute->addItem("LAN2", 1);

    connect(ui->enableLAN_checkBox, SIGNAL(toggled(bool)), this, SLOT(enableLAN_checkBoxSlot(bool)));
    connect(ui->enableDHCP_checkBox, SIGNAL(toggled(bool)), this, SLOT(enableDHCP_checkBoxSlot(bool)));
    connect(ui->comboBox_workMode, SIGNAL(currentIndexChanged(int)), this, SLOT(workModeComboBoxChangedSlot(int)));
    connect(ui->selectNIC_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectNIC_comboBoxChangedSlot(int)));

    pNetworkDb = NULL;
    pNetworkDbOri = NULL;
    currentNICid = 0;
    ipConflictCheckType = -1; //0: bond0 1:lan1 2:lan2

    //poe nvr
    if (qMsNvr->isPoe()) {
        ui->label_poeIPaddr->show();
        ui->poeIPaddr_lineEdit->show();

        ui->comboBox_workMode->addItem(GET_TEXT("SYSTEMNETWORK/71007", "Multi address"), 0);
        ui->label_workMode->hide();
        ui->comboBox_workMode->hide();
        ui->enableLAN_checkBox->hide();
    } else {
        ui->label_poeIPaddr->hide();
        ui->poeIPaddr_lineEdit->hide();
    }
}

void WizardPageNetwork::initializePage()
{
    if (pNetworkDb == NULL) {
        pNetworkDb = new struct network;
    }
    if (pNetworkDbOri == NULL) {
        pNetworkDbOri = new struct network;
    }
    memset(pNetworkDb, 0, sizeof(struct network));
    memset(pNetworkDbOri, 0, sizeof(struct network));

    read_network(SQLITE_FILE_NAME, pNetworkDb);
    memcpy(pNetworkDbOri, pNetworkDb, sizeof(struct network));

    ui->comboBoxDefaultRoute->setCurrentIndexFromData(pNetworkDb->defaultRoute);

    if (ui->comboBox_workMode->currentIndex() != pNetworkDb->mode) {
        ui->comboBox_workMode->setCurrentIndex(pNetworkDb->mode);
    } else {
        workModeComboBoxChangedSlot(pNetworkDb->mode);
    }
}

bool WizardPageNetwork::validatePage()
{
    if (!isValidatorInput()) {
        return false;
    }
    int respon = 0;
    struct ip_conflict conflict;
    memset(&conflict, 0, sizeof(struct ip_conflict));
    //
    if (ui->comboBoxDefaultRoute->isVisible()) {
        pNetworkDb->defaultRoute = ui->comboBoxDefaultRoute->currentData().toInt();
    }
    //
    if (ui->comboBox_workMode->currentIndex() == NETMODE_MULTI) {
        pNetworkDb->mode = NETMODE_MULTI;
        saveCurrentNICDlg(currentNICid);
        if (!(pNetworkDb->lan1_enable || pNetworkDb->lan2_enable)) {
            ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71073", "At last one lan should be enabled."));
            return false;
        }

        if (memcmp(pNetworkDb, pNetworkDbOri, sizeof(struct network)) != 0) {
            if (pNetworkDb->lan1_enable && pNetworkDb->lan2_enable && !ui->enableDHCP_checkBox->isChecked()) {
                int ret = 0;
                if (!ret) {
                    if (qMsNvr->isPoe()) {
                        ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71115", "IP Conflict between the LAN1 and NVR Internal IPv4, please modify again."));
                    } else {
                        ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71074", "LAN1 and LAN2 belong to the same network segment. Are you sure to continue?"));
                    }
                    return false;
                }
            }
            if (memcmp(pNetworkDb->lan1_ip_address, pNetworkDbOri->lan1_ip_address, sizeof(pNetworkDb->lan1_ip_address)) != 0) {
                snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_ETH0);
                snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->lan1_ip_address);
                sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                ipConflictCheckType = 1;
                //m_waitting->//showWait();
                return false;
            } else if (memcmp(pNetworkDb->lan2_ip_address, pNetworkDbOri->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address)) != 0) {
                if (qMsNvr->isPoe()) {
                    respon = nextOrReboot(pNetworkDbOri->lan2_ip_address, 2);
                    return respon;
                }
                snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_ETH1);
                snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->lan2_ip_address);
                sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                ipConflictCheckType = 2;
                //m_waitting->//showWait();
                return false;
            } else {
                respon = nextOrReboot(pNetworkDbOri->lan1_ip_address, 1);
                return respon;
            }
        }
    } else if (ui->comboBox_workMode->currentIndex() == NETMODE_BACKUP || ui->comboBox_workMode->currentIndex() == NETMODE_LOADBALANCE) {
        pNetworkDb->mode = ui->comboBox_workMode->currentIndex();
        saveBond0Dlg();
        if (memcmp(pNetworkDb, pNetworkDbOri, sizeof(struct network)) != 0) {
            if (!ui->enableDHCP_checkBox->isChecked() && memcmp(pNetworkDb->bond0_ip_address, pNetworkDbOri->bond0_ip_address, sizeof(pNetworkDb->bond0_ip_address)) != 0) {
                snprintf(conflict.dev, sizeof(conflict.dev), "%s", DEVICE_NAME_BOND);
                snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", pNetworkDb->bond0_ip_address);
                sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(struct ip_conflict));
                ipConflictCheckType = 0;
                //m_waitting->//showWait();
                return false;
            } else {
                respon = nextOrReboot(pNetworkDbOri->bond0_ip_address, 1);
                return respon;
            }
        }
    }
    return true;
}

void WizardPageNetwork::readFromProfileToDlg(int mode)
{
    const device_info &sys_info = qMsNvr->deviceInfo();

    char ifaddr[16] = { 0 };
    char ifmask[16] = { 0 };
    char ifgw[16] = { 0 };
    if (mode == NETMODE_MULTI) {
        memset(pNetworkDbOri, 0, sizeof(struct network));
        read_network(SQLITE_FILE_NAME, pNetworkDbOri); //重新读取数据库，清除之前multi保存的数据

        currentNICid = 0;
        ui->selectNIC_comboBox->clear();
        ui->selectNIC_comboBox->setEnabled(true);
        if (sys_info.max_lan == 1 || qMsNvr->isPoe()) {
            ui->selectNIC_comboBox->addItem(trUtf8("LAN"), trUtf8("LAN"));
        } else {
            ui->selectNIC_comboBox->addItem(trUtf8("LAN1"), trUtf8("LAN1"));
            if (!qMsNvr->isPoe()) {
                ui->enableLAN_checkBox->show();
                ui->selectNIC_comboBox->addItem(trUtf8("LAN2"), trUtf8("LAN2"));
            }
        }
    } else if (mode == NETMODE_BACKUP || mode == NETMODE_LOADBALANCE) {
        memset(pNetworkDb, 0, sizeof(struct network));
        read_network(SQLITE_FILE_NAME, pNetworkDb); //重新读取数据库，清除之前multi保存的数据

        memset(pNetworkDbOri, 0, sizeof(struct network));
        read_network(SQLITE_FILE_NAME, pNetworkDbOri); //重新读取数据库，清除之前multi保存的数据

        ui->enableLAN_checkBox->setChecked(true);
        ui->enableLAN_checkBox->hide();
        enableLAN_checkBoxSlot(true);

        ui->selectNIC_comboBox->clear();
        ui->selectNIC_comboBox->addItem(trUtf8("Bond0"), trUtf8("Bond0"));
        ui->selectNIC_comboBox->setEnabled(false);

        if (pNetworkDb->bond0_enable) {
            ui->enableLAN_checkBox->setChecked(true);
            enableLAN_checkBoxSlot(true);
        } else {
            ui->enableLAN_checkBox->setChecked(false);
            enableLAN_checkBoxSlot(false);
        }

        if (pNetworkDb->bond0_type) {
            ui->enableDHCP_checkBox->setChecked(true);
            enableDHCP_checkBoxSlot(true);
        } else {
            ui->enableDHCP_checkBox->setChecked(false);
            enableDHCP_checkBoxSlot(false);
        }

        ui->ipaddr_lineEdit->setText(QString(pNetworkDb->bond0_ip_address));
        ui->netmask_lineEdit->setText(QString(pNetworkDb->bond0_netmask));
        ui->gateway_lineEdit->setText(QString(pNetworkDb->bond0_gateway));
        ui->dns1_lineEdit->setText(QString(pNetworkDb->bond0_primary_dns));
        ui->dns2_lineEdit->setText(QString(pNetworkDb->bond0_second_dns));
        ui->bondPrimary_comboBox->setCurrentIndex(pNetworkDb->bond0_primary_net); //0-eth0 ,1-eth1

        if (pNetworkDb->bond0_type == 1 && pNetworkDb->bond0_enable == 1) {
            int result1 = net_get_ifaddr("bond0", ifaddr, sizeof(ifaddr));
            int result2 = net_get_netmask("bond0", ifmask, sizeof(ifmask));
            int result3 = net_get_gateway("bond0", ifgw, sizeof(ifgw));
            if (result1 == 0) {
                ui->ipaddr_lineEdit->setText(QString(ifaddr));
                snprintf(pNetworkDbOri->bond0_ip_address, sizeof(pNetworkDbOri->bond0_ip_address), "%s", ui->ipaddr_lineEdit->text().trimmed().toStdString().c_str());
            }
            if (result2 == 0) {
                ui->netmask_lineEdit->setText(QString(ifmask));
                snprintf(pNetworkDbOri->bond0_netmask, sizeof(pNetworkDbOri->lan1_ip_address), "%s", ui->netmask_lineEdit->text().trimmed().toStdString().c_str());
            }
            if (result3 == 0) {
                ui->gateway_lineEdit->setText(QString(ifgw));
                snprintf(pNetworkDbOri->bond0_gateway, sizeof(pNetworkDbOri->bond0_gateway), "%s", ui->gateway_lineEdit->text().trimmed().toStdString().c_str());
            }
        }
    }
}

void WizardPageNetwork::saveCurrentNICDlg(int id)
{
    const device_info &sys_info = qMsNvr->deviceInfo();
    if (id == 0) {
        if (ui->enableDHCP_checkBox->isChecked()) {
            pNetworkDb->lan1_type = 1;
        } else {
            pNetworkDb->lan1_type = 0;
        }

        if (sys_info.max_lan == 1 || qMsNvr->isPoe()) {
            pNetworkDb->lan1_enable = 1;
        } else {
            if (ui->enableLAN_checkBox->isChecked()) {
                pNetworkDb->lan1_enable = 1;
            } else {
                pNetworkDb->lan1_enable = 0;
            }
        }

        snprintf(pNetworkDb->lan1_ip_address, sizeof(pNetworkDb->lan1_ip_address), "%s", ui->ipaddr_lineEdit->text().trimmed().toStdString().c_str());
        snprintf(pNetworkDb->lan1_netmask, sizeof(pNetworkDb->lan1_netmask), "%s", ui->netmask_lineEdit->text().trimmed().toStdString().c_str());
        snprintf(pNetworkDb->lan1_gateway, sizeof(pNetworkDb->lan1_gateway), "%s", ui->gateway_lineEdit->text().trimmed().toStdString().c_str());
        snprintf(pNetworkDb->lan1_primary_dns, sizeof(pNetworkDb->lan1_primary_dns), "%s", ui->dns1_lineEdit->text().trimmed().toStdString().c_str());
        snprintf(pNetworkDb->lan1_second_dns, sizeof(pNetworkDb->lan1_second_dns), "%s", ui->dns2_lineEdit->text().trimmed().toStdString().c_str());

        if (qMsNvr->isPoe()) {
            snprintf(pNetworkDb->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address), "%s", ui->poeIPaddr_lineEdit->text().trimmed().toStdString().c_str());
        }
    } else if (id == 1) {
        if (ui->enableDHCP_checkBox->isChecked()) {
            pNetworkDb->lan2_type = 1;
        } else {
            pNetworkDb->lan2_type = 0;
        }

        if (ui->enableLAN_checkBox->isChecked()) {
            pNetworkDb->lan2_enable = 1;
        } else {
            pNetworkDb->lan2_enable = 0;
        }

        snprintf(pNetworkDb->lan2_ip_address, sizeof(pNetworkDb->lan2_ip_address), "%s", ui->ipaddr_lineEdit->text().trimmed().toStdString().c_str());
        snprintf(pNetworkDb->lan2_netmask, sizeof(pNetworkDb->lan2_netmask), "%s", ui->netmask_lineEdit->text().trimmed().toStdString().c_str());
        snprintf(pNetworkDb->lan2_gateway, sizeof(pNetworkDb->lan2_gateway), "%s", ui->gateway_lineEdit->text().trimmed().toStdString().c_str());
        snprintf(pNetworkDb->lan2_primary_dns, sizeof(pNetworkDb->lan2_primary_dns), "%s", ui->dns1_lineEdit->text().trimmed().toStdString().c_str());
        snprintf(pNetworkDb->lan2_second_dns, sizeof(pNetworkDb->lan2_second_dns), "%s", ui->dns2_lineEdit->text().trimmed().toStdString().c_str());
    }
}

void WizardPageNetwork::saveBond0Dlg()
{
    if ((ui->enableLAN_checkBox->isVisible() && ui->enableLAN_checkBox->isChecked()) || !ui->enableLAN_checkBox->isVisible()) {
        pNetworkDb->bond0_enable = 1;
    } else {
        pNetworkDb->bond0_enable = 0;
    }

    if (ui->enableDHCP_checkBox->isChecked()) {
        pNetworkDb->bond0_type = 1;
    } else {
        pNetworkDb->bond0_type = 0;
    }

    snprintf(pNetworkDb->bond0_ip_address, sizeof(pNetworkDb->bond0_ip_address), "%s", ui->ipaddr_lineEdit->text().trimmed().toStdString().c_str());
    snprintf(pNetworkDb->bond0_netmask, sizeof(pNetworkDb->bond0_netmask), "%s", ui->netmask_lineEdit->text().trimmed().toStdString().c_str());
    snprintf(pNetworkDb->bond0_gateway, sizeof(pNetworkDb->bond0_gateway), "%s", ui->gateway_lineEdit->text().trimmed().toStdString().c_str());
    snprintf(pNetworkDb->bond0_primary_dns, sizeof(pNetworkDb->bond0_primary_dns), "%s", ui->dns1_lineEdit->text().trimmed().toStdString().c_str());
    snprintf(pNetworkDb->bond0_second_dns, sizeof(pNetworkDb->bond0_second_dns), "%s", ui->dns2_lineEdit->text().trimmed().toStdString().c_str());

    pNetworkDb->bond0_primary_net = ui->bondPrimary_comboBox->currentIndex();
}

bool WizardPageNetwork::isValidatorInput()
{
    bool valid = true;
    QString strIP = ui->ipaddr_lineEdit->text().trimmed();
    if (ui->ipaddr_lineEdit->isEnabled() && !ui->ipaddr_lineEdit->checkValid()) {
        valid = false;
    }
    if (valid) {
        QStringList ipList = strIP.split(".");
        if (ipList.size() != 4) {
            qMsWarning() << "invalid ip:" << strIP;
        } else {
            int ip1 = ipList.first().toInt();
            if (ip1 < 1 || ip1 > 223) {
                ui->ipaddr_lineEdit->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
                valid = false;
            }
        }
    }

    QString strMask = ui->netmask_lineEdit->text().trimmed();
    valid = ui->netmask_lineEdit->checkValid() && valid;

    if (!strMask.isEmpty() && ui->netmask_lineEdit->isEnabled() && !MyQt::isSubnetMask(strMask)) {
        ui->netmask_lineEdit->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
        valid = false;
    }
    if (ui->gateway_lineEdit->isEnabled()  && !ui->gateway_lineEdit->checkValid()) {
        valid = false;
    }
    if (ui->dns1_lineEdit->isEnabled() && !ui->dns1_lineEdit->checkValid()) {
        valid = false;
    }
    if (ui->dns2_lineEdit->isEnabled() &&  !ui->dns2_lineEdit->checkValid()) {
        valid = false;
    }
    //david.milesight
    QString strPOE = ui->poeIPaddr_lineEdit->text();
    if (qMsNvr->isPoe()  && !ui->poeIPaddr_lineEdit->checkValid()) {
        valid = false;
    }
    return valid;
}

void WizardPageNetwork::storeAndApplay()
{
    if (memcmp(pNetworkDb, pNetworkDbOri, sizeof(struct network)) != 0) {
        QString strMessage = GET_TEXT("SYSTEMNETWORK/71109", "The change will be effective after reboot. Do you want to reboot now?");
        if (MessageBox::Cancel == MessageBox::question(this, strMessage)) {
            return;
        }
        memcpy(pNetworkDbOri, pNetworkDb, sizeof(struct network));
        write_network(SQLITE_FILE_NAME, pNetworkDb);

        struct req_set_sysconf timeconf;
        memset(&timeconf, 0, sizeof(timeconf));
        snprintf(timeconf.arg, sizeof(timeconf.arg), "%s", "network");
        sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&timeconf, sizeof(timeconf));

        sendMessageOnly(REQUEST_FLAG_SET_NETWORK_RESTART, (void *)&timeconf, sizeof(timeconf));
        //m_waitting->//showWait();
    }
}

int WizardPageNetwork::nextOrReboot(char *str, int device)
{
    if (memcmp(pNetworkDb, pNetworkDbOri, sizeof(struct network)) != 0) {
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
            return 1;
        }

        //
        QString strMessage = GET_TEXT("SYSTEMNETWORK/71109", "The change will be effective after reboot. Do you want to reboot now?");
        if (MessageBox::Cancel == MessageBox::question(this, strMessage)) {
            if (device == 1)
                ui->ipaddr_lineEdit->setText(QString(str));
            else
                ui->poeIPaddr_lineEdit->setText(QString(str));
            return 1;
        }

        memcpy(pNetworkDbOri, pNetworkDb, sizeof(struct network));
        write_network(SQLITE_FILE_NAME, pNetworkDb);

        struct req_set_sysconf timeconf;
        memset(&timeconf, 0, sizeof(timeconf));
        snprintf(timeconf.arg, sizeof(timeconf.arg), "%s", "network");
        sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&timeconf, sizeof(timeconf));

        sendMessageOnly(REQUEST_FLAG_SET_NETWORK_RESTART, (void *)&timeconf, sizeof(timeconf));
        //m_waitting->//showWait();
    }
    return 0;
}

void WizardPageNetwork::onLanguageChanged()
{
    ui->label_workMode->setText(GET_TEXT("WIZARD/11023", "Work Mode"));
    ui->labelDefaultRoute->setText(GET_TEXT("WIZARD/11086", "Default Route"));
    ui->label_selectNIC->setText(GET_TEXT("WIZARD/11024", "NIC"));
    ui->enableLAN_checkBox->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->enableDHCP_checkBox->setText(GET_TEXT("WIZARD/11025", "Enable DHCP"));
    ui->label_ipaddr->setText(GET_TEXT("COMMON/1033", "IP Address"));
    ui->label_netmask->setText(GET_TEXT("WIZARD/11026", "Subnet Mask"));
    ui->label_gateway->setText(GET_TEXT("COMMON/1034", "Gateway"));
    ui->label_dnsPre->setText(GET_TEXT("WIZARD/11027", "Preferred DNS Server"));
    ui->label_dnsAlt->setText(GET_TEXT("WIZARD/11028", "Alternate DNS Server"));
    ui->label_bondPrimary->setText(GET_TEXT("NETWORKSTATUS/61003", "Main NIC"));
    ui->label_poeIPaddr->setText(GET_TEXT("WIZARD/11030", "PoE NIC IPv4 Address"));

    const device_info &sys_info = qMsNvr->deviceInfo();
    if (sys_info.max_lan == 1 || qMsNvr->isPoe()) {
        ui->comboBox_workMode->setItemText(0, GET_TEXT("SYSTEMNETWORK/71007", "Multi address"));
    } else {
        ui->comboBox_workMode->setItemText(0, GET_TEXT("SYSTEMNETWORK/71007", "Multi address"));
        ui->comboBox_workMode->setItemText(1, GET_TEXT("SYSTEMNETWORK/71008", "Load Balance"));
        ui->comboBox_workMode->setItemText(2, GET_TEXT("SYSTEMNETWORK/71009", "Net Fault-tolerance"));
    }
    if (qMsNvr->isPoe()) {
        ui->comboBox_workMode->setItemText(0, GET_TEXT("SYSTEMNETWORK/71007", "Multi address"));
    }
}

void WizardPageNetwork::workModeComboBoxChangedSlot(int index)
{
    const struct device_info &sys_info = qMsNvr->deviceInfo();

    if (index == NETMODE_MULTI) {
        ui->label_bondPrimary->hide();
        ui->bondPrimary_comboBox->hide();
    } else if (index == NETMODE_BACKUP) {
        ui->label_bondPrimary->show();
        ui->bondPrimary_comboBox->show();
    } else {
        ui->label_bondPrimary->hide();
        ui->bondPrimary_comboBox->hide();
    }

    //
    if (index != NETMODE_MULTI || sys_info.max_lan == 1 || qMsNvr->isPoe()) {
        ui->labelDefaultRoute->hide();
        ui->comboBoxDefaultRoute->hide();
    } else {
        ui->labelDefaultRoute->show();
        ui->comboBoxDefaultRoute->show();
    }

    //
    readFromProfileToDlg(index);
}

void WizardPageNetwork::selectNIC_comboBoxChangedSlot(int index)
{
    if (index < 0) {
        return;
    }
    char ifaddr[16] = { 0 };
    char ifmask[16] = { 0 };
    char ifgw[16] = { 0 };

    if (ui->comboBox_workMode->currentIndex() == NETMODE_MULTI) {
        if (currentNICid != index) {
            saveCurrentNICDlg(currentNICid);
            currentNICid = index;
        }
        if (index == 0) {
            if (pNetworkDb->lan1_enable) {
                ui->enableLAN_checkBox->setChecked(true);
                enableLAN_checkBoxSlot(true);
                if (pNetworkDb->lan1_type) {
                    ui->enableDHCP_checkBox->setChecked(true);
                    enableDHCP_checkBoxSlot(true);
                } else {
                    ui->enableDHCP_checkBox->setChecked(false);
                    enableDHCP_checkBoxSlot(false);
                }
            } else {
                ui->enableLAN_checkBox->setChecked(false);
                enableLAN_checkBoxSlot(false);
            }
            if (pNetworkDb->lan1_type == 1) {
                //dhcp
                net_get_ifaddr("eth0", ifaddr, sizeof(ifaddr));
                net_get_netmask("eth0", ifmask, sizeof(ifmask));
                net_get_gateway("eth0", ifgw, sizeof(ifgw));
                ui->ipaddr_lineEdit->setText(QString(ifaddr));
                ui->netmask_lineEdit->setText(QString(ifmask));
                ui->gateway_lineEdit->setText(QString(ifgw));

                snprintf(pNetworkDbOri->lan1_ip_address, sizeof(pNetworkDbOri->lan1_ip_address), "%s", ui->ipaddr_lineEdit->text().trimmed().toStdString().c_str());
                snprintf(pNetworkDbOri->lan1_netmask, sizeof(pNetworkDbOri->lan1_netmask), "%s", ui->netmask_lineEdit->text().trimmed().toStdString().c_str());
                snprintf(pNetworkDbOri->lan1_gateway, sizeof(pNetworkDbOri->lan1_gateway), "%s", ui->gateway_lineEdit->text().trimmed().toStdString().c_str());
            } else {
                ui->ipaddr_lineEdit->setText(QString(pNetworkDb->lan1_ip_address));
                ui->netmask_lineEdit->setText(QString(pNetworkDb->lan1_netmask));
                ui->gateway_lineEdit->setText(QString(pNetworkDb->lan1_gateway));
            }
            ui->dns1_lineEdit->setText(QString(pNetworkDb->lan1_primary_dns));
            ui->dns2_lineEdit->setText(QString(pNetworkDb->lan1_second_dns));
            if (qMsNvr->isPoe()) {
                ui->poeIPaddr_lineEdit->setText(QString(pNetworkDb->lan2_ip_address));
            }
        } else if (index == 1) {
            if (pNetworkDb->lan2_enable) {
                ui->enableLAN_checkBox->setChecked(true);
                enableLAN_checkBoxSlot(true);
                if (pNetworkDb->lan2_type) {
                    ui->enableDHCP_checkBox->setChecked(true);
                    enableDHCP_checkBoxSlot(true);
                } else {
                    ui->enableDHCP_checkBox->setChecked(false);
                    enableDHCP_checkBoxSlot(false);
                }
            } else {
                ui->enableLAN_checkBox->setChecked(false);
                enableLAN_checkBoxSlot(false);
            }
            if (pNetworkDb->lan2_type == 1) {
                //dhcp
                net_get_ifaddr("eth1", ifaddr, sizeof(ifaddr));
                net_get_netmask("eth1", ifmask, sizeof(ifmask));
                net_get_gateway("eth1", ifgw, sizeof(ifgw));
                ui->ipaddr_lineEdit->setText(QString(ifaddr));
                ui->netmask_lineEdit->setText(QString(ifmask));
                ui->gateway_lineEdit->setText(QString(ifgw));

                snprintf(pNetworkDbOri->lan2_ip_address, sizeof(pNetworkDbOri->lan2_ip_address), "%s", ui->ipaddr_lineEdit->text().trimmed().toStdString().c_str());
                snprintf(pNetworkDbOri->lan2_netmask, sizeof(pNetworkDbOri->lan2_netmask), "%s", ui->netmask_lineEdit->text().trimmed().toStdString().c_str());
                snprintf(pNetworkDbOri->lan2_gateway, sizeof(pNetworkDbOri->lan2_gateway), "%s", ui->gateway_lineEdit->text().trimmed().toStdString().c_str());
            } else {
                ui->ipaddr_lineEdit->setText(QString(pNetworkDb->lan2_ip_address));
                ui->netmask_lineEdit->setText(QString(pNetworkDb->lan2_netmask));
                ui->gateway_lineEdit->setText(QString(pNetworkDb->lan2_gateway));
            }
            ui->dns1_lineEdit->setText(QString(pNetworkDb->lan2_primary_dns));
            ui->dns2_lineEdit->setText(QString(pNetworkDb->lan2_second_dns));
        }
    }
}

void WizardPageNetwork::enableLAN_checkBoxSlot(bool state)
{
    if (state == true) {
        ui->enableDHCP_checkBox->setEnabled(true);
        ui->ipaddr_lineEdit->setEnabled(true);
        ui->netmask_lineEdit->setEnabled(true);
        ui->gateway_lineEdit->setEnabled(true);
        ui->dns1_lineEdit->setEnabled(true);
        ui->dns2_lineEdit->setEnabled(true);

        ui->label_ipaddr->setEnabled(true);
        ui->label_netmask->setEnabled(true);
        ui->label_gateway->setEnabled(true);
        ui->label_dnsAlt->setEnabled(true);
        ui->label_dnsPre->setEnabled(true);
    } else {
        ui->enableDHCP_checkBox->setChecked(false);
        ui->enableDHCP_checkBox->setEnabled(false);
        ui->ipaddr_lineEdit->setEnabled(false);
        ui->netmask_lineEdit->setEnabled(false);
        ui->gateway_lineEdit->setEnabled(false);
        ui->dns1_lineEdit->setEnabled(false);
        ui->dns2_lineEdit->setEnabled(false);

        ui->label_ipaddr->setEnabled(false);
        ui->label_netmask->setEnabled(false);
        ui->label_gateway->setEnabled(false);
        ui->label_dnsAlt->setEnabled(false);
        ui->label_dnsPre->setEnabled(false);
    }
}

void WizardPageNetwork::enableDHCP_checkBoxSlot(bool state)
{
    if (state == true) {
        ui->ipaddr_lineEdit->setEnabled(false);
        ui->netmask_lineEdit->setEnabled(false);
        ui->gateway_lineEdit->setEnabled(false);
        ui->dns1_lineEdit->setEnabled(false);
        ui->dns2_lineEdit->setEnabled(false);

        ui->label_ipaddr->setEnabled(false);
        ui->label_netmask->setEnabled(false);
        ui->label_gateway->setEnabled(false);
        ui->label_dnsAlt->setEnabled(false);
        ui->label_dnsPre->setEnabled(false);
    } else {
        ui->ipaddr_lineEdit->setEnabled(true);
        ui->netmask_lineEdit->setEnabled(true);
        ui->gateway_lineEdit->setEnabled(true);
        ui->dns1_lineEdit->setEnabled(true);
        ui->dns2_lineEdit->setEnabled(true);

        ui->label_ipaddr->setEnabled(true);
        ui->label_netmask->setEnabled(true);
        ui->label_gateway->setEnabled(true);
        ui->label_dnsAlt->setEnabled(true);
        ui->label_dnsPre->setEnabled(true);
    }
}
