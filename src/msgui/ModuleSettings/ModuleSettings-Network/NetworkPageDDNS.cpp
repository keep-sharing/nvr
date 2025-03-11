#include "NetworkPageDDNS.h"
#include "ui_NetworkPageDDNS.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QtDebug>

NetworkPageDDNS::NetworkPageDDNS(QWidget *parent)
    : AbstractNetworkPage(parent)
    , ui(new Ui::NetworkPageDDNS)
{
    ui->setupUi(this);

    initNetDDNSTab();

    ui->lineEdit_HTTP->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEdit_RTSP->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEdit_UserName->setCheckMode(MyLineEdit::UserNameCheck);
    ui->lineEdit_Password->setCheckMode(MyLineEdit::DDNSCheck);
    ui->lineEdit_HostName->setCheckMode(MyLineEdit::DDNSCheck);
    ui->lineEdit_Hash->setCheckMode(MyLineEdit::DDNSCheck);
    ui->lineEdit_DDNSURL->setCheckMode(MyLineEdit::DDNSCheck);

    onLanguageChanged();
}

NetworkPageDDNS::~NetworkPageDDNS()
{
    freeNetDDNStab();
    delete ui;
}

void NetworkPageDDNS::initializeData()
{
    gotoNetDDNStab();

    sendMessage(REQUEST_FLAG_GET_DDNS_STATUS, nullptr, 0);
}

void NetworkPageDDNS::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_DDNS_STATUS:
        ON_RESPONSE_FLAG_GET_DDNS_STATUS(message);
        break;
    }
}

void NetworkPageDDNS::ON_RESPONSE_FLAG_GET_DDNS_STATUS(MessageReceive *message)
{
    int result = *((int *)message->data);
    switch (result) {
    case 1:
        ui->lineEdit_status->setText(GET_TEXT("SYSTEMNETWORK/71018", "Running"));
        break;
    default:
        ui->lineEdit_status->setText(GET_TEXT("SYSTEMNETWORK/71019", "Not Running"));
        break;
    }
}

void NetworkPageDDNS::initNetDDNSTab()
{
    const struct device_info &sys_info = qMsNvr->deviceInfo();

    ui->comboBox_provider->clear();
    if (sys_info.oem_type == OEM_TYPE_STANDARD || sys_info.msddns != 0 || (QString(sys_info.company) == QString("Milesight"))) {
        ui->comboBox_provider->addItem(GET_TEXT("", "ddns.milesight.com"), GET_TEXT("", "ddns.milesight.com"));
    }
    ui->comboBox_provider->addItem(GET_TEXT("", "dyndns.org"), GET_TEXT("", "dyndns.org"));
    ui->comboBox_provider->addItem(GET_TEXT("", "freedns.afraid.org"), GET_TEXT("", "freedns.afraid.org"));
    ui->comboBox_provider->addItem(GET_TEXT("", "www.no-ip.com"), GET_TEXT("", "www.no-ip.com"));
    ui->comboBox_provider->addItem(GET_TEXT("", "www.zoneedit.com"), GET_TEXT("", "www.zoneedit.com"));
    ui->comboBox_provider->addItem(GET_TEXT("", "customize"), GET_TEXT("", "customize"));

    ui->comboBox_ddnsEnable->clear();
    ui->comboBox_ddnsEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_ddnsEnable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    connect(ui->comboBox_provider, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDdnsDomainChanged(int)));
    connect(ui->comboBox_ddnsEnable, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDdnsEnableChanged(int)));
    connect(ui->comboBox_ddnsEnable, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDdnsUseUPnP(int)));

    QRegExp numrx(QString("[0-9]*"));
    QValidator *numvalidator = new QRegExpValidator(numrx, this);
    ui->lineEdit_HTTP->setValidator(numvalidator);
    ui->lineEdit_RTSP->setValidator(numvalidator);

    pDDNS_Db = NULL;
    pDDNS_DbOri = NULL;
}

