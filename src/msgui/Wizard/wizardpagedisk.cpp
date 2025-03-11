#include "wizardpagedisk.h"
#include "ui_wizardpagedisk.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDisk.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "MessageFilter.h"

const int DiskInfoRole = Qt::UserRole + 50;

WizardPageDisk::WizardPageDisk(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPageDisk)
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
    headerList << GET_TEXT("COMMON/1052", "Type");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setAlternatingRowColors(false);
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableClicked(int, int)));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortType(ColumnPort, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnTotal, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnFree, SortFilterProxyModel::SortInt);
    //column width
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnPort, 100);
    ui->tableView->setColumnWidth(ColumnVendor, 210);
    ui->tableView->setColumnWidth(ColumnStatus, 125);
    ui->tableView->setColumnWidth(ColumnTotal, 120);
    ui->tableView->setColumnWidth(ColumnFree, 120);

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_GET_MSFS_DISKINFO, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_FORMAT_MSFS_DISK, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_DISK_INIT, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_DISK_LOAD, this);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

WizardPageDisk::~WizardPageDisk()
{
    delete ui;
}

void WizardPageDisk::initializeData()
{
    gMsDisk.getDiskInfo();

    //m_waitting->//showWait();
}

void WizardPageDisk::saveSetting()
{
    return;
}

void WizardPageDisk::previousPage()
{
    showWizardPage(Wizard_NetWork);
}

void WizardPageDisk::nextPage()
{
    showWizardPage(Wizard_Camera);
}

void WizardPageDisk::skipWizard()
{
    AbstractWizardPage::skipWizard();
}

void WizardPageDisk::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void WizardPageDisk::filterMessage(MessageReceive *message)
{
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
    }
}

void WizardPageDisk::ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message)
{
    Q_UNUSED(message)
    if (!isVisible()) {
        return;
    }

    refreshTable();
    //m_waitting->//closeWait();
}

void WizardPageDisk::ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(MessageReceive *message)
{
    if (!isVisible()) {
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

void WizardPageDisk::ON_RESPONSE_FLAG_PROGRESS_DISK_INIT(MessageReceive *message)
{
    Q_UNUSED(message)

    refreshTable();
}

void WizardPageDisk::ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(MessageReceive *message)
{
    Q_UNUSED(message)

    refreshTable();
}

void WizardPageDisk::refreshTable()
{
    QMap<int, BaseDiskInfo> diskInfoMap = gMsDisk.diskInfoMap();
    //
    quint64 totalBytes = 0;
    quint64 freeBytes = 0;
    ui->tableView->clearContent();
    int row = 0;
    for (auto iter = diskInfoMap.constBegin(); iter != diskInfoMap.constEnd(); ++iter) {
        const BaseDiskInfo &disk = iter.value();
        if (disk.isShowInDiskWidget()) {
            ui->tableView->setItemChecked(row, false);
            ui->tableView->setItemIntValue(row, ColumnPort, disk.port());
            ui->tableView->setItemData(row, ColumnPort, QVariant::fromValue(disk), DiskInfoRole);
            ui->tableView->setItemText(row, ColumnVendor, disk.vendorString());
            ui->tableView->setItemText(row, ColumnStatus, disk.statusString());
            ui->tableView->setItemDiskBytesValue(row, ColumnTotal, disk.totalBytes());
            ui->tableView->setItemDiskBytesValue(row, ColumnFree, disk.freeBytes());
            ui->tableView->setItemText(row, ColumnType, disk.typeString());

            ui->tableView->setItemEnable(row, ColumnCheck, disk.isCheckable());

            totalBytes += disk.totalBytes();
            freeBytes += disk.freeBytes();
            row++;
        }
    }
    ui->lineEdit_total->setText(BaseDiskInfo::bytesString(totalBytes));
    ui->lineEdit_available->setText(BaseDiskInfo::bytesString(freeBytes));
}

void WizardPageDisk::onLanguageChanged()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30011", "Port");
    headerList << GET_TEXT("CHANNELMANAGE/30028", "Vendor");
    headerList << GET_TEXT("MENU/10006", "Status");
    headerList << GET_TEXT("DISK/92003", "Total");
    headerList << GET_TEXT("DISK/92032", "Free");
    headerList << GET_TEXT("COMMON/1052", "Type");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->label_total->setText(GET_TEXT("DISK/92001", "Total Capacity"));
    ui->label_available->setText(GET_TEXT("DISKMANAGE/72028", "Available Capacity"));
    ui->pushButton_initialize->setText(GET_TEXT("DISK/92025", "Initialize"));
}

void WizardPageDisk::onTableClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
}

void WizardPageDisk::on_pushButton_initialize_clicked()
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
