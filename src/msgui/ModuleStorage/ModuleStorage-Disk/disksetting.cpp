#include "disksetting.h"
#include "ui_disksetting.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDisk.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "addnetworkdisk.h"
#include "centralmessage.h"
#include "harddiskedit.h"
#include "msuser.h"
#include "networkdiskedit.h"
#include <QElapsedTimer>
#include <QtDebug>

extern "C" {
#include "log.h"
#include "msdb.h"
}

extern "C" {
#include "log.h"
#include "msdb.h"
}

const int DiskInfoRole = Qt::UserRole + 50;

DiskSetting::DiskSetting(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::DiskSetting)
{
    ui->setupUi(this);

    //initialize table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30011", "Port");
    headerList << GET_TEXT("CHANNELMANAGE/30028", "Vendor");
    headerList << GET_TEXT("MENU/10006", "Status");
    headerList << GET_TEXT("DISK/92003", "Total");
    headerList << GET_TEXT("DISK/92032", "Free");
    headerList << GET_TEXT("DISK/92016", "Property");
    headerList << GET_TEXT("COMMON/1052", "Type");
    headerList << GET_TEXT("DISK/92004", "Group");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderChecked(bool)));
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableClicked(int, int)));
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortableForColumn(ColumnEdit, false);
    ui->tableView->setSortableForColumn(ColumnDelete, false);
    ui->tableView->setSortType(ColumnPort, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnTotal, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnFree, SortFilterProxyModel::SortInt);
    //column width
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnPort, 100);
    ui->tableView->setColumnWidth(ColumnVendor, 300);
    ui->tableView->setColumnWidth(ColumnStatus, 125);
    ui->tableView->setColumnWidth(ColumnTotal, 150);
    ui->tableView->setColumnWidth(ColumnFree, 140);
    ui->tableView->setColumnWidth(ColumnProperty, 200);
    ui->tableView->setColumnWidth(ColumnType, 150);
    ui->tableView->setColumnWidth(ColumnGroup, 100);
    ui->tableView->setColumnWidth(ColumnEdit, 100);
    ui->tableView->setColumnWidth(ColumnDelete, 100);

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_GET_MSFS_DISKINFO, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_FORMAT_MSFS_DISK, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_DISK_INIT, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_DISK_LOAD, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_DEL_MSFS_NAS, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_REMOVE_RAID, this);

    onLanguageChanged();
}

DiskSetting::~DiskSetting()
{
    delete ui;
}

void DiskSetting::initializeData()
{
    m_selectDiskMap.clear();
    ui->tableView->clearSort();
    on_pushButton_refresh_clicked();
}

void DiskSetting::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void DiskSetting::filterMessage(MessageReceive *message)
{
    if (!isVisible()) {
        return;
    }

    switch (message->type()) {
    case RESPONSE_FLAG_GET_MSFS_DISKINFO:
        ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(message);
        break;
    case RESPONSE_FLAG_FORMAT_MSFS_DISK:
        ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(message);
        break;
    case RESPONSE_FLAG_PROGRESS_DISK_INIT:
        ON_RESPONSE_FLAG_PROGRESS_DISK_INIT(message);
        break;
    case RESPONSE_FLAG_PROGRESS_DISK_LOAD:
        ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(message);
        break;
    case RESPONSE_FLAG_DEL_MSFS_NAS:
        ON_RESPONSE_FLAG_DEL_MSFS_NAS(message);
        break;
    case RESPONSE_FLAG_REMOVE_RAID:
        ON_RESPONSE_FLAG_REMOVE_RAID(message);
        break;
    }
}

void DiskSetting::ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message)
{
    Q_UNUSED(message)

    refreshTable();

    //closeWait();
}

void DiskSetting::ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "DiskSetting::ON_RESPONSE_FLAG_FORMAT_MSFS_DISK, data is null.";
        return;
    }
    const int result = (*(int *)(message->data));
    switch (result) {
    case -1:
        ShowMessageBox(GET_TEXT("DISKMANAGE/72016", "Initialized failed."));
        break;
    case MF_NAS_LOW_CAP:
        ShowMessageBox(GET_TEXT("DISKMANAGE/72113", "The remain space of Network Disk is less than 20GB, cannot be initialized."));
        break;
    }

    refreshTable();
}

void DiskSetting::ON_RESPONSE_FLAG_PROGRESS_DISK_INIT(MessageReceive *message)
{
    Q_UNUSED(message)

    refreshTable();
}

void DiskSetting::ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(MessageReceive *message)
{
    Q_UNUSED(message)

    refreshTable();
}

void DiskSetting::ON_RESPONSE_FLAG_DEL_MSFS_NAS(MessageReceive *message)
{
    //closeWait();

    if (!message->data) {
        qWarning() << "DiskSetting::ON_RESPONSE_FLAG_DEL_MSFS_NAS, data is null.";
        return;
    }
    int result = (*(int *)(message->data));
    if (result == -1) {
        ShowMessageBox(GET_TEXT("DISK/92023", "Delete NAS false."));
    } else {
        refreshTable();
    }
}