void NetworkPageDDNS::gotoNetDDNStab()
{
    if (pDDNS_Db == NULL) {
        pDDNS_Db = new struct ddns;
    }
    if (pDDNS_DbOri == NULL) {
        pDDNS_DbOri = new struct ddns;
    }
    memset(pDDNS_Db, 0, sizeof(struct ddns));
    memset(pDDNS_DbOri, 0, sizeof(struct ddns));

    read_ddns(SQLITE_FILE_NAME, pDDNS_Db);
    memcpy(pDDNS_DbOri, pDDNS_Db, sizeof(struct ddns));

    get_mac_addr(mac0, mac1);
    readDDNSConfig();
}

void NetworkPageDDNS::freeNetDDNStab()
{
    if (pDDNS_Db) {
        delete pDDNS_Db;
        pDDNS_Db = nullptr;
    }
    if (pDDNS_DbOri) {
        delete pDDNS_DbOri;
        pDDNS_DbOri = nullptr;
    }
}

void NetworkPageDDNS::readDDNSConfig()
{
    const struct device_info &sys_info = qMsNvr->deviceInfo();

    int index = ui->comboBox_provider->findData(QString(pDDNS_Db->domain));
    if (index == -1) {
        ui->comboBox_provider->setCurrentIndex(0);
        slotDdnsDomainChanged(0);
    } else {
        ui->comboBox_provider->setCurrentIndex(index);
        slotDdnsDomainChanged(index);
    }
    disconnect(ui->comboBox_ddnsEnable, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDdnsUseUPnP(int)));
    if (pDDNS_Db->enable) {
        ui->comboBox_ddnsEnable->setCurrentIndex(1);
        slotDdnsEnableChanged(1);
    } else {
        ui->comboBox_ddnsEnable->setCurrentIndex(0);
        slotDdnsEnableChanged(0);
    }
    connect(ui->comboBox_ddnsEnable, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDdnsUseUPnP(int)));
    if (pDDNS_Db->http_port) {
        ui->lineEdit_HTTP->setText(QString("%1").arg(pDDNS_Db->http_port));
    }
    if (pDDNS_Db->rtsp_port) {
        ui->lineEdit_RTSP->setText(QString("%1").arg(pDDNS_Db->rtsp_port));
    }
    ui->lineEdit_UserName->setText(QString(pDDNS_Db->username));
    ui->lineEdit_Password->setText(QString(pDDNS_Db->password));
    ui->lineEdit_Hash->setText(QString(pDDNS_Db->free_dns_hash));
    ui->lineEdit_DDNSURL->setText(QString(pDDNS_Db->ddnsurl));
    ui->lineEdit_HostName->setText(QString(pDDNS_Db->host_name));
    if (ui->comboBox_provider->currentIndex() == 0 && (sys_info.oem_type == OEM_TYPE_STANDARD || sys_info.msddns != 0 || (QString(sys_info.company) == QString("Milesight")))) {
        QString strurl = QString("http://ddns.milesight.com/");
        strurl += getMACStr(mac0);
        ui->lineEdit_DDNSURL->setText(strurl);
    }
}

void NetworkPageDDNS::saveDDNSConfig()
{
    if (ui->comboBox_ddnsEnable->currentIndex() == 1) {
        pDDNS_Db->enable = 1;
    } else {
        pDDNS_Db->enable = 0;
    }

    QString str = ui->comboBox_provider->itemData(ui->comboBox_provider->currentIndex(), Qt::UserRole).value<QString>();
    snprintf(pDDNS_Db->domain, sizeof(pDDNS_Db->domain), "%s", str.toStdString().c_str());

    pDDNS_Db->http_port = ui->lineEdit_HTTP->text().trimmed().toInt();
    pDDNS_Db->rtsp_port = ui->lineEdit_RTSP->text().trimmed().toInt();
    snprintf(pDDNS_Db->username, sizeof(pDDNS_Db->username), "%s", ui->lineEdit_UserName->text().trimmed().toStdString().c_str());
    snprintf(pDDNS_Db->password, sizeof(pDDNS_Db->password), "%s", ui->lineEdit_Password->text().trimmed().toStdString().c_str());
    snprintf(pDDNS_Db->free_dns_hash, sizeof(pDDNS_Db->free_dns_hash), "%s", ui->lineEdit_Hash->text().trimmed().toStdString().c_str());
    if (str != QString("ddns.milesight.com")) {
        snprintf(pDDNS_Db->ddnsurl, sizeof(pDDNS_Db->ddnsurl), "%s", ui->lineEdit_DDNSURL->text().trimmed().toStdString().c_str());
    }
    snprintf(pDDNS_Db->host_name, sizeof(pDDNS_Db->host_name), "%s", ui->lineEdit_HostName->text().trimmed().toStdString().c_str());
}

