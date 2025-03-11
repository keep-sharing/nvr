#include "NetworkPageSNMP.h"
#include "ui_NetworkPageSNMP.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "centralmessage.h"

#define MINPORT 1024
#define MAXPORT 65535

NetworkPageSNMP::NetworkPageSNMP(QWidget *parent)
    : AbstractNetworkPage(parent)
    , ui(new Ui::NetworkPageSNMP)
{
    ui->setupUi(this);

    initNetSnmpTab();
    ui->lineEdit_readCommunity->setCheckMode(MyLineEdit::SnmpCheck);
    ui->lineEdit_writeCommunity->setCheckMode(MyLineEdit::SnmpCheck);
    ui->lineEdit_rdSecurityName->setCheckMode(MyLineEdit::SnmpCheck);
    ui->lineEdit_wrSecurityName->setCheckMode(MyLineEdit::SnmpCheck);
    ui->lineEdit_snmpPort->setCheckMode(MyLineEdit::SpecialRangeCheck, 1024, 65535, 161);
    ui->lineEdit_rdAuthPsw->setCheckMode(MyLineEdit::SnmpPasswordCheck);
    ui->lineEdit_rdPrivKeyPsw->setCheckMode(MyLineEdit::SnmpPasswordCheck);
    ui->lineEdit_wrAuthPsw->setCheckMode(MyLineEdit::SnmpPasswordCheck);
    ui->lineEdit_wrPrivKeyPsw->setCheckMode(MyLineEdit::SnmpPasswordCheck);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
}

NetworkPageSNMP::~NetworkPageSNMP()
{
    freeNetSnmpTab();
    delete ui;
}

void NetworkPageSNMP::initNetSnmpTab()
{
    onLanguageChanged();

    QRegExp rx(QString("[0-9]*"));
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_snmpPort->setValidator(validator);

    ui->lineEdit_rdAuthPsw->setEchoMode(QLineEdit::Password);
    ui->lineEdit_rdPrivKeyPsw->setEchoMode(QLineEdit::Password);
    ui->lineEdit_wrAuthPsw->setEchoMode(QLineEdit::Password);
    ui->lineEdit_wrPrivKeyPsw->setEchoMode(QLineEdit::Password);

    memset(&NetSnmpDb, 0x0, sizeof(NetSnmpDb));
    memset(&NetSnmpDbOri, 0x0, sizeof(NetSnmpDbOri));

    memset(&netMore, 0x0, sizeof(netMore));

    //connect
    connect(ui->comboBox_snmpv1Enable, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSnmpv1Changed(int)));
    connect(ui->comboBox_snmpv2cEnable, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSnmpv2cChanged(int)));
    connect(ui->comboBox_snmpv3Enable, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsnmpv3Changed(int)));
    connect(ui->comboBox_rdLevelOfSecurity, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRdLevelOfSecurityChanged(int)));
    connect(ui->comboBox_wrLevelOfsecurity, SIGNAL(currentIndexChanged(int)), this, SLOT(slotWrLevelOfSecurityChanged(int)));
}

void NetworkPageSNMP::gotoNetSnmpTab()
{
    memset(&NetSnmpDb, 0x0, sizeof(NetSnmpDb));
    memset(&NetSnmpDbOri, 0x0, sizeof(NetSnmpDbOri));

    //read && fill data
    readSnmpConfig();

    //more port
    memset(&netMore, 0x0, sizeof(netMore));
    read_port(SQLITE_FILE_NAME, &netMore);

    if (pEmailPort == nullptr) {
        pEmailPort = new struct email;
    }
    memset(pEmailPort, 0, sizeof(struct email));
    read_email(SQLITE_FILE_NAME, pEmailPort);

    //set disable & hide
    snmpSetV1V2cEnable();
    snmpSetV3Enable();
    snmpSetSecurity();
}

void NetworkPageSNMP::freeNetSnmpTab()
{
    if (pEmailPort) {
        delete pEmailPort;
        pEmailPort = nullptr;
    }
}

void NetworkPageSNMP::initializeData()
{
    onLanguageChanged();
    gotoNetSnmpTab();
}