void DiskSetting::ON_RESPONSE_FLAG_REMOVE_RAID(MessageReceive *message)
{
    //closeWait();
    if (!message->data) {
        qWarning() << "DiskSetting::ON_RESPONSE_FLAG_REMOVE_RAID, data is null.";
        return;
    }
    int result = *(int *)message->data;
    if (result == -1) {
        //失败
    }

    gMsDisk.getDiskInfo();
}

void DiskSetting::refreshTable()
{
    QMap<int, BaseDiskInfo> diskInfoMap = gMsDisk.diskInfoMap();
    //
    quint64 totalBytes = 0;
    quint64 freeBytes = 0;
    ui->tableView->clearContent();
    int row = 0;
    int networkDiskCount = 0;
    for (auto iter = diskInfoMap.constBegin(); iter != diskInfoMap.constEnd(); ++iter) {
        const BaseDiskInfo &disk = iter.value();
        if (disk.isShowInDiskWidget()) {
            ui->tableView->setRowCount(row + 1);
            if (m_selectDiskMap.contains(disk.port())) {
                ui->tableView->setItemChecked(row, m_selectDiskMap.value(disk.port()));
            } else {
                ui->tableView->setItemChecked(row, false);
            }
            ui->tableView->setItemIntValue(row, ColumnPort, disk.port());
            ui->tableView->setItemText(row, ColumnVendor, disk.vendorString());
            ui->tableView->setItemText(row, ColumnStatus, disk.statusString());
            ui->tableView->setItemDiskBytesValue(row, ColumnTotal, disk.totalBytes());
            ui->tableView->setItemDiskBytesValue(row, ColumnFree, disk.freeBytes());
            ui->tableView->setItemText(row, ColumnProperty, disk.propertyString());
            ui->tableView->setItemText(row, ColumnType, disk.typeString());
            ui->tableView->setItemText(row, ColumnGroup, disk.groupString());
            ui->tableView->setItemData(row, ColumnGroup, disk.groupValue(), SortIntRole);
            if (!disk.isEditEnable()) {
                ui->tableView->setItemText(row, ColumnEdit, "-");
            }
            if (!disk.isDeleteEnable()) {
                ui->tableView->setItemText(row, ColumnDelete, "-");
            }
            ui->tableView->setItemEnable(row, ColumnCheck, disk.isCheckable());
            ui->tableView->setItemData(row, ColumnPort, QVariant::fromValue(disk), DiskInfoRole);
            totalBytes += disk.totalBytes();
            freeBytes += disk.freeBytes();
            row++;

            switch (disk.typeValue()) {
            case DISK_TYPE_NAS:
            case DISK_TYPE_CIFS:
                networkDiskCount++;
                break;
            default:
                break;
            }
        }
    }
    ui->tableView->reorder();
    ui->lineEdit_total->setText(BaseDiskInfo::bytesString(totalBytes));
    ui->lineEdit_available->setText(BaseDiskInfo::bytesString(freeBytes));
    ui->pushButton_add->setEnabled(networkDiskCount < 8);
}

