#include "PageRaidSetting.h"
#include "ui_PageRaidSetting.h"
#include "EventLoop.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsDisk.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "createraid.h"
#include "rebulidraid.h"

const int DiskInfoRole = Qt::UserRole + 50;

PageRaidSetting::PageRaidSetting(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::RaidSetting)
{
    ui->setupUi(this);

    initializeDiskTable();
    initializeArrayTable();

    ui->groupBox_disk->setVisible(false);
    ui->groupBox_array->setVisible(false);

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_GET_MSFS_DISKINFO, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_DISK_LOAD, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_REMOVE_RAID, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_RAID_REBUILD, this);

    onLanguageChanged();
}

PageRaidSetting::~PageRaidSetting()
{
    delete ui;
}

void PageRaidSetting::initializeData()
{
    //showWait();
    ui->tableView_disk->clearSort();
    ui->tableView_array->clearSort();
    sendMessage(REQUEST_FLAG_GET_RAID_MODE, nullptr, 0);
}

void PageRaidSetting::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_RAID_MODE:
        ON_RESPONSE_FLAG_GET_RAID_MODE(message);
        break;
    case RESPONSE_FLAG_GET_NETWORK_BANDWIDTH:
        ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH(message);
        break;
    case RESPONSE_FLAG_CREATE_RAID:
        ON_RESPONSE_FLAG_CREATE_RAID(message);
        break;
    case RESPONSE_FLAG_CREATE_SPACE:
        ON_RESPONSE_FLAG_CREATE_SPACE(message);
        break;
    case RESPONSE_FLAG_REMOVE_SPACE:
        ON_RESPONSE_FLAG_REMOVE_SPACE(message);
        break;
    }
}

void PageRaidSetting::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_MSFS_DISKINFO:
        ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(message);
        break;
    case RESPONSE_FLAG_PROGRESS_DISK_LOAD:
        ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(message);
        break;
    case RESPONSE_FLAG_REMOVE_RAID:
        ON_RESPONSE_FLAG_REMOVE_RAID(message);
        break;
    case RESPONSE_FLAG_PROGRESS_RAID_REBUILD:
        ON_RESPONSE_FLAG_PROGRESS_RAID_REBUILD(message);
        break;
    default:
        break;
    }
}

void PageRaidSetting::ON_RESPONSE_FLAG_GET_RAID_MODE(MessageReceive *message)
{
    if (!message->data) {
        //closeWait();
        qWarning() << "RaidSetting::ON_RESPONSE_FLAG_GET_RAID_MODE, data is null.";
        return;
    }
    int mode = *((int *)message->data);
    m_isRaidEnable = (mode == 1);
    if (m_isRaidEnable) {
        ui->groupBox_disk->setVisible(true);
        ui->groupBox_array->setVisible(true);
    }
    if (m_isRaidEnable) {
        ui->checkBox_enable->setChecked(true);
        gMsDisk.getDiskInfo();
    } else {
        ui->checkBox_enable->setChecked(false);
        //closeWait();
    }
}

void PageRaidSetting::ON_RESPONSE_FLAG_CREATE_SPACE(MessageReceive *message)
{
    Q_UNUSED(message)

    gMsDisk.getDiskInfo();
}

void PageRaidSetting::ON_RESPONSE_FLAG_REMOVE_SPACE(MessageReceive *message)
{
    Q_UNUSED(message)

    gMsDisk.getDiskInfo();
}

void PageRaidSetting::ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH(MessageReceive *message)
{
    //closeWait();

    if (message->data == NULL) {
        return;
    }
    struct resp_network_bandwidth *info = (struct resp_network_bandwidth *)message->data;
    if (info->free / 1024 < 120) {
        ShowMessageBox(GET_TEXT("RAID/93013", "RAID requires at least 120Mbps bandwidth, the NVR currently does not meet the requirement."));
        return;
    } else {

        const int result = MessageBox::question(this, GET_TEXT("RAID/93008", "RAID mode will take effect after rebooting. Do you want to reboot now?"));
        if (result == MessageBox::Cancel) {
            return;
        }
    }
}

void PageRaidSetting::ON_RESPONSE_FLAG_CREATE_RAID(MessageReceive *message)
{
    int result = *(int *)message->data;
    gEventLoopExit(result);
}

void PageRaidSetting::ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message)
{
    Q_UNUSED(message)

    if (!isVisible()) {
        return;
    }

    refreshDiskTable();

    //closeWait();
}