bool NetworkPageDDNS::isValidatorDDNSInput()
{
    if (ui->comboBox_ddnsEnable->currentIndex() == 0) { //disable 状态，不进行参数验证。
        return true;
    }

    QString str = ui->comboBox_provider->itemData(ui->comboBox_provider->currentIndex(), Qt::UserRole).value<QString>();
    if (str == GET_TEXT("", "")) {
        ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71125", "Provider cannot be empty."));
        ui->comboBox_provider->setFocus();
        return false;
    } else if ((str != GET_TEXT("", "freedns.afraid.org")) && (str != GET_TEXT("", "ddns.milesight.com"))) {
        bool valid = ui->lineEdit_UserName->checkValid();
        valid = ui->lineEdit_Password->checkValid() && valid;
        if (str == GET_TEXT("", "dyndns.org")) {
            if (!ui->lineEdit_DDNSURL->text().isEmpty() && !ui->lineEdit_DDNSURL->checkValid()) {
                valid = false;
            }
        }
        if (str != GET_TEXT("", "customize")) {
            if (!ui->lineEdit_HostName->checkValid()) {
                valid = false;
            }
        } else {
            if (!ui->lineEdit_DDNSURL->checkValid()) {
                valid = false;
            }
        }
        if (!valid) {
            return false;
        }
    } else if (str == GET_TEXT("", "freedns.afraid.org")) {
        bool valid = ui->lineEdit_HostName->checkValid();
        valid = ui->lineEdit_Hash->checkValid() && valid;
        if (!valid) {
            return false;
        }

    } else if (str == GET_TEXT("", "ddns.milesight.com")) {
        bool valid = ui->lineEdit_HTTP->checkValid();
        valid = ui->lineEdit_RTSP->checkValid() && valid;
        if (!valid) {
            return false;
        }
        if (ui->lineEdit_HTTP->text().trimmed().toInt() == ui->lineEdit_RTSP->text().trimmed().toInt()) {
            ui->lineEdit_HTTP->setCustomValid(false, GET_TEXT("SYSTEMNETWORK/71126", "Please use another port."));
            return false;
        }
    }

    struct upnp UPnP;
    read_upnp(SQLITE_FILE_NAME, &UPnP);
    if (UPnP.enable && (UPnP.http_port != ui->lineEdit_HTTP->text().trimmed().toInt() || UPnP.rtsp_port != ui->lineEdit_RTSP->text().trimmed().toInt())) {
        //MSHN-6793 QT-Network-DDNS：中性版本，UPnP已启用，当Enable DDNS时，会弹出提示“Do you want to use UPnP ports?”(异常)
        if (!qMsNvr->isOEM()) {
            if (MessageBox::Yes == MessageBox::question(this, GET_TEXT("SYSTEMNETWORK/71150", "Do you want to use UPnP ports?"))) {
                ui->lineEdit_HTTP->setText(QString("%1").arg(UPnP.http_port));
                ui->lineEdit_RTSP->setText(QString("%1").arg(UPnP.rtsp_port));
            }
        }
    }
    return true;
}

QString NetworkPageDDNS::getMACStr(char *mac)
{
    QString macStr;
    macStr += mac[0];
    macStr += mac[1];
    macStr += mac[3];
    macStr += mac[4];
    macStr += mac[6];
    macStr += mac[7];
    macStr += mac[9];
    macStr += mac[10];
    macStr += mac[12];
    macStr += mac[13];
    macStr += mac[15];
    macStr += mac[16];
    return macStr;
}