void NetworkPageSNMP::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void NetworkPageSNMP::snmpSetV1V2cEnable()
{
    if (ui->comboBox_snmpv1Enable->currentIndex() == 0
        && ui->comboBox_snmpv2cEnable->currentIndex() == 0) {
        ui->lineEdit_readCommunity->setEnabled(false);
        ui->lineEdit_writeCommunity->setEnabled(false);
    } else {
        ui->lineEdit_readCommunity->setEnabled(true);
        ui->lineEdit_writeCommunity->setEnabled(true);
    }
}

void NetworkPageSNMP::snmpSetV3Enable()
{
    if (ui->comboBox_snmpv3Enable->currentIndex() == 0) {
        ui->lineEdit_rdSecurityName->setEnabled(false);
        ui->comboBox_rdLevelOfSecurity->setEnabled(false);
        ui->lineEdit_wrSecurityName->setEnabled(false);
        ui->comboBox_wrLevelOfsecurity->setEnabled(false);
    } else {
        ui->lineEdit_rdSecurityName->setEnabled(true);
        ui->comboBox_rdLevelOfSecurity->setEnabled(true);
        ui->lineEdit_wrSecurityName->setEnabled(true);
        ui->comboBox_wrLevelOfsecurity->setEnabled(true);
    }
}

void NetworkPageSNMP::snmpSetSecurity()
{
    int snmpv3 = ui->comboBox_snmpv3Enable->currentIndex();
    if (snmpv3 == 1) {
        int rdLevelOfSecurity = ui->comboBox_rdLevelOfSecurity->currentIndex();
        if (rdLevelOfSecurity == 0) {
            ui->label_rdAuthAlgo->show();
            ui->label_rdAuthPsw->show();
            ui->label_rdPrivKeyAlgo->show();
            ui->label_rdPrivKeyPsw->show();

            ui->comboBox_rdAuthAlgo->show();
            ui->lineEdit_rdAuthPsw->show();
            ui->comboBox_rdPrivKeyAlgo->show();
            ui->lineEdit_rdPrivKeyPsw->show();
        } else if (rdLevelOfSecurity == 1) {
            ui->label_rdAuthAlgo->show();
            ui->label_rdAuthPsw->show();
            ui->label_rdPrivKeyAlgo->hide();
            ui->label_rdPrivKeyPsw->hide();

            ui->comboBox_rdAuthAlgo->show();
            ui->lineEdit_rdAuthPsw->show();
            ui->comboBox_rdPrivKeyAlgo->hide();
            ui->lineEdit_rdPrivKeyPsw->hide();
        } else {
            ui->label_rdAuthAlgo->hide();
            ui->label_rdAuthPsw->hide();
            ui->label_rdPrivKeyAlgo->hide();
            ui->label_rdPrivKeyPsw->hide();

            ui->comboBox_rdAuthAlgo->hide();
            ui->lineEdit_rdAuthPsw->hide();
            ui->comboBox_rdPrivKeyAlgo->hide();
            ui->lineEdit_rdPrivKeyPsw->hide();
        }

        int wrLevelOfSecurity = ui->comboBox_wrLevelOfsecurity->currentIndex();
        if (wrLevelOfSecurity == 0) {
            ui->label_wrAuthAlgo->show();
            ui->label_wrAuthPsw->show();
            ui->label_wrPrivKeyAlgo->show();
            ui->label_wrPrivKeyPsw->show();

            ui->comboBox_wrAuthAlgo->show();
            ui->lineEdit_wrAuthPsw->show();
            ui->comboBox_wrPrivKeyAlgo->show();
            ui->lineEdit_wrPrivKeyPsw->show();
        } else if (wrLevelOfSecurity == 1) {
            ui->label_wrAuthAlgo->show();
            ui->label_wrAuthPsw->show();
            ui->label_wrPrivKeyAlgo->hide();
            ui->label_wrPrivKeyPsw->hide();

            ui->comboBox_wrAuthAlgo->show();
            ui->lineEdit_wrAuthPsw->show();
            ui->comboBox_wrPrivKeyAlgo->hide();
            ui->lineEdit_wrPrivKeyPsw->hide();
        } else {
            ui->label_wrAuthAlgo->hide();
            ui->label_wrAuthPsw->hide();
            ui->label_wrPrivKeyAlgo->hide();
            ui->label_wrPrivKeyPsw->hide();

            ui->comboBox_wrAuthAlgo->hide();
            ui->lineEdit_wrAuthPsw->hide();
            ui->comboBox_wrPrivKeyAlgo->hide();
            ui->lineEdit_wrPrivKeyPsw->hide();
        }
    } else {
        ui->label_rdAuthAlgo->hide();
        ui->label_rdAuthPsw->hide();
        ui->label_rdPrivKeyAlgo->hide();
        ui->label_rdPrivKeyPsw->hide();
        ui->comboBox_rdAuthAlgo->hide();
        ui->lineEdit_rdAuthPsw->hide();
        ui->comboBox_rdPrivKeyAlgo->hide();
        ui->lineEdit_rdPrivKeyPsw->hide();

        ui->label_wrAuthAlgo->hide();
        ui->label_wrAuthPsw->hide();
        ui->label_wrPrivKeyAlgo->hide();
        ui->label_wrPrivKeyPsw->hide();
        ui->comboBox_wrAuthAlgo->hide();
        ui->lineEdit_wrAuthPsw->hide();
        ui->comboBox_wrPrivKeyAlgo->hide();
        ui->lineEdit_wrPrivKeyPsw->hide();
    }
}

