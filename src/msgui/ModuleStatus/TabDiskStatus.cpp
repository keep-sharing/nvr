#include "TabDiskStatus.h"
#include "ui_TabDiskStatus.h"
#include "MsDisk.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "centralmessage.h"
#include "MessageFilter.h"

TabDiskStatus::TabDiskStatus(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::PageDiskStatus)
{
    ui->setupUi(this);

    m_waitting = new MsWaitting(this);

    QStringList headerList;
    headerList << GET_TEXT("CHANNELMANAGE/30011", "Port");
    headerList << GET_TEXT("CHANNELMANAGE/30028", "Vendor");
    headerList << GET_TEXT("MENU/10006", "Status");
    headerList << GET_TEXT("DISK/92003", "Total");
    headerList << GET_TEXT("DISK/92032", "Free");
    headerList << GET_TEXT("DISK/92016", "Property");
    headerList << GET_TEXT("COMMON/1052", "Type");
    headerList << GET_TEXT("DISK/92004", "Group");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setSortType(ColumnPort, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnTotal, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnFree, SortFilterProxyModel::SortInt);

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_GET_MSFS_DISKINFO, this);

    onLanguageChanged();
}

TabDiskStatus::~TabDiskStatus()
{
    delete ui;
}

void TabDiskStatus::initializeData()
{
    ui->tableView->clearSort();
    refreshData();
}

QList<int> TabDiskStatus::localDiskList()
{
    return m_localDiskPortList;
}

void TabDiskStatus::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabDiskStatus::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_MSFS_DISKINFO:
        ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(message);
        break;
    }
}

void TabDiskStatus::refreshData()
{
    gMsDisk.getDiskInfo();
    //m_waitting->//showWait();
}

void TabDiskStatus::resizeEvent(QResizeEvent *event)
{
    int tableWidth = ui->tableView->width() - 50;

    ui->tableView->setColumnWidth(0, 100);
    ui->tableView->setColumnWidth(1, 300);
    int columnWidth = (tableWidth - 400) / 6;
    for (int i = 2; i < ui->tableView->columnCount(); ++i) {
        ui->tableView->setColumnWidth(i, columnWidth);
    }
    QWidget::resizeEvent(event);
}

void TabDiskStatus::onLanguageChanged()
{
    ui->label_total->setText(GET_TEXT("DISK/92001", "Total Capacity"));
    ui->label_available->setText(GET_TEXT("DISKMANAGE/72028", "Available Capacity"));
}

void TabDiskStatus::ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message)
{
    Q_UNUSED(message)

    if (!isVisible()) {
        return;
    }

    m_localDiskPortList.clear();
    //
    QMap<int, BaseDiskInfo> diskInfoMap = gMsDisk.diskInfoMap();
    quint64 totalBytes = 0;
    quint64 freeBytes = 0;
    int row = 0;
    for (auto iter = diskInfoMap.constBegin(); iter != diskInfoMap.constEnd(); ++iter) {
        const BaseDiskInfo &disk = iter.value();
        if (disk.typeValue() == DISK_TYPE_LOCAL) {
            m_localDiskPortList.append(disk.port());
        }
        if (disk.isShowInDiskWidget()) {
            ui->tableView->setItemIntValue(row, ColumnPort, disk.port());
            ui->tableView->setItemText(row, ColumnVendor, disk.vendorString());
            ui->tableView->setItemText(row, ColumnStatus, disk.statusString());
            ui->tableView->setItemDiskBytesValue(row, ColumnTotal, disk.totalBytes());
            ui->tableView->setItemDiskBytesValue(row, ColumnFree, disk.freeBytes());
            ui->tableView->setItemText(row, ColumnProperty, disk.propertyString());
            ui->tableView->setItemText(row, ColumnType, disk.typeString());
            ui->tableView->setItemText(row, ColumnGroup, disk.groupString());
            ui->tableView->setItemData(row, ColumnGroup, disk.groupValue(), SortIntRole);
            totalBytes += disk.totalBytes();
            freeBytes += disk.freeBytes();
            row++;
        }
    }
    ui->tableView->reorder();
    ui->lineEdit_total->setText(BaseDiskInfo::bytesString(totalBytes));
    ui->lineEdit_available->setText(BaseDiskInfo::bytesString(freeBytes));

    //m_waitting->//closeWait();
}
