#include "SearchCameraEditMulti.h"
#include "ui_SearchCameraEditMulti.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "myqt.h"
#include <QFile>
#include <QHostAddress>
#include <QtDebug>

SearchCameraEditMulti::SearchCameraEditMulti(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::SearchCameraEditMulti)
{
    ui->setupUi(this);

    //
    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    //table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CAMERASEARCH/32002", "NO.");
    headerList << GET_TEXT("CHANNELMANAGE/30027", "MAC");
    headerList << "Old IP";
    headerList << "New IP";
    headerList << "Result";
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->hideColumn(ColumnCheck);
    //sort
    ui->tableView->setSortType(ColumnNumber, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnOldIP, SortFilterProxyModel::SortIP);
    ui->tableView->setSortType(ColumnNewIP, SortFilterProxyModel::SortIP);
    //
    ui->tableView->setColumnWidth(ColumnNumber, 50);
    ui->tableView->setColumnWidth(ColumnMac, 150);
    ui->tableView->setColumnWidth(ColumnOldIP, 150);
    ui->tableView->setColumnWidth(ColumnNewIP, 150);

    //valid check
    ui->lineEdit_ip->setCheckMode(MyLineEdit::IPCheck);
    ui->lineEdit_subnetMask->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_gateway->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_dns->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_userName->setCheckMode(MyLineEdit::UserNameCheck);
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);

    onLanguageChanged();
}

SearchCameraEditMulti::~SearchCameraEditMulti()
{
    delete ui;
}

int SearchCameraEditMulti::execEdit(const QList<req_set_ipcaddr_batch> &list)
{
    m_listSetIpcAddr = list;

    const req_set_ipcaddr_batch &ipc = list.first();
    ui->lineEdit_ip->setText(ipc.oldipaddr);
    ui->lineEdit_subnetMask->setText(ipc.netmask);
    ui->lineEdit_gateway->setText(ipc.gateway);
    ui->lineEdit_dns->setText(ipc.primarydns);
    ui->lineEdit_userName->setText(ipc.username);
    ui->lineEdit_password->clear();

    ui->pushButton_ok->setEnabled(true);
    on_lineEdit_ip_textChanged(ui->lineEdit_ip->text());
    return exec();
}

void SearchCameraEditMulti::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCADDR_BATCH:
        ON_RESPONSE_FLAG_GET_IPCADDR_BATCH(message);
        break;
    }
}

void SearchCameraEditMulti::ON_RESPONSE_FLAG_GET_IPCADDR_BATCH(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();

    struct req_get_ipcaddr_batch *ipcaddr_batch = (struct req_get_ipcaddr_batch *)message->data;
    if (!ipcaddr_batch) {
        ShowMessageBox(GET_TEXT("CAMERASEARCH/32015", "Edit IP Address Failed."));
        return;
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const req_get_ipcaddr &get_ipcaddr = ipcaddr_batch->batch_ch[i];
        const QString strMac = get_ipcaddr.mac;
        if (strMac.isEmpty()) {
            continue;
        }
        for (int row = 0; row < ui->tableView->rowCount(); ++row) {
            const QString strRowMac = ui->tableView->itemText(row, ColumnMac);
            if (strRowMac == strMac) {
                QString strResult;
                switch (get_ipcaddr.cstate) {
                case IPC_OK:
                    strResult = "Succeed";
                    break;
                case IPC_NETWORK_ERR:
                    strResult = "Failed: IPC_NETWORK_ERR";
                    break;
                case IPC_INVALID_USER:
                    strResult = "Failed: IPC_INVALID_USER";
                    break;
                case IPC_UNKNOWN_ERR:
                    strResult = "Failed: IPC_UNKNOWN_ERR";
                    break;
                case IPC_PROTO_NOT_SUPPORT:
                    strResult = "Failed: IPC_PROTO_NOT_SUPPORT";
                    break;
                case IPC_OUT_LIMIT_BANDWIDTH:
                    strResult = "Failed: IPC_OUT_LIMIT_BANDWIDTH";
                    break;
                default:
                    strResult = "Failed";
                    break;
                }
                ui->tableView->setItemText(row, ColumnResult, strResult);
            }
        }
    }
}