void NetworkPageDDNS::onLanguageChanged()
{
    ui->label_ddns->setText(GET_TEXT("SYSTEMNETWORK/71002", "DDNS"));
    ui->label_Provider->setText(GET_TEXT("SYSTEMNETWORK/71026", "Provider"));
    ui->label_HTTP->setText(GET_TEXT("SYSTEMNETWORK/71027", "External HTTP Port"));
    ui->label_RTSP->setText(GET_TEXT("SYSTEMNETWORK/71028", "External RTSP Port"));
    ui->label_DDNSURL->setText(GET_TEXT("SYSTEMNETWORK/71118", "DDNS URL"));
    ui->label_UserName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_Password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_Hash->setText(GET_TEXT("SYSTEMNETWORK/71119", "Hash"));
    ui->label_HostName->setText(GET_TEXT("SYSTEMNETWORK/71038", "Host Name"));
    ui->label_status->setText(GET_TEXT("SYSTEMNETWORK/71017", "DDNS Status"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->comboBox_ddnsEnable->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_ddnsEnable->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));
}

void NetworkPageDDNS::slotDdnsEnableChanged(int index)
{
    ui->comboBox_provider->setEnabled(index);
    ui->lineEdit_HTTP->setEnabled(index);
    ui->lineEdit_RTSP->setEnabled(index);
    ui->lineEdit_UserName->setEnabled(index);
    ui->lineEdit_Password->setEnabled(index);
    QString str = ui->comboBox_provider->itemData(ui->comboBox_provider->currentIndex(), Qt::UserRole).value<QString>();
    if (str == QString("customize") || str == QString("dyndns.org")) {
        ui->lineEdit_DDNSURL->setEnabled(index);
    }
    ui->lineEdit_Hash->setEnabled(index);
    ui->lineEdit_HostName->setEnabled(index);
}

void NetworkPageDDNS::slotDdnsUseUPnP(int index)
{
    if (index) {
        struct upnp UPnP;
        read_upnp(SQLITE_FILE_NAME, &UPnP);
        if (UPnP.enable && (UPnP.http_port != ui->lineEdit_HTTP->text().trimmed().toInt() || UPnP.rtsp_port != ui->lineEdit_RTSP->text().trimmed().toInt())) {
            //MSHN-6793 QT-Network-DDNS：中性版本，UPnP已启用，当Enable DDNS时，会弹出提示“Do you want to use UPnP ports?”(异常)
            if (!qMsNvr->isOEM()) {
                if (MessageBox::Yes == MessageBox::question(this, GET_TEXT("SYSTEMNETWORK/71150", "Do you want to use UPnP ports?"))) {
                    ui->lineEdit_HTTP->setText(QString("%1").arg(UPnP.http_port));
                    ui->lineEdit_RTSP->setText(QString("%1").arg(UPnP.rtsp_port));
                }
            }
        }
    }
}

