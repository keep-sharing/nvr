#include "networkdiskedit.h"
#include "ui_networkdiskedit.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "msuser.h"

extern "C" {
#include "log.h"
#include "msdb.h"
}

NetworkDiskEdit::NetworkDiskEdit(QWidget *parent)
    : AbstractDiskEdit(parent)
    , ui(new Ui::NetworkDiskEdit)
{
    ui->setupUi(this);

    ui->comboBox_type->clear();
    ui->comboBox_type->addItem("NFS", DISK_TYPE_NAS);
    //ui->comboBox_type->addItem(QString("SMB/CIFS"), DISK_TYPE_CIFS);

    ui->comboBox_group->clear();
    for (int i = 0; i < 16; ++i) {
        ui->comboBox_group->addItem(QString::number(i + 1), i);
    }

    ui->comboBox_property->clear();
    ui->comboBox_property->addItem(GET_TEXT("DISKMANAGE/72107", "R/W"), 1);
    ui->comboBox_property->addItem(GET_TEXT("DISKMANAGE/72108", "Read-only"), 0);

    ui->lineEditUserName->setMaxLength(32);
    ui->lineEditPassword->setMaxLength(32);
    ui->lineEditPassword->setEchoMode(QLineEdit::Password);

    ui->lineEdit_directory->setCheckMode(MyLineEdit::NasDirectoryCheck);

    QStringList headerLabels;
    headerLabels << "";
    headerLabels << GET_TEXT("CAMERASEARCH/32002", "No.");
    headerLabels << GET_TEXT("DISK/92006", "Directory");
    ui->tableView->setHorizontalHeaderLabels(headerLabels);
    ui->tableView->setColumnCount(headerLabels.count());
    ui->tableView->hideColumn(ColumnCheck);
    ui->tableView->setSortingEnabled(false);
    connect(ui->tableView, SIGNAL(itemClicked(int,int)), this, SLOT(onTableItemClicked(int,int)));

    ui->lineEdit_address->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_directory->setCheckMode(MyLineEdit::NasDirectoryCheck);
    ui->lineEdit_networkDisk->setCheckMode(MyLineEdit::EmptyCheck);

    onLanguageChanged();
}

NetworkDiskEdit::~NetworkDiskEdit()
{
    delete ui;
}

void NetworkDiskEdit::setDiskPort(int port)
{
    m_port = port;
    diskInfo disk;
    memset(&disk, 0, sizeof(disk));
    read_disk(SQLITE_FILE_NAME, &disk, m_port);

    ui->lineEdit_networkDisk->setText(disk.disk_vendor);
    ui->comboBox_group->setCurrentIndexFromData(disk.disk_group);
    ui->comboBox_group->setEnabled(BaseDiskInfo::is_group_enable);
    ui->comboBox_property->setCurrentIndexFromData(disk.disk_property);
    ui->lineEdit_address->setText(disk.disk_address);
    QString directory(disk.disk_directory);
    directory.remove(":");
    ui->lineEdit_directory->setText(directory);
    ui->lineEditUserName->setText(disk.user);
    ui->lineEditPassword->setText(disk.password);

    ui->comboBox_type->setCurrentIndexFromData(disk.disk_type);
}

void NetworkDiskEdit::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_MSFS_NAS:
        ON_RESPONSE_FLAG_SEARCH_MSFS_NAS(message);
        break;
    default:
        break;
    }
}

bool NetworkDiskEdit::isNoChange()
{
    QString strAddress = ui->lineEdit_address->text();
    QString strDirectory = ui->lineEdit_directory->text();

    if (strAddress == m_address && strDirectory == m_directory) {
        return true;
    } else {
        return false;
    }
}

void NetworkDiskEdit::ON_RESPONSE_FLAG_SEARCH_MSFS_NAS(MessageReceive *message)
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
        ui->tableView->setItemIntValue(row, ColumnNo, row + 1);
        ui->tableView->setItemText(row, ColumnDirectory, nas_info->directory[row]);
    }
}

void NetworkDiskEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("DISK/92015", "Network Disk Edit"));
    ui->label_networkDisk->setText(GET_TEXT("DISK/92008", "Network Disk"));
    ui->label_type->setText(GET_TEXT("COMMON/1052", "Type"));
    ui->label_group->setText(GET_TEXT("DISK/92004", "Group"));
    ui->label_property->setText(GET_TEXT("DISK/92016", "Property"));
    ui->label_address->setText(GET_TEXT("COMMON/1033", "IP Address"));
    ui->label_directory->setText(GET_TEXT("DISK/92006", "Directory"));
    ui->labelUserName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->labelPassword->setText(GET_TEXT("COMMON/1008", "Password"));

    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void NetworkDiskEdit::onTableItemClicked(int row, int column)
{
    Q_UNUSED(column)

    ui->lineEdit_directory->setText(ui->tableView->itemText(row, ColumnDirectory));
}