bool SearchCameraEditMulti::isInputValid()
{
    bool valid = ui->lineEdit_ip->checkValid();
    valid = ui->lineEdit_subnetMask->checkValid() && valid;
    valid = ui->lineEdit_gateway->checkValid() && valid;
    valid = ui->lineEdit_dns->checkValid() && valid;
    valid = ui->lineEdit_password->checkValid() && valid;
    valid = ui->lineEdit_userName->checkValid() && valid;
    if (!valid) {
        return false;
    }
    return true;
}

void SearchCameraEditMulti::onLanguageChanged()
{
    ui->label_ip->setText(GET_TEXT("CAMERASEARCH/32020", "Start IP"));
    ui->label_subnetMask->setText(GET_TEXT("WIZARD/11026", "Subnet Mask"));
    ui->label_gateway->setText(GET_TEXT("COMMON/1034", "Gateway"));
    ui->label_dns->setText(GET_TEXT("CAMERASEARCH/32007", "DNS"));
    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void SearchCameraEditMulti::on_pushButton_ok_clicked()
{
    if (!isInputValid()) {
        return;
    }
    const int result = MessageBox::question(this, GET_TEXT("CAMERASEARCH/32036", "It needs a while to modify the cameras, continue?"));
    if (result == MessageBox::Cancel) {
        return;
    }
    //MsWaitting::showGlobalWait();
    const QString strStartIP = ui->lineEdit_ip->text();
    const QString strMask = ui->lineEdit_subnetMask->text();
    const QString strGateway = ui->lineEdit_gateway->text();
    const QString strDNS = ui->lineEdit_dns->text();
    const QString strUserName = ui->lineEdit_userName->text();
    const QString strPassword = ui->lineEdit_password->text();
    quint32 startIP = QHostAddress(strStartIP).toIPv4Address();
    qDebug() << QString("----begin set ip batch----");
    for (int i = 0; i < m_listSetIpcAddr.count(); ++i) {
        req_set_ipcaddr_batch &ipcaddr = m_listSetIpcAddr[i];
        QString strNewIP = QHostAddress(startIP).toString();
        snprintf(ipcaddr.newipaddr, sizeof(ipcaddr.newipaddr), "%s", strNewIP.toStdString().c_str());
        snprintf(ipcaddr.netmask, sizeof(ipcaddr.netmask), "%s", strMask.toStdString().c_str());
        snprintf(ipcaddr.gateway, sizeof(ipcaddr.gateway), "%s", strGateway.toStdString().c_str());
        snprintf(ipcaddr.primarydns, sizeof(ipcaddr.primarydns), "%s", strDNS.toStdString().c_str());
        snprintf(ipcaddr.username, sizeof(ipcaddr.username), "%s", strUserName.toStdString().c_str());
        snprintf(ipcaddr.password, sizeof(ipcaddr.password), "%s", strPassword.toStdString().c_str());
        ipcaddr.batchiNo = i;
        ipcaddr.batchAllCnt = m_listSetIpcAddr.count();
        qDebug() << QString("mac: %1, oldip: %2, newip: %3").arg(ipcaddr.mac).arg(ipcaddr.oldipaddr).arg(ipcaddr.newipaddr);
        sendMessageOnly(REQUEST_FLAG_SET_IPCADDR_BATCH, (void *)&ipcaddr, sizeof(req_set_ipcaddr_batch));
        startIP++;
    }
    qDebug() << QString("----end set ip batch----");
    sendMessage(REQUEST_FLAG_GET_IPCADDR_BATCH, nullptr, 0);

    ui->pushButton_ok->setEnabled(false);
}

void SearchCameraEditMulti::on_pushButton_cancel_clicked()
{
    if (ui->pushButton_ok->isEnabled()) {
        reject();
    } else {
        accept();
    }
}

void SearchCameraEditMulti::on_lineEdit_ip_textChanged(const QString &ip)
{
    quint32 startIP = QHostAddress(ip).toIPv4Address();
    //
    ui->tableView->clearContent();
    for (int i = 0; i < m_listSetIpcAddr.count(); ++i) {
        const req_set_ipcaddr_batch &ipcaddr = m_listSetIpcAddr.at(i);
        QHostAddress newAddress(startIP);
        QString strNewIP = newAddress.toString();
        ui->tableView->setItemIntValue(i, ColumnNumber, i + 1);
        ui->tableView->setItemText(i, ColumnMac, QString(ipcaddr.mac));
        ui->tableView->setItemText(i, ColumnOldIP, QString(ipcaddr.oldipaddr));
        ui->tableView->setItemText(i, ColumnNewIP, strNewIP);
        startIP++;
    }
}
