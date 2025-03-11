#include "addnetworkdisk.h"
#include "ui_addnetworkdisk.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "centralmessage.h"
#include "msuser.h"
#include "MyDebug.h"
#include "myqt.h"

extern "C" {
#include "log.h"
#include "msdb.h"
#include "log.h"
}

AddNetworkDisk::AddNetworkDisk(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::AddNetworkDisk)
{
    ui->setupUi(this);

    ui->comboBox_type->addItem(QString("NFS"), DISK_TYPE_NAS);
    //ui->comboBox_type->addItem(QString("SMB/CIFS"), DISK_TYPE_CIFS);

    QStringList headerLabels;
    headerLabels << GET_TEXT("CAMERASEARCH/32002", "No.");
    headerLabels << GET_TEXT("DISK/92006", "Directory");
    ui->tableView->setHorizontalHeaderLabels(headerLabels);
    ui->tableView->setColumnCount(headerLabels.count());
    ui->tableView->setSortingEnabled(false);
    ui->tableView->setHeaderCheckable(false);
    connect(ui->tableView, SIGNAL(itemClicked(int,int)), this, SLOT(onTableItemClicked(int,int)));

    m_waitting = new MsWaitting(this);
    m_waitting->setWindowModality(Qt::WindowModal);

    ui->lineEdit_ipAddress->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_directory->setCheckMode(MyLineEdit::NasDirectoryCheck);
    ui->lineEdit_networkDisk->setCheckMode(MyLineEdit::EmptyCheck);

    ui->lineEditUserName->setMaxLength(32);
    ui->lineEditPassword->setMaxLength(32);
    ui->lineEditPassword->setEchoMode(QLineEdit::Password);

    onLanguageChanged();
}

AddNetworkDisk::~AddNetworkDisk()
{
    delete ui;
}

void AddNetworkDisk::setDiskPort(int port)
{
    m_port = port;
    ui->lineEdit_networkDisk->setText(QString("Network Disk%1").arg(port - 16));

    ui->comboBox_type->setCurrentIndex(0);
}

void AddNetworkDisk::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_MSFS_NAS:
        ON_RESPONSE_FLAG_SEARCH_MSFS_NAS(message);
        break;
    case RESPONSE_FLAG_ADD_MSFS_NAS:
        ON_RESPONSE_FLAG_ADD_MSFS_NAS(message);
        break;
    }
}

void AddNetworkDisk::ON_RESPONSE_FLAG_SEARCH_MSFS_NAS(MessageReceive *message)
{
    //m_waitting->//closeWait();
    struct resp_search_nas_info *nas_info = (struct resp_search_nas_info *)message->data;
    if (!nas_info || nas_info->nCnt == 0) {
        ShowMessageBox(GET_TEXT("DISK/92020", "No result! Please check the IP Address."));
        return;
    }
    ui->tableView->clearContent();
    ui->tableView->setRowCount(nas_info->nCnt);
    for (int row = 0; row < nas_info->nCnt; ++row) {
        ui->tableView->setItemIntValue(row, 0, row + 1);
        ui->tableView->setItemText(row, 1, nas_info->directory[row]);
    }
}

void AddNetworkDisk::ON_RESPONSE_FLAG_ADD_MSFS_NAS(MessageReceive *message)
{
    Q_UNUSED(message)

    //m_waitting->//closeWait();
}

void AddNetworkDisk::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("DISK/92007", "Network Disk Add"));
    ui->label_networkDisk->setText(GET_TEXT("DISK/92008", "Network Disk"));
    ui->label_type->setText(GET_TEXT("COMMON/1052", "Type"));
    ui->label_ipAddress->setText(GET_TEXT("COMMON/1033", "IP Address"));
    ui->label_directory->setText(GET_TEXT("DISK/92006", "Directory"));
    ui->labelUserName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->labelPassword->setText(GET_TEXT("COMMON/1008", "Password"));

    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void AddNetworkDisk::onTableItemClicked(int row, int column)
{
    Q_UNUSED(column)

    ui->lineEdit_directory->setText(ui->tableView->itemText(row, 1));
}

void AddNetworkDisk::on_comboBox_type_indexSet(int index)
{
    int type = ui->comboBox_type->itemData(index).toInt();
    switch (type) {
    case DISK_TYPE_NAS:
        ui->labelUserName->hide();
        ui->labelPassword->hide();
        ui->lineEditUserName->hide();
        ui->lineEditPassword->hide();
        ui->tableView->show();
        ui->pushButton_search->show();
        break;
    case DISK_TYPE_CIFS:
        ui->labelUserName->show();
        ui->labelPassword->show();
        ui->lineEditUserName->show();
        ui->lineEditPassword->show();
        ui->tableView->hide();
        ui->pushButton_search->hide();
        break;
    default:
        qMsCritical() << "error index:" << index;
        break;
    }
}