bool NetworkPageSNMP::isValidatorSnmpInput()
{
    bool valid = ui->lineEdit_readCommunity->checkValid();
    valid = ui->lineEdit_writeCommunity->checkValid() && valid;
    valid = ui->lineEdit_rdSecurityName->checkValid() && valid;

    if (ui->comboBox_snmpv3Enable->currentIndex() == 1 && ui->comboBox_rdLevelOfSecurity->currentIndex() != 2) {
        if (!ui->lineEdit_rdAuthPsw->checkValid()) {
            valid = false;
        }
    }

    if (ui->comboBox_snmpv3Enable->currentIndex() == 1 && ui->comboBox_rdLevelOfSecurity->currentIndex() == 0) {
        if (!ui->lineEdit_rdPrivKeyPsw->checkValid()) {
            valid = false;
        }
    }
    valid = ui->lineEdit_wrSecurityName->checkValid() && valid;

    if (ui->comboBox_snmpv3Enable->currentIndex() == 1 && ui->comboBox_wrLevelOfsecurity->currentIndex() != 2) {
        if (!ui->lineEdit_wrAuthPsw->checkValid()) {
            valid = false;
        }
    }

    if (ui->comboBox_snmpv3Enable->currentIndex() == 1 && ui->comboBox_wrLevelOfsecurity->currentIndex() == 0) {
        if (!ui->lineEdit_wrPrivKeyPsw->checkValid()) {
            valid = false;
        }
    }
    valid = ui->lineEdit_snmpPort->checkValid() && valid;

    int snmpPort = ui->lineEdit_snmpPort->text().trimmed().toInt();

    if (ui->lineEdit_wrSecurityName->checkValid()
        && ui->lineEdit_rdSecurityName->checkValid()
        && ui->comboBox_snmpv3Enable->currentIndex() == 1
        && ui->lineEdit_wrSecurityName->text() == ui->lineEdit_rdSecurityName->text()
        && !ui->lineEdit_wrSecurityName->text().isEmpty()) {
        ui->lineEdit_wrSecurityName->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
        valid = false;
    }

    //端口冲突

    if (!ui->lineEdit_snmpPort->text().isEmpty() && (snmpPort == netMore.ssh_port || snmpPort == netMore.http_port || snmpPort == netMore.rtsp_port || snmpPort == netMore.https_port || snmpPort == netMore.posPort || snmpPort == pEmailPort->port)) {
        ui->lineEdit_snmpPort->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
        valid = false;
    }
    if (!ui->lineEdit_snmpPort->text().isEmpty() && NetSnmpDbOther.port != snmpPort) {
        if (is_port_check(snmpPort) || is_port_use(snmpPort)) {
            ui->lineEdit_snmpPort->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
            valid = false;
        }
    }
    if (!valid) {
        return false;
    }
    return true;
}