void PageRaidSetting::ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(MessageReceive *message)
{
    Q_UNUSED(message)

    if (!isVisible()) {
        return;
    }

    refreshDiskTable();
}

void PageRaidSetting::ON_RESPONSE_FLAG_REMOVE_RAID(MessageReceive *message)
{
    if (!isVisible()) {
        return;
    }

    //closeWait();
    if (!message->data) {
        qWarning() << "RaidSetting::ON_RESPONSE_FLAG_REMOVE_RAID, data is null.";
        return;
    }
    int result = *(int *)message->data;
    if (result == -1) {
        //失败
    }

    gMsDisk.getDiskInfo();
}

void PageRaidSetting::ON_RESPONSE_FLAG_PROGRESS_RAID_REBUILD(MessageReceive *message)
{
    Q_UNUSED(message)

    if (!isVisible()) {
        return;
    }

    refreshDiskTable();
}

void PageRaidSetting::resizeEvent(QResizeEvent *event)
{
    int tableWidth = width() - 50;

    //disk column width
    ui->tableView_disk->setColumnWidth(DiskColumnCheck, 50);
    ui->tableView_disk->setColumnWidth(DiskColumnPort, 100);
    ui->tableView_disk->setColumnWidth(DiskColumnStatus, 200);
    ui->tableView_disk->setColumnWidth(DiskColumnCapacity, 200);
    ui->tableView_disk->setColumnWidth(DiskColumnType, 200);
    ui->tableView_disk->setColumnWidth(DiskColumnVendor, tableWidth - 950);

    //array column width
    ui->tableView_array->setColumnWidth(ArrayColumnNumber, 50);

    QWidget::resizeEvent(event);
}

void PageRaidSetting::initializeDiskTable()
{
    //header
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30011", "Port");
    headerList << GET_TEXT("CHANNELMANAGE/30028", "Vendor");
    headerList << GET_TEXT("DISKMANAGE/72109", "Status");
    headerList << GET_TEXT("RAID/93004", "Capacity");
    headerList << GET_TEXT("COMMON/1052", "Type");
    headerList << GET_TEXT("DISKMANAGE/72064", "Hot Spare");
    ui->tableView_disk->setHorizontalHeaderLabels(headerList);
    ui->tableView_disk->setColumnCount(headerList.size());
    connect(ui->tableView_disk, SIGNAL(itemClicked(int, int)), this, SLOT(onTableDiskClicked(int, int)));
    //delegate
    ui->tableView_disk->setItemDelegateForColumn(DiskColumnHotSpare, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonSwitch, this));
    //sort
    ui->tableView_disk->setSortableForColumn(DiskColumnCheck, false);
    ui->tableView_disk->setSortableForColumn(DiskColumnHotSpare, false);
    ui->tableView_disk->setSortType(DiskColumnPort, SortFilterProxyModel::SortInt);
    ui->tableView_disk->setSortType(DiskColumnCapacity, SortFilterProxyModel::SortInt);
}