void DiskSetting::onLanguageChanged()
{
    ui->label_total->setText(GET_TEXT("DISK/92001", "Total Capacity"));
    ui->label_available->setText(GET_TEXT("DISKMANAGE/72028", "Available Capacity"));
    ui->labelNote->setText(GET_TEXT("DISK/92040", "Note: It is risky to the data safety if too much bandwidth is occupied by NAS."));

    ui->pushButton_add->setText(GET_TEXT("COMMON/1000", "Add"));
    ui->pushButton_refresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButton_initialize->setText(GET_TEXT("DISK/92025", "Initialize"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void DiskSetting::onTableHeaderChecked(bool checked)
{
    m_selectDiskMap.clear();
    for (int i = 0; i < ui->tableView->rowCount(); i++) {
        int port = ui->tableView->itemText(i, ColumnPort).toInt();
        if (ui->tableView->isRowEnable(i)) {
            m_selectDiskMap.insert(port, checked);
        }
    }
}

void DiskSetting::onTableClicked(int row, int column)
{
    switch (column) {
    case ColumnCheck:
        if (ui->tableView->isRowEnable(row)) {
            bool checked = ui->tableView->isItemChecked(row);
            int port = ui->tableView->itemText(row, ColumnPort).toInt();
            m_selectDiskMap.insert(port, checked);
        }
        break;
    case ColumnPort:
    case ColumnVendor:
    case ColumnStatus:
    case ColumnTotal:
    case ColumnFree:
    case ColumnProperty:
    case ColumnType:
    case ColumnGroup: {
        if (ui->tableView->isRowEnable(row)) {
            bool checked = ui->tableView->isItemChecked(row);
            ui->tableView->setItemChecked(row, !checked);
            int port = ui->tableView->itemText(row, ColumnPort).toInt();
            m_selectDiskMap.insert(port, !checked);
        }
        break;
    }
    case ColumnEdit: {
        if (!ui->tableView->itemText(row, ColumnEdit).isEmpty()) {
            break;
        }
        const BaseDiskInfo &info = ui->tableView->itemData(row, ColumnPort, DiskInfoRole).value<BaseDiskInfo>();
        switch (info.typeValue()) {
        case DISK_TYPE_LOCAL:
        case DISK_TYPE_ESATA:
        case DISK_TYPE_RAID: {
            if (!m_hardDiskEdit) {
                m_hardDiskEdit = new HardDiskEdit(this);
            }
            m_hardDiskEdit->setDiskInfo(info);
            int result = m_hardDiskEdit->exec();
            if (result == HardDiskEdit::Accepted) {
                on_pushButton_refresh_clicked();
            }
            break;
        }
        case DISK_TYPE_NAS:
        case DISK_TYPE_CIFS: {
            NetworkDiskEdit *networkDiskEdit = new NetworkDiskEdit(this);
            networkDiskEdit->setDiskPort(info.port());
            int result = networkDiskEdit->exec();
            if (result == HardDiskEdit::Accepted) {
                on_pushButton_refresh_clicked();
            }
            delete networkDiskEdit;
            break;
        }
        default:
            break;
        }
        break;
    }
    case ColumnDelete: {
        if (!ui->tableView->itemText(row, ColumnDelete).isEmpty()) {
            break;
        }
        const BaseDiskInfo &info = ui->tableView->itemData(row, ColumnPort, DiskInfoRole).value<BaseDiskInfo>();
        switch (info.typeValue()) {
        case DISK_TYPE_NAS:
        case DISK_TYPE_CIFS: {
            const int &result = MessageBox::question(this, GET_TEXT("DISK/92024", "Are you sure to delete NAS?"));
            if (result == MessageBox::Cancel) {
                break;
            }
            gMsDisk.removeNas(info.port());
            //showWait();

            //
            struct log_data l_data;
            struct op_lr_add_del_network_disk oland;
            memset(&l_data, 0, sizeof(l_data));
            memset(&oland, 0, sizeof(oland));
            snprintf(l_data.log_data_info.ip, sizeof(l_data.log_data_info.ip), "%s", info.address().toStdString().c_str());
            snprintf(l_data.log_data_info.user, sizeof(l_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
            l_data.log_data_info.mainType = MAIN_OP;
            l_data.log_data_info.subType = SUP_OP_DELETE_NETWORK_DISK_LOCK;
            l_data.log_data_info.chan_no = info.port();
            if (DISK_TYPE_NAS == info.typeValue()) {
                snprintf(oland.type, sizeof(oland.type), "%s", "NFS");
            } else if (DISK_TYPE_CIFS == info.typeValue()) {
                snprintf(oland.type, sizeof(oland.type), "%s", "SMB/CIFS");
            } else {
                snprintf(oland.type, sizeof(oland.type), "%d", info.typeValue());
            }
            snprintf(oland.ip, sizeof(oland.ip), "%s", info.address().toStdString().c_str());
            snprintf(oland.path, sizeof(oland.path), "%s", info.directory().toStdString().c_str());
            msfs_log_pack_detail(&l_data, OP_ADD_NETWORK_DISK, &oland, sizeof(oland));
            sendMessageOnly(REQUEST_FLAG_LOG_WRITE, &l_data, sizeof(struct log_data));
            break;
        }
        case DISK_TYPE_LOCAL:
        case DISK_TYPE_ESATA: {
            gMsDisk.removeDisk(info.port());
            on_pushButton_refresh_clicked();
            break;
        }
        case DISK_TYPE_RAID: {
            gMsDisk.removeRaid(info.port());
            //showWait();
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void DiskSetting::on_pushButton_refresh_clicked()
{
    gMsDisk.getDiskInfo();
    //showWait();
}

void DiskSetting::on_pushButton_add_clicked()
{
    AddNetworkDisk addNetworkDisk(this);
    int port = gMsDisk.validNetworkDiskPort();
    if (port < 0) {
        ShowMessageBox(GET_TEXT("DISK/92028", "You have reached Network Disk quantity limitation."));
        return;
    }
    addNetworkDisk.setDiskPort(port);
    addNetworkDisk.exec();

    on_pushButton_refresh_clicked();
}

void DiskSetting::on_pushButton_initialize_clicked()
{
    int count = 0;
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        if (ui->tableView->isItemChecked(i)) {
            const BaseDiskInfo &info = ui->tableView->itemData(i, ColumnPort, DiskInfoRole).value<BaseDiskInfo>();

            if (info.isReadOnly()) {
                ShowMessageBox("Read-only disk selected. Initialization cannot be performed.");
                return;
            }
            int port = info.port();
            if (gMsDisk.addFormatPort(port)) {
                count++;
            }
        }
    }
    if (count == 0) {
        ShowMessageBox(GET_TEXT("DISK/92018", "Please select at least one Disk."));
        return;
    }
    if (MessageBox::question(this, GET_TEXT("DISK/92027", "HDD data will be erased，including recorded video files, continue to initialize?")) == MessageBox::Cancel) {
        gMsDisk.clearFormatPort();
        return;
    }
    gMsDisk.startFormat();
    refreshTable();
}

void DiskSetting::on_pushButton_back_clicked()
{
    emit sig_back();
}