void NetworkPageSNMP::saveSnmpConfig()
{
    NetSnmpDb.v1_enable = ui->comboBox_snmpv1Enable->currentIndex();
    NetSnmpDb.v2c_enable = ui->comboBox_snmpv2cEnable->currentIndex();
    snprintf(NetSnmpDb.read_community, sizeof(NetSnmpDb.read_community), "%s", ui->lineEdit_readCommunity->text().trimmed().toStdString().c_str());
    snprintf(NetSnmpDb.write_community, sizeof(NetSnmpDb.write_community), "%s", ui->lineEdit_writeCommunity->text().trimmed().toStdString().c_str());

    NetSnmpDb.v3_enable = ui->comboBox_snmpv3Enable->currentIndex();
    snprintf(NetSnmpDb.read_security_name, sizeof(NetSnmpDb.read_security_name), "%s", ui->lineEdit_rdSecurityName->text().trimmed().toStdString().c_str());
    NetSnmpDb.read_level_security = ui->comboBox_rdLevelOfSecurity->currentIndex();
    NetSnmpDb.read_auth_algorithm = ui->comboBox_rdAuthAlgo->currentIndex();
    if (ui->comboBox_snmpv3Enable->currentIndex() == 1 && ui->comboBox_rdLevelOfSecurity->currentIndex() != 2)
        snprintf(NetSnmpDb.read_auth_password, sizeof(NetSnmpDb.read_auth_password), "%s", ui->lineEdit_rdAuthPsw->text().trimmed().toStdString().c_str());
    NetSnmpDb.read_pri_algorithm = ui->comboBox_rdPrivKeyAlgo->currentIndex();
    if (ui->comboBox_snmpv3Enable->currentIndex() == 1 && ui->comboBox_rdLevelOfSecurity->currentIndex() == 0)
        snprintf(NetSnmpDb.read_pri_password, sizeof(NetSnmpDb.read_pri_password), "%s", ui->lineEdit_rdPrivKeyPsw->text().trimmed().toStdString().c_str());

    snprintf(NetSnmpDb.write_security_name, sizeof(NetSnmpDb.write_security_name), "%s", ui->lineEdit_wrSecurityName->text().trimmed().toStdString().c_str());
    NetSnmpDb.write_level_security = ui->comboBox_wrLevelOfsecurity->currentIndex();
    NetSnmpDb.write_auth_algorithm = ui->comboBox_wrAuthAlgo->currentIndex();
    if (ui->comboBox_snmpv3Enable->currentIndex() == 1 && ui->comboBox_wrLevelOfsecurity->currentIndex() != 2)
        snprintf(NetSnmpDb.write_auth_password, sizeof(NetSnmpDb.write_auth_password), "%s", ui->lineEdit_wrAuthPsw->text().trimmed().toStdString().c_str());
    NetSnmpDb.write_pri_algorithm = ui->comboBox_wrPrivKeyAlgo->currentIndex();
    if (ui->comboBox_snmpv3Enable->currentIndex() == 1 && ui->comboBox_wrLevelOfsecurity->currentIndex() == 0)
        snprintf(NetSnmpDb.write_pri_password, sizeof(NetSnmpDb.write_pri_password), "%s", ui->lineEdit_wrPrivKeyPsw->text().trimmed().toStdString().c_str());

    NetSnmpDb.port = ui->lineEdit_snmpPort->text().trimmed().toInt();
}