void PageRaidSetting::initializeArrayTable()
{
    //header
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CAMERASEARCH/32002", "No.");
    headerList << GET_TEXT("COMMON/1051", "Name");
    headerList << GET_TEXT("RAID/93001", "Physical Disk");
    headerList << GET_TEXT("DISK/92013", "RAID Capacity");
    headerList << GET_TEXT("DISKMANAGE/72109", "Status");
    headerList << GET_TEXT("RAID/93007", "Level");
    headerList << GET_TEXT("DISKMANAGE/72064", "Hot Spare");
    headerList << GET_TEXT("DISKMANAGE/72065", "Rebuild");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    headerList << GET_TEXT("RAID/93005", "Task");
    ui->tableView_array->setHorizontalHeaderLabels(headerList);
    ui->tableView_array->setColumnCount(headerList.size());
    ui->tableView_array->setHeaderCheckable(false);
    connect(ui->tableView_array, SIGNAL(itemClicked(int, int)), this, SLOT(onTableArrayClicked(int, int)));
    //
    ui->tableView_array->setItemDelegateForColumn(ArrayColumnRebulid, new ItemButtonDelegate(QPixmap(":/common/common/rebuild.png"), this));
    ui->tableView_array->setItemDelegateForColumn(ArrayColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //sort
    ui->tableView_array->setSortableForColumn(ArrayColumnCheck, false);
    ui->tableView_array->setSortableForColumn(ArrayColumnRebulid, false);
    ui->tableView_array->setSortableForColumn(ArrayColumnDelete, false);
    ui->tableView_array->setSortType(ArrayColumnNumber, SortFilterProxyModel::SortInt);
    ui->tableView_array->setSortType(ArrayColumnCapacity, SortFilterProxyModel::SortInt);
    //
    ui->tableView_array->hideColumn(ArrayColumnCheck);
}

void PageRaidSetting::refreshDiskTable()
{
    QMap<int, BaseDiskInfo> diskInfoMap = gMsDisk.diskInfoMap();
    //
    ui->tableView_disk->clearContent();
    ui->tableView_array->clearContent();
    int diskRow = 0;
    int arrayRow = 0;

    QMap<int, int> hotspareMap;
    for (auto iter = diskInfoMap.constBegin(); iter != diskInfoMap.constEnd(); ++iter) {
        const BaseDiskInfo &disk = iter.value();
        if (disk.hotSpareStatus() == HotSpareOn) {
            hotspareMap.insert(disk.port(), 0);
        }
    }
    QString hotspareString;
    for (auto iter = hotspareMap.constBegin(); iter != hotspareMap.constEnd(); ++iter) {
        hotspareString.append(QString::number(iter.key()));
        hotspareString.append(",");
    }
    if (hotspareString.isEmpty()) {
        hotspareString = "-";
    } else {
        hotspareString.chop(1);
    }

    for (auto iter = diskInfoMap.constBegin(); iter != diskInfoMap.constEnd(); ++iter) {
        const BaseDiskInfo &disk = iter.value();
        if (disk.isShowInRaidWidget()) {
            ui->tableView_disk->setItemChecked(diskRow, false);
            ui->tableView_disk->setItemIntValue(diskRow, DiskColumnPort, disk.port());
            ui->tableView_disk->setItemData(diskRow, DiskColumnPort, QVariant::fromValue(disk), DiskInfoRole);
            ui->tableView_disk->setItemText(diskRow, DiskColumnVendor, disk.vendorString());
            ui->tableView_disk->setItemText(diskRow, DiskColumnStatus, disk.statusString());
            ui->tableView_disk->setItemDiskBytesValue(diskRow, DiskColumnCapacity, disk.totalBytes());
            ui->tableView_disk->setItemText(diskRow, DiskColumnType, disk.typeString());

            const HotSpareStatus &hotSpare = disk.hotSpareStatus();
            switch (hotSpare) {
            case HotSpareDisable:
                ui->tableView_disk->setItemText(diskRow, DiskColumnHotSpare, "-");
                break;
            case HotSpareOn:
                ui->tableView_disk->setItemData(diskRow, DiskColumnHotSpare, true, ItemCheckedRole);
                break;
            case HotSpareOff:
                ui->tableView_disk->setItemData(diskRow, DiskColumnHotSpare, false, ItemCheckedRole);
                break;
            }
            ui->tableView_disk->setItemEnable(diskRow, DiskColumnCheck, disk.isCheckableInRaidWidget());
            diskRow++;
        }
        if (disk.typeValue() == DISK_TYPE_RAID) {
            ui->tableView_array->setItemIntValue(arrayRow, ArrayColumnNumber, disk.port());
            ui->tableView_array->setItemData(arrayRow, ArrayColumnNumber, QVariant::fromValue(disk), DiskInfoRole);
            ui->tableView_array->setItemText(arrayRow, ArrayColumnName, disk.vendorString());
            ui->tableView_array->setItemText(arrayRow, ArrayColumnDisk, disk.raidListString());
            ui->tableView_array->setItemDiskBytesValue(arrayRow, ArrayColumnCapacity, disk.totalBytes());
            ui->tableView_array->setItemText(arrayRow, ArrayColumnStatus, disk.raidStatusString());
            ui->tableView_array->setItemText(arrayRow, ArrayColumnLevel, disk.raidLevelString());
            ui->tableView_array->setItemText(arrayRow, ArrayColumnHotSpare, hotspareString);
            if (disk.rebulidProgress() >= 0) {
                ui->tableView_array->setItemText(arrayRow, ArrayColumnTask, QString("Rebulid: %1%").arg(disk.rebulidProgress()));
            }
            if (disk.raidStatusValue() != RAID_STATE_DEGRADE) {
                ui->tableView_array->setItemText(arrayRow, ArrayColumnRebulid, "-");
            }
            if (disk.raidStatusValue() == RAID_STATE_RECOVERY) {
                int port = disk.port();
                Q_UNUSED(port)
            }
            arrayRow++;
        }
    }
}

void PageRaidSetting::onLanguageChanged()
{
    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->label_raidMode->setText(GET_TEXT("DISKMANAGE/72067", "RAID Mode"));
    ui->groupBox_disk->setTitle(GET_TEXT("RAID/93001", "Physical Disk"));
    ui->groupBox_array->setTitle(GET_TEXT("RAID/93003", "Array"));

    ui->pushButton_quickCreator->setText(GET_TEXT("RAID/93002", "Quick Create"));
    ui->pushButton_creator->setText(GET_TEXT("DISKMANAGE/72074", "Create"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PageRaidSetting::onTableDiskClicked(int row, int column)
{
    switch (column) {
    case DiskColumnPort:
    case DiskColumnVendor:
    case DiskColumnStatus:
    case DiskColumnCapacity:
    case DiskColumnType: {
        if (ui->tableView_disk->isRowEnable(row)) {
            bool checked = ui->tableView_disk->isItemChecked(row);
            ui->tableView_disk->setItemChecked(row, !checked);
        }
        break;
    }
    case DiskColumnHotSpare: {
        if (!ui->tableView_disk->itemText(row, DiskColumnHotSpare).isEmpty()) {
            break;
        }
        bool checked = ui->tableView_disk->itemData(row, column, ItemCheckedRole).toBool();
        checked = !checked;

        const BaseDiskInfo &info = ui->tableView_disk->itemData(row, DiskColumnPort, DiskInfoRole).value<BaseDiskInfo>();
        int port = info.port();
        Q_UNUSED(port)

        ui->tableView_disk->setItemData(row, column, checked, ItemCheckedRole);

        break;
    }
    default:
        break;
    }
}

void PageRaidSetting::onTableArrayClicked(int row, int column)
{
    switch (column) {
    case ArrayColumnHotSpare: {
        if (!ui->tableView_array->itemText(row, ArrayColumnHotSpare).isEmpty()) {
            break;
        }
        break;
    }
    case ArrayColumnRebulid: {
        if (!ui->tableView_array->itemText(row, ArrayColumnRebulid).isEmpty()) {
            break;
        }
        const BaseDiskInfo &info = ui->tableView_array->itemData(row, ArrayColumnNumber, DiskInfoRole).value<BaseDiskInfo>();
        if (!m_rebulidRaid) {
            m_rebulidRaid = new RebulidRaid(this);
        }
        QList<int> m_checkedPortList;
        QMap<int, BaseDiskInfo> m_diskMap;
        for (int i = 0; i < ui->tableView_disk->rowCount(); ++i) {
            const BaseDiskInfo &info = ui->tableView_disk->itemData(i, DiskColumnPort, DiskInfoRole).value<BaseDiskInfo>();
            m_diskMap.insert(info.port(), info);

            if (ui->tableView_disk->isItemChecked(i)) {
                m_checkedPortList.append(info.port());
            }
        }
        m_rebulidRaid->setDiskInfoMap(m_diskMap);
        int result = m_rebulidRaid->execRaidRebulid(info);
        if (result == CreateRaid::Accepted) {
            ;
        }
        break;
    }
    case ArrayColumnDelete: {
        if (!ui->tableView_array->itemText(row, ArrayColumnDelete).isEmpty()) {
            break;
        }
        int result = MessageBox::question(this, GET_TEXT("DISKMANAGE/72019", "HDD data will be erased, continue to remove?"));
        if (result == MessageBox::Cancel) {
            break;
        }
        const BaseDiskInfo &info = ui->tableView_array->itemData(row, ArrayColumnNumber, DiskInfoRole).value<BaseDiskInfo>();
        int port = info.port();
        gMsDisk.removeRaid(port);
        //showWait();
        break;
    }
    default:
        break;
    }
}

void PageRaidSetting::on_pushButton_quickCreator_clicked()
{
    struct raid_op_t raidinfo;
    memset(&raidinfo, 0, sizeof(struct raid_op_t));
    raidinfo.raid_port = 26;
    raidinfo.raid_level = RAID_5;

    quint64 bytes = 0;
    int checkedCount = 0;
    QMap<int, int> portMap;
    for (int i = 0; i < ui->tableView_disk->rowCount(); ++i) {
        const BaseDiskInfo &info = ui->tableView_disk->itemData(i, DiskColumnPort, DiskInfoRole).value<BaseDiskInfo>();

        if (!ui->tableView_disk->isItemChecked(i)) {
            continue;
        }

        switch (info.statusValue()) {
        case DISK_STATE_UNFORMATTED:
        case DISK_STATE_NORMAL:
            break;
        default:
            continue;
        }

        portMap.insert(info.port(), 0);

        raidinfo.disk_port[raidinfo.disk_num] = info.port();
        raidinfo.disk_num++;

        if (bytes > info.totalBytes() || bytes == 0) {
            bytes = info.totalBytes();
        }
        checkedCount++;
    }

    if (checkedCount < 3) {
        ShowMessageBox(GET_TEXT("RAID/93010", "At least 3 ready physical disks are required."));
        return;
    }

    bytes = bytes + bytes * (checkedCount - 2);
    //RAID0/5/6/10创建的总容量如果超过16TB则会出现无法录像的情况
    //NOTE 2021-09-14 已经支持16TB
#if 0
    if ((bytes >> 20) > quint64(16 << 20)) {
        ShowMessageBox(GET_TEXT("DISKMANAGE/72114", "Total RAID capacity must not exceed 16TB! "));
        return;
    }
#endif

    if (MessageBox::question(this, GET_TEXT("DISKMANAGE/72046", "HDD data will be erased, continue to create?")) == MessageBox::Cancel) {
        return;
    }

    showWait(this);
    sendMessage(REQUEST_FLAG_CREATE_RAID, &raidinfo, sizeof(struct raid_op_t));
    int result = gEventLoopExec();
    //closeWait();
    if (result == 0) {

    } else if (result < 0) {
        MessageBox::information(this, GET_TEXT("RAID/93017", "Failed to create RAID because some physical disks error."));
    } else {
        for (auto iter = portMap.begin(); iter != portMap.end();) {
            int port = iter.key();
            if (((1 << port) & result)) {
                iter = portMap.erase(iter);
            } else {
                ++iter;
            }
        }
        if (portMap.size() < 3) {
            MessageBox::information(this, GET_TEXT("RAID/93017", "Failed to create RAID because some physical disks error."));
        } else {
            //格式化失败后，去掉失败的盘再试一次
            if (MessageBox::question(this, GET_TEXT("DISK/150000", "Some physical disks error, continue？")) == MessageBox::Cancel) {
                gMsDisk.getDiskInfo();
                return;
            }
            memset(raidinfo.disk_port, 0, sizeof(raidinfo.disk_port));
            raidinfo.disk_num = 0;
            for (auto iter = portMap.begin(); iter != portMap.end(); iter++) {
                int port = iter.key();
                raidinfo.disk_port[raidinfo.disk_num] = port;
                raidinfo.disk_num++;
            }
            showWait(this);
            sendMessage(REQUEST_FLAG_CREATE_RAID, &raidinfo, sizeof(struct raid_op_t));
            result = gEventLoopExec();
            //closeWait();
            if (result != 0) {
                MessageBox::information(this, GET_TEXT("RAID/93017", "Failed to create RAID because some physical disks error."));
            }
        }
    }
    gMsDisk.getDiskInfo();
}

void PageRaidSetting::on_pushButton_creator_clicked()
{
    if (!m_createRaid) {
        m_createRaid = new CreateRaid(this);
    }
    QList<int> m_checkedPortList;
    QMap<int, BaseDiskInfo> m_diskMap;
    for (int i = 0; i < ui->tableView_disk->rowCount(); ++i) {
        const BaseDiskInfo &info = ui->tableView_disk->itemData(i, DiskColumnPort, DiskInfoRole).value<BaseDiskInfo>();
        m_diskMap.insert(info.port(), info);

        if (ui->tableView_disk->isItemChecked(i)) {
            m_checkedPortList.append(info.port());
        }
    }
    m_createRaid->setDiskInfoMap(m_diskMap, m_checkedPortList);
    int result = m_createRaid->exec();
    if (result == CreateRaid::Accepted) {
        gMsDisk.getDiskInfo();
    }
}

void PageRaidSetting::on_pushButton_apply_clicked()
{
    if (!ui->checkBox_enable->isChecked() && m_isRaidEnable) {
        int result = MessageBox::question(this, GET_TEXT("DISKMANAGE/72087", "The device requires to reboot when disabling RAID mode, and the data in RAID disk will be cleared, continue?"));
        if (result == MessageBox::Yes) {
            gMsDisk.removeAllRaid();
        }
    } else if (ui->checkBox_enable->isChecked() && !m_isRaidEnable) {
        sendMessage(REQUEST_FLAG_GET_NETWORK_BANDWIDTH, (void *)NULL, 0);
        //showWait();
    }
}

void PageRaidSetting::on_pushButton_back_clicked()
{
    emit sig_back();
}
