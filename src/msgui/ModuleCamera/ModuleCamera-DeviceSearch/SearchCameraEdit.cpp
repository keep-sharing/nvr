#include "SearchCameraEdit.h"
#include "ui_SearchCameraEdit.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "myqt.h"
#include <QFile>
#include <QtDebug>

SearchCameraEdit::SearchCameraEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::SearchCameraEdit)
{
    ui->setupUi(this);
    setTitleWidget(ui->label_title);

    //
    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    initializeProtocol();

    //valid check
    ui->lineEdit_ip->setCheckMode(MyLineEdit::ServerCheck);
    ui->lineEdit_subnetMask->setCheckMode(MyLineEdit::IPCheck);
    ui->lineEdit_gateway->setCheckMode(MyLineEdit::IPCheck);
    ui->lineEdit_dns->setCheckMode(MyLineEdit::IPCheck);
    ui->lineEdit_port->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEdit_userName->setCheckMode(MyLineEdit::UserNameCheck);
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);
    //
    onLanguageChanged();
}

SearchCameraEdit::~SearchCameraEdit()
{
    delete ui;
}

void SearchCameraEdit::showEdit(const req_set_ipcaddr &set_ipcaddr)
{
    memset(&m_ipcaddrSource, 0, sizeof(req_set_ipcaddr));
    memcpy(&m_ipcaddrSource, &set_ipcaddr, sizeof(req_set_ipcaddr));

    ui->lineEdit_mac->setText(set_ipcaddr.mac);
    ui->comboBox_protocol->setCurrentIndexFromData(set_ipcaddr.protocol_id);
    ui->lineEdit_ip->setText(set_ipcaddr.oldipaddr);
    ui->lineEdit_subnetMask->setText(set_ipcaddr.netmask);
    ui->lineEdit_gateway->setText(set_ipcaddr.gateway);
    ui->lineEdit_dns->setText(set_ipcaddr.primarydns);
    ui->lineEdit_port->setText(QString::number(set_ipcaddr.port));
    ui->lineEdit_userName->setText(set_ipcaddr.username);
    ui->lineEdit_password->clear();
    on_comboBox_protocol_activated(ui->comboBox_protocol->currentIndex());
}

void SearchCameraEdit::showEdit(const req_set_ipcaddr_batch &set_ipcaddr)
{
    req_set_ipcaddr ipcaddr;
    memset(&ipcaddr, 0, sizeof(req_set_ipcaddr));
    memcpy(&ipcaddr, &set_ipcaddr, sizeof(req_set_ipcaddr));
    showEdit(ipcaddr);
}

req_set_ipcaddr SearchCameraEdit::currentIpcaddrInfo() const
{
    //qDebug()<<"SearchCamera oldip:"<< m_ipcaddr.oldipaddr <<" newip:"<<m_ipcaddr.newipaddr<<" mac:"<<m_ipcaddr.mac;
    return m_ipcaddr;
}

void SearchCameraEdit::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_IP_CONFLICT_BY_DEV:
        ON_RESPONSE_FLAG_IP_CONFLICT_BY_DEV(message);
        break;
    case RESPONSE_FLAG_SET_IPCADDR:
        ON_RESPONSE_FLAG_SET_IPCADDR(message);
        break;
    }
}

void SearchCameraEdit::ON_RESPONSE_FLAG_IP_CONFLICT_BY_DEV(MessageReceive *message)
{
    if (!message->data) {
        //MsWaitting::closeGlobalWait();
        qWarning() << "SearchCameraEdit::ON_RESPONSE_FLAG_IP_CONFLICT_BY_DEV";
        return;
    }
    int result = *((int *)message->data);
    switch (result) {
    case -1: {
        //MsWaitting::closeGlobalWait();
        if (MessageBox::question(this, GET_TEXT("CAMERASEARCH/32016", "Detect IP Conflict, Continue?")) == MessageBox::Yes) {
            //MsWaitting::showGlobalWait();
            sendMessage(REQUEST_FLAG_SET_IPCADDR, (void *)&m_ipcaddr, sizeof(m_ipcaddr));
        }
        break;
    }
    default:
        sendMessage(REQUEST_FLAG_SET_IPCADDR, (void *)&m_ipcaddr, sizeof(m_ipcaddr));
        break;
    }
}

void SearchCameraEdit::ON_RESPONSE_FLAG_SET_IPCADDR(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    if (!message->data) {
        qWarning() << "SearchCameraEdit::ON_RESPONSE_FLAG_SET_IPCADDR";
        return;
    }

    int ret = *((int *)message->data);
    switch (ret) {
    case 0:
        ShowMessageBox(GET_TEXT("CAMERASEARCH/32014", "Edit IP Address Successfully."));
        accept();
        break;
    case IPC_INVALID_USER:
        ShowMessageBox(GET_TEXT("MYLINETIP/112005", "Incorrect User Name or Password."));
        break;
    default:
        ShowMessageBox(GET_TEXT("CAMERASEARCH/32015", "Edit IP Address Failed."));
        break;
    }
}

void SearchCameraEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("CAMERASEARCH/32003", "IP Edit"));
    ui->label_mac->setText(GET_TEXT("CHANNELMANAGE/30027", "MAC"));
    ui->label_protocol->setText(GET_TEXT("CHANNELMANAGE/30014", "Protocol"));
    ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
    ui->label_subnetMask->setText(GET_TEXT("WIZARD/11026", "Subnet Mask"));
    ui->label_gateway->setText(GET_TEXT("COMMON/1034", "Gateway"));
    ui->label_dns->setText(GET_TEXT("CAMERASEARCH/32007", "DNS"));
    ui->label_port->setText(GET_TEXT("CHANNELMANAGE/30011", "Port"));
    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void SearchCameraEdit::initializeProtocol()
{
    QRegExp rx(QString("[0-9]*"));
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_port->setValidator(validator);
    ui->comboBox_protocol->clear();

    ipc_protocol *protocol_array = nullptr;
    int count = 0;
    read_ipc_protocols(SQLITE_FILE_NAME, &protocol_array, &count);
    if (protocol_array) {
        for (int i = 0; i < count; ++i) {
            const ipc_protocol &protocol = protocol_array[i];
            if (protocol.enable && protocol.function & IPC_FUNC_SEARCHABLE) {
                ui->comboBox_protocol->addItem(protocol.pro_name, protocol.pro_id);
            }
        }
    }
    release_ipc_protocol(&protocol_array);
}

bool SearchCameraEdit::isInputValid()
{
    bool valid = ui->lineEdit_ip->checkValid();
    valid = ui->lineEdit_subnetMask->checkValid() && valid;
    valid = ui->lineEdit_gateway->checkValid() && valid;
    valid = ui->lineEdit_dns->checkValid() && valid;
    valid = ui->lineEdit_port->checkValid() && valid;
    valid = ui->lineEdit_password->checkValid() && valid;
    valid = ui->lineEdit_userName->checkValid() && valid;
    if (!valid) {
        return false;
    }
    return true;
}
void SearchCameraEdit::on_comboBox_protocol_activated(int index)
{
    const IPC_PROTOCOL &protocol = static_cast<IPC_PROTOCOL>(ui->comboBox_protocol->itemData(index).toInt());
    switch (protocol) {
    case IPC_PROTOCOL_MILESIGHT:
    case IPC_PROTOCOL_NESA:
    case IPC_PROTOCOL_ALPHAFINITY:
        ui->label_port->show();
        ui->lineEdit_port->show();
        break;
    default:
        ui->label_port->hide();
        ui->lineEdit_port->hide();
        break;
    }
}

void SearchCameraEdit::on_pushButton_ok_clicked()
{
    if(!isInputValid()) {
        return ;
    }
    memset(&m_ipcaddr, 0, sizeof(m_ipcaddr));
    snprintf(m_ipcaddr.mac, sizeof(m_ipcaddr.mac), "%s", ui->lineEdit_mac->text().toStdString().c_str());
    snprintf(m_ipcaddr.newipaddr, sizeof(m_ipcaddr.newipaddr), "%s", ui->lineEdit_ip->text().toStdString().c_str());
    snprintf(m_ipcaddr.oldipaddr, sizeof(m_ipcaddr.oldipaddr), "%s", m_ipcaddrSource.oldipaddr);
    snprintf(m_ipcaddr.netmask, sizeof(m_ipcaddr.netmask), "%s", ui->lineEdit_subnetMask->text().toStdString().c_str());
    snprintf(m_ipcaddr.gateway, sizeof(m_ipcaddr.gateway), "%s", ui->lineEdit_gateway->text().toStdString().c_str());
    snprintf(m_ipcaddr.primarydns, sizeof(m_ipcaddr.primarydns), "%s", ui->lineEdit_dns->text().toStdString().c_str());
    snprintf(m_ipcaddr.username, sizeof(m_ipcaddr.username), "%s", ui->lineEdit_userName->text().toStdString().c_str());
    snprintf(m_ipcaddr.password, sizeof(m_ipcaddr.password), "%s", ui->lineEdit_password->text().toStdString().c_str());
    snprintf(m_ipcaddr.netif_form, sizeof(m_ipcaddr.netif_form), "%s", m_ipcaddrSource.netif_form);
    m_ipcaddr.protocol_id = ui->comboBox_protocol->currentData().toInt();
    m_ipcaddr.port = ui->lineEdit_port->text().toInt();
    ip_conflict conflict;
    if (QString(m_ipcaddr.newipaddr) != QString(m_ipcaddrSource.oldipaddr)) {
        memset(&conflict, 0, sizeof(conflict));
        snprintf(conflict.dev, sizeof(conflict.dev), "%s", m_ipcaddrSource.netif_form);
        snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", m_ipcaddr.newipaddr);
        sendMessage(REQUEST_FLAG_IP_CONFLICT_BY_DEV, (void *)&conflict, sizeof(conflict));
        //MsWaitting::showGlobalWait();
    } else if (QString(m_ipcaddr.netmask) != QString(m_ipcaddrSource.netmask) || QString(m_ipcaddr.gateway) != QString(m_ipcaddrSource.gateway) || QString(m_ipcaddr.primarydns) != QString(m_ipcaddrSource.primarydns) || m_ipcaddr.port != m_ipcaddrSource.port) {
        sendMessage(REQUEST_FLAG_SET_IPCADDR, (void *)&m_ipcaddr, sizeof(m_ipcaddr));
        //MsWaitting::showGlobalWait();
    } else {
        reject();
    }
}

void SearchCameraEdit::on_pushButton_cancel_clicked()
{
    reject();
}