void NetworkPageSNMP::readSnmpConfig()
{
    read_snmp(SQLITE_FILE_NAME, &NetSnmpDb);
    memcpy(&NetSnmpDbOri, &NetSnmpDb, sizeof(snmp));

    //fill data
    ui->comboBox_snmpv1Enable->setCurrentIndex(NetSnmpDb.v1_enable);
    ui->comboBox_snmpv2cEnable->setCurrentIndex(NetSnmpDb.v2c_enable);
    ui->lineEdit_readCommunity->setText(NetSnmpDb.read_community);
    ui->lineEdit_writeCommunity->setText(NetSnmpDb.write_community);

    ui->comboBox_snmpv3Enable->setCurrentIndex(NetSnmpDb.v3_enable);
    ui->lineEdit_rdSecurityName->setText(NetSnmpDb.read_security_name);
    ui->comboBox_rdLevelOfSecurity->setCurrentIndex(NetSnmpDb.read_level_security);
    ui->comboBox_rdAuthAlgo->setCurrentIndex(NetSnmpDb.read_auth_algorithm);
    ui->lineEdit_rdAuthPsw->setText(NetSnmpDb.read_auth_password);
    ui->comboBox_rdPrivKeyAlgo->setCurrentIndex(NetSnmpDb.read_pri_algorithm);
    ui->lineEdit_rdPrivKeyPsw->setText(NetSnmpDb.read_pri_password);

    ui->lineEdit_wrSecurityName->setText(NetSnmpDb.write_security_name);
    ui->comboBox_wrLevelOfsecurity->setCurrentIndex(NetSnmpDb.write_level_security);
    ui->comboBox_wrAuthAlgo->setCurrentIndex(NetSnmpDb.write_auth_algorithm);
    ui->lineEdit_wrAuthPsw->setText(NetSnmpDb.write_auth_password);
    ui->comboBox_wrPrivKeyAlgo->setCurrentIndex(NetSnmpDb.write_pri_algorithm);
    ui->lineEdit_wrPrivKeyPsw->setText(NetSnmpDb.write_pri_password);

    ui->lineEdit_snmpPort->setText(QString("%1").arg(NetSnmpDb.port));
}

void NetworkPageSNMP::writeSnmpConfig()
{
    write_snmp(SQLITE_FILE_NAME, &NetSnmpDb);
    memcpy(&NetSnmpDbOri, &NetSnmpDb, sizeof(snmp));
}

void NetworkPageSNMP::setReadSnmpVaild()
{
    ui->lineEdit_rdAuthPsw->setValid(true);
    ui->lineEdit_rdPrivKeyPsw->setValid(true);
}

void NetworkPageSNMP::setWriteSnmpValid()
{
    ui->lineEdit_wrAuthPsw->setValid(true);
    ui->lineEdit_wrPrivKeyPsw->setValid(true);
}

int NetworkPageSNMP::is_port_use(int port)
{
    FILE *fp;
    int result = 0;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "netstat -an | grep -E \"LISTEN\" | grep \"0.0.0.0:%d \" > /tmp/port", port);
    ms_system(cmd);
    fp = fopen("/tmp/port", "rb");
    if (!fp)
        return 0;

    while (fgets(cmd, sizeof(cmd), fp)) {
        if (cmd[0] != '\0') {
            result = 1;
            break;
        }
    }
    fclose(fp);
    if (result) {
        return result;
    }

    snprintf(cmd, sizeof(cmd), "netstat -an | grep -E \"ESTABLISHED|WAITTING|FIN_WAIT\" | grep \":%d \" > /tmp/port", port);
    ms_system(cmd);
    fp = fopen("/tmp/port", "rb");
    if (!fp)
        return 0;

    while (fgets(cmd, sizeof(cmd), fp)) {
        if (cmd[0] != '\0') {
            result = 1;
            ms_system("rm -rf /tmp/test");
            ms_system("cp /tmp/port /tmp/test");
            break;
        }
    }
    fclose(fp);

    ms_system("rm -f /tmp/port");
    if (result) {
        snprintf(cmd, sizeof(cmd), "%s", "grep \"::ffff\" /tmp/test > /tmp/port");
        ms_system(cmd);
        fp = fopen("/tmp/port", "rb");
        if (!fp)
            return 0;

        while (fgets(cmd, sizeof(cmd), fp)) {
            if (cmd[0] != '\0') {
                result = 0;
                break;
            }
        }
        fclose(fp);
        ms_system("rm -f /tmp/port");
    }

    return result;
}