void NetworkPageDDNS::slotDdnsDomainChanged(int index)
{
    ui->lineEdit_HTTP->setValid(true);
    ui->lineEdit_RTSP->setValid(true);
    ui->lineEdit_UserName->setValid(true);
    ui->lineEdit_Password->setValid(true);
    ui->lineEdit_HostName->setValid(true);
    ui->lineEdit_Hash->setValid(true);
    ui->lineEdit_DDNSURL->setValid(true);

    QString str = ui->comboBox_provider->itemData(index, Qt::UserRole).value<QString>();
    if (str == QString("customize")) {
        ui->label_HTTP->hide();
        ui->lineEdit_HTTP->hide();
        ui->label_RTSP->hide();
        ui->lineEdit_RTSP->hide();
        ui->label_UserName->show();
        ui->lineEdit_UserName->show();
        ui->label_Password->show();
        ui->lineEdit_Password->show();
        ui->label_DDNSURL->setText(GET_TEXT("SYSTEMNETWORK/71118", "DDNS URL"));
        ui->label_DDNSURL->show();
        ui->lineEdit_DDNSURL->show();
        ui->lineEdit_DDNSURL->setEnabled(true);
        ui->lineEdit_DDNSURL->clear();
        ui->lineEdit_DDNSURL->setText(QString(pDDNS_Db->ddnsurl));
        ui->label_Hash->hide();
        ui->lineEdit_Hash->hide();
        ui->label_HostName->hide();
        ui->lineEdit_HostName->hide();
    } else if (str == QString("ddns.milesight.com")) {
        ui->label_HTTP->show();
        ui->lineEdit_HTTP->show();
        ui->label_RTSP->show();
        ui->lineEdit_RTSP->show();
        ui->label_UserName->hide();
        ui->lineEdit_UserName->hide();
        ui->label_Password->hide();
        ui->lineEdit_Password->hide();
        ui->label_DDNSURL->setText(GET_TEXT("SYSTEMNETWORK/71118", "DDNS URL"));
        ui->label_DDNSURL->show();
        ui->lineEdit_DDNSURL->show();
        ui->lineEdit_DDNSURL->setEnabled(false);
        ui->label_Hash->hide();
        ui->lineEdit_Hash->hide();
        ui->label_HostName->hide();
        ui->lineEdit_HostName->hide();
        QString strurl = QString("http://ddns.milesight.com/");
        strurl += getMACStr(mac0);
        ui->lineEdit_DDNSURL->setText(strurl);
    } else if (str == QString("freedns.afraid.org")) {
        ui->label_HTTP->hide();
        ui->lineEdit_HTTP->hide();
        ui->label_RTSP->hide();
        ui->lineEdit_RTSP->hide();
        ui->label_UserName->hide();
        ui->lineEdit_UserName->hide();
        ui->label_Password->hide();
        ui->lineEdit_Password->hide();
        ui->label_DDNSURL->hide();
        ui->lineEdit_DDNSURL->hide();
        ui->label_Hash->show();
        ui->lineEdit_Hash->show();
        ui->label_HostName->show();
        ui->lineEdit_HostName->show();
    } else {
        ui->label_DDNSURL->hide();
        ui->lineEdit_DDNSURL->hide();
        if (str == QString("dyndns.org")) {
            ui->label_DDNSURL->setText(GET_TEXT("SYSTEMNETWORK/71205", "Host IP"));
            ui->label_DDNSURL->show();
            ui->lineEdit_DDNSURL->show();
            ui->lineEdit_DDNSURL->setEnabled(true);
            ui->lineEdit_DDNSURL->clear();
            ui->lineEdit_DDNSURL->setText(QString(pDDNS_Db->ddnsurl));
        }
        ui->label_HTTP->hide();
        ui->lineEdit_HTTP->hide();
        ui->label_RTSP->hide();
        ui->lineEdit_RTSP->hide();
        ui->label_UserName->show();
        ui->lineEdit_UserName->show();
        ui->label_Password->show();
        ui->lineEdit_Password->show();
        ui->label_Hash->hide();
        ui->lineEdit_Hash->hide();
        ui->label_HostName->show();
        ui->lineEdit_HostName->show();
    }
}

void NetworkPageDDNS::on_pushButton_apply_clicked()
{
    if (!isValidatorDDNSInput())
        return;
    saveDDNSConfig();
    if (memcmp(pDDNS_Db, pDDNS_DbOri, sizeof(struct ddns)) != 0) {
        memcpy(pDDNS_DbOri, pDDNS_Db, sizeof(struct ddns));
        write_ddns(SQLITE_FILE_NAME, pDDNS_Db);

        struct req_set_sysconf timeconf;
        memset(&timeconf, 0, sizeof(timeconf));
        snprintf(timeconf.arg, sizeof(timeconf.arg), "%s", "ddns");
        sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&timeconf, sizeof(timeconf));

        //TODO: log
        //DbLogWrite(current_user.username, SUB_OP_CONFIG_LOCAL, &ch_mask, SUB_PARAM_NETWORK_DDNS);
    }
}

void NetworkPageDDNS::on_pushButton_back_clicked()
{
    freeNetDDNStab();

    emit sig_back();
}