void NetworkDiskEdit::on_comboBox_type_indexSet(int index)
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

void NetworkDiskEdit::on_pushButton_search_clicked()
{
    bool valid = ui->lineEdit_address->checkValid();
    if (!valid) {
        return;
    }
    struct req_search_nas_info nas_info;
    memset(&nas_info, 0, sizeof(struct req_search_nas_info));
    nas_info.disk_port = m_port;
    snprintf(nas_info.disk_address, sizeof(nas_info.disk_address), "%s", ui->lineEdit_address->text().trimmed().toStdString().c_str());
    sendMessage(REQUEST_FLAG_SEARCH_MSFS_NAS, (void *)&nas_info, sizeof(struct req_search_nas_info));

    //m_waitting->//showWait();
}

void NetworkDiskEdit::on_pushButton_ok_clicked()
{
    bool valid = true;
    valid &= ui->lineEdit_networkDisk->checkValid();
    valid &= ui->lineEdit_address->checkValid();
    valid &= ui->lineEdit_directory->checkValid();
    if (!valid) {
        return;
    }

    QString strVendor = ui->lineEdit_networkDisk->text();
    QString strAddress = ui->lineEdit_address->text();
    QString strDirectory = ui->lineEdit_directory->text();
    int group = ui->comboBox_group->currentData().toInt();
    int property = ui->comboBox_property->currentData().toInt();
    int type = ui->comboBox_type->currentIntData();

    //
    int dbCount = 0;
    diskInfo db_disk_array[MAX_DISK_NUM];
    read_disks(SQLITE_FILE_NAME, db_disk_array, &dbCount);
    for (int i = 0; i < dbCount; ++i) {
        const diskInfo &disk = db_disk_array[i];
        if (disk.disk_port == m_port) {
            continue;
        }
        if (QString(disk.disk_address) == strAddress && QString(disk.disk_directory) == strDirectory) {
            ui->lineEdit_networkDisk->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
            return;
        }
    }

    if (!property) {
        const int result = MessageBox::question(this, GET_TEXT("DISK/92031", "Recording will stop after changing to Read-Only Mode，continue？"));
        if (result == MessageBox::Cancel) {
            return;
        }
    }

    struct diskInfo disk;
    memset(&disk, 0, sizeof(struct diskInfo));
    disk.enable = 1;
    disk.disk_type = type;
    disk.disk_property = property;
    disk.disk_group = group;
    disk.disk_port = m_port;
    snprintf(disk.disk_vendor, sizeof(disk.disk_vendor), "%s", strVendor.toStdString().c_str());
    snprintf(disk.disk_address, sizeof(disk.disk_address), "%s", strAddress.toStdString().c_str());
    snprintf(disk.disk_directory, sizeof(disk.disk_directory), "%s", strDirectory.toStdString().c_str());
    if (type == DISK_TYPE_CIFS) {
        snprintf(disk.user, sizeof(disk.user), "%s", ui->lineEditUserName->text().toStdString().c_str());
        snprintf(disk.password, sizeof(disk.password), "%s", ui->lineEditPassword->text().toStdString().c_str());
    }
    write_disk(SQLITE_FILE_NAME, &disk);
    sendMessage(REQUEST_FLAG_UPDATE_MSFS_NAS, (void *)&disk.disk_port, sizeof(disk.disk_port));

    MessageBox::information(this,  GET_TEXT("DISK/150001", "Edit Network Disk Successfully."));
    accept();

    //
    struct log_data l_data;
    struct op_lr_add_del_network_disk oland;
    memset(&l_data, 0, sizeof(l_data));
    memset(&oland, 0, sizeof(oland));
    snprintf(l_data.log_data_info.ip, sizeof(l_data.log_data_info.ip), "%s", disk.disk_address);
    snprintf(l_data.log_data_info.user, sizeof(l_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    l_data.log_data_info.mainType = MAIN_OP;
    l_data.log_data_info.subType = SUP_OP_CONFIG_NETWORK_DISK_LOCK;
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

void NetworkDiskEdit::on_pushButton_cancel_clicked()
{
    reject();
}