int NetworkPageSNMP::is_port_check(int port)
{
    int max_poe_num = get_param_int(SQLITE_FILE_NAME, PARAM_POE_NUM, 0);
    if (max_poe_num <= 0)
        return 0;
    if (max_poe_num == 4) {
        if (port >= 23001 && port <= 23004) {
            return 1;
        }
    } else if (max_poe_num == 8) {
        if (port >= 23001 && port <= 23008) {
            return 1;
        }
    } else if (max_poe_num == 16) {
        if (port >= 23001 && port <= 23016) {
            return 1;
        }
    }
    return 0;
}

void NetworkPageSNMP::onLanguageChanged()
{
    ui->label_snmpv1->setText(GET_TEXT("SYSTEMNETWORK/71166", "SNMP V1"));
    ui->label_snmpv2c->setText(GET_TEXT("SYSTEMNETWORK/71167", "SNMP V2c"));
    ui->label_readCommunity->setText(GET_TEXT("SYSTEMNETWORK/71168", "Read Community"));
    ui->label_writeCommunity->setText(GET_TEXT("SYSTEMNETWORK/71169", "Write Community"));

    ui->label_snmpv3->setText(GET_TEXT("SYSTEMNETWORK/71170", "SNMP V3"));
    ui->label_rdSecurityName->setText(GET_TEXT("SYSTEMNETWORK/71171", "Read Security Name"));
    ui->label_rdLevelOfSecurity->setText(GET_TEXT("SYSTEMNETWORK/71172", "Level of Security"));
    ui->label_rdAuthAlgo->setText(GET_TEXT("SYSTEMNETWORK/71173", "Authentication Algorithm"));
    ui->label_rdAuthPsw->setText(GET_TEXT("SYSTEMNETWORK/71174", "Authentication Password"));
    ui->label_rdPrivKeyAlgo->setText(GET_TEXT("SYSTEMNETWORK/71175", "Private Key Algorithm"));
    ui->label_rdPrivKeyPsw->setText(GET_TEXT("SYSTEMNETWORK/71176", "Private Key Password"));

    ui->label_wrSecurityName->setText(GET_TEXT("SYSTEMNETWORK/71177", "Write Security Name"));
    ui->label_wrLevelOfsecurity->setText(GET_TEXT("SYSTEMNETWORK/71172", "Level of Security"));
    ui->label_wrAuthAlgo->setText(GET_TEXT("SYSTEMNETWORK/71173", "Authentication Algorithm"));
    ui->label_wrAuthPsw->setText(GET_TEXT("SYSTEMNETWORK/71174", "Authentication Password"));
    ui->label_wrPrivKeyAlgo->setText(GET_TEXT("SYSTEMNETWORK/71175", "Private Key Algorithm"));
    ui->label_wrPrivKeyPsw->setText(GET_TEXT("SYSTEMNETWORK/71176", "Private Key Password"));

    ui->label_snmpPort->setText(GET_TEXT("SYSTEMNETWORK/71186", "SNMP Port"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->comboBox_snmpv1Enable->clear();
    ui->comboBox_snmpv1Enable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_snmpv1Enable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_snmpv2cEnable->clear();
    ui->comboBox_snmpv2cEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_snmpv2cEnable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_snmpv3Enable->clear();
    ui->comboBox_snmpv3Enable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_snmpv3Enable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_rdLevelOfSecurity->clear();
    ui->comboBox_rdLevelOfSecurity->addItem(GET_TEXT("SYSTEMNETWORK/71179", "auth,priv"), 0);
    ui->comboBox_rdLevelOfSecurity->addItem(GET_TEXT("SYSTEMNETWORK/71180", "auth,no priv"), 1);
    ui->comboBox_rdLevelOfSecurity->addItem(GET_TEXT("SYSTEMNETWORK/71181", "no auth,no priv"), 2);

    ui->comboBox_rdAuthAlgo->clear();
    ui->comboBox_rdAuthAlgo->addItem(GET_TEXT("SYSTEMNETWORK/71182", "MD5"), 0);
    ui->comboBox_rdAuthAlgo->addItem(GET_TEXT("SYSTEMNETWORK/71183", "SHA"), 1);

    ui->comboBox_rdPrivKeyAlgo->clear();
    ui->comboBox_rdPrivKeyAlgo->addItem(GET_TEXT("SYSTEMNETWORK/71184", "DES"), 0);
    ui->comboBox_rdPrivKeyAlgo->addItem(GET_TEXT("SYSTEMNETWORK/71185", "AES"), 1);

    ui->comboBox_wrLevelOfsecurity->clear();
    ui->comboBox_wrLevelOfsecurity->addItem(GET_TEXT("SYSTEMNETWORK/71179", "auth,priv"), 0);
    ui->comboBox_wrLevelOfsecurity->addItem(GET_TEXT("SYSTEMNETWORK/71180", "auth,no priv"), 1);
    ui->comboBox_wrLevelOfsecurity->addItem(GET_TEXT("SYSTEMNETWORK/71181", "no auth,no priv"), 2);

    ui->comboBox_wrAuthAlgo->clear();
    ui->comboBox_wrAuthAlgo->addItem(GET_TEXT("SYSTEMNETWORK/71182", "MD5"), 0);
    ui->comboBox_wrAuthAlgo->addItem(GET_TEXT("SYSTEMNETWORK/71183", "MD5"), 1);

    ui->comboBox_wrPrivKeyAlgo->clear();
    ui->comboBox_wrPrivKeyAlgo->addItem(GET_TEXT("SYSTEMNETWORK/71184", "DES"), 0);
    ui->comboBox_wrPrivKeyAlgo->addItem(GET_TEXT("SYSTEMNETWORK/71185", "AES"), 1);
}

void NetworkPageSNMP::slotSnmpv1Changed(int index)
{
    Q_UNUSED(index)

    snmpSetV1V2cEnable();
}

void NetworkPageSNMP::slotSnmpv2cChanged(int index)
{
    Q_UNUSED(index)

    snmpSetV1V2cEnable();
}

void NetworkPageSNMP::slotsnmpv3Changed(int index)
{
    Q_UNUSED(index)

    snmpSetV3Enable();
    snmpSetSecurity();
    setReadSnmpVaild();
    setWriteSnmpValid();
}

void NetworkPageSNMP::slotRdLevelOfSecurityChanged(int index)
{
    Q_UNUSED(index)

    snmpSetSecurity();
    setReadSnmpVaild();
}

void NetworkPageSNMP::slotWrLevelOfSecurityChanged(int index)
{
    Q_UNUSED(index)

    snmpSetSecurity();
    setWriteSnmpValid();
}

bool NetworkPageSNMP::saveSetting()
{
    memset(&NetSnmpDbOther, 0, sizeof(snmp));
    read_snmp(SQLITE_FILE_NAME, &NetSnmpDbOther);
    if (!isValidatorSnmpInput()) {
        return false;
    }
    if (ui->comboBox_snmpv1Enable->currentIndex() == 1 || ui->comboBox_snmpv2cEnable->currentIndex() == 1) {
        const int &result = MessageBox::question(this, GET_TEXT("SYSTEMNETWORK/71210", "There is security risk if SNMP V1 or SNMP V2c is enabled, continue?"));
        if (result != MessageBox::Yes) {
            return false;
        }
    }

    saveSnmpConfig();

    if (memcmp(&NetSnmpDb, &NetSnmpDbOri, sizeof(snmp)) != 0) {
        writeSnmpConfig();

        struct req_set_sysconf snmpconf;
        memset(&snmpconf, 0, sizeof(snmpconf));
        snprintf(snmpconf.arg, sizeof(snmpconf.arg), "%s", "snmp");
        sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&snmpconf, sizeof(snmpconf));

        //TODO: log
        //DbLogWrite(current_user.username, SUB_OP_CONFIG_LOCAL, &ch_mask, SUB_PARAM_NETWORK);

        //showWait();
        QEventLoop eventloop;
        QTimer::singleShot(2000, &eventloop, SLOT(quit()));
        eventloop.exec();
        //closeWait();
        ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71112", "Save successfully!"));
    }
    return true;
}

void NetworkPageSNMP::on_pushButton_apply_clicked()
{
    saveSetting();
}

void NetworkPageSNMP::on_pushButton_back_clicked()
{
    emit sig_back();
}