void AddNetworkDisk::on_pushButton_search_clicked()
{
    bool valid = ui->lineEdit_ipAddress->checkValid();
    if (!valid) {
        return;
    }
    //m_waitting->//showWait();
    struct req_search_nas_info nas_info;
    memset(&nas_info, 0, sizeof(struct req_search_nas_info));
    nas_info.disk_port = m_port;
    snprintf(nas_info.disk_address, sizeof(nas_info.disk_address), "%s", ui->lineEdit_ipAddress->text().trimmed().toStdString().c_str());
    sendMessage(REQUEST_FLAG_SEARCH_MSFS_NAS, (void *)&nas_info, sizeof(struct req_search_nas_info));
}

void AddNetworkDisk::on_pushButton_ok_clicked()
{
    const QString &strName = ui->lineEdit_networkDisk->text().trimmed();
    bool valid = ui->lineEdit_networkDisk->checkValid();
    valid = ui->lineEdit_ipAddress->checkValid() && valid;
    valid = ui->lineEdit_directory->checkValid() && valid;
    if (!valid) {
        return;
    }
    const QString &strIP = ui->lineEdit_ipAddress->text().trimmed();
    const QString &strDirectory = ui->lineEdit_directory->text().trimmed();
    int type = ui->comboBox_type->currentIntData();

    //


    int dbCount = 0;
    diskInfo db_disk_array[MAX_DISK_NUM];
    read_disks(SQLITE_FILE_NAME, db_disk_array, &dbCount);
    for (int i = 0; i < dbCount; ++i) {
        const diskInfo &disk = db_disk_array[i];
        if (disk.disk_port == m_port && disk.enable) {
            MessageBox::information(this, "The information has changed. Please try again.");
            close();
            return;
        }
        if (QString(disk.disk_address) == strIP && QString(disk.disk_directory) == strDirectory) {
            ui->lineEdit_networkDisk->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
            return;
        }
    }
    //
    struct diskInfo disk;
    memset(&disk, 0, sizeof(struct diskInfo));
    disk.disk_port = m_port;
    disk.enable = 1;
    disk.disk_type = type;
    disk.disk_property = 1;
    snprintf(disk.disk_vendor, sizeof(disk.disk_vendor), "%s", strName.toStdString().c_str());
    snprintf(disk.disk_address, sizeof(disk.disk_address), "%s", strIP.toStdString().c_str());
    snprintf(disk.disk_directory, sizeof(disk.disk_directory), "%s", strDirectory.toStdString().c_str());
    if (type == DISK_TYPE_CIFS) {
        snprintf(disk.user, sizeof(disk.user), "%s", ui->lineEditUserName->text().toStdString().c_str());
        snprintf(disk.password, sizeof(disk.password), "%s", ui->lineEditPassword->text().toStdString().c_str());
    }
    write_disk(SQLITE_FILE_NAME, &disk);
    //
    sendMessage(REQUEST_FLAG_ADD_MSFS_NAS, (void *)&(m_port), sizeof(int));
    m_waitting->execWait();
    MessageBox::information(this,  GET_TEXT("DISK/150002", "Add Network Disk Successfully."));
    close();

    //
    struct log_data l_data;
    struct op_lr_add_del_network_disk oland;
    memset(&l_data, 0, sizeof(l_data));
    memset(&oland, 0, sizeof(oland));
    snprintf(l_data.log_data_info.ip, sizeof(l_data.log_data_info.ip), "%s", disk.disk_address);
    snprintf(l_data.log_data_info.user, sizeof(l_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    l_data.log_data_info.mainType = MAIN_OP;
    l_data.log_data_info.subType = SUP_OP_ADD_NETWORK_DISK_LOCK;
    l_data.log_data_info.chan_no = disk.disk_port;
    if (DISK_TYPE_NAS == disk.disk_type) {
        snprintf(oland.type, sizeof(oland.type), "%s", "NFS");
    } else if (DISK_TYPE_CIFS == disk.disk_type) {
        snprintf(oland.type, sizeof(oland.type), "%s", "SMB/CIFS");
    } else {
        snprintf(oland.type, sizeof(oland.type), "%d", disk.disk_type);
    }
    snprintf(oland.ip, sizeof(oland.ip), "%s", disk.disk_address);
    snprintf(oland.path, sizeof(oland.path), "%s", disk.disk_directory);
    msfs_log_pack_detail(&l_data, OP_ADD_NETWORK_DISK, &oland, sizeof(oland));
    sendMessageOnly(REQUEST_FLAG_LOG_WRITE, &l_data, sizeof(struct log_data));
}

void AddNetworkDisk::on_pushButton_cancel_clicked()
{
    close();
}
