#include "PageGroupStatus.h"
#include "ui_PageGroupStatus.h"
#include "centralmessage.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include <QtDebug>

PageGroupStatus::PageGroupStatus(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::GroupStatus)
{
    ui->setupUi(this);

    ui->tabBar->addTab(GET_TEXT("GROUPSTATUS/65001", "Sort by Group"));
    ui->tabBar->addTab(GET_TEXT("GROUPSTATUS/65002", "Sort by Channel"));
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));

    //initialize group table
    QStringList headerList;
    headerList << GET_TEXT("DISK/92004", "Group");
    headerList << GET_TEXT("DISK/92000", "Disk");
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    ui->tableView_group->setHorizontalHeaderLabels(headerList);
    ui->tableView_group->setColumnCount(headerList.size());
    ui->tableView_group->setSortingEnabled(false);
    ui->tableView_group->setHeaderCheckable(false);
    //column width
    ui->tableView_group->setColumnWidth(GroupSortColumnGroup, 100);
    ui->tableView_group->setColumnWidth(GroupSortColumnDisk, 300);

    //initialize channel table
    headerList.clear();
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("DISK/92004", "Group");
    ui->tableView_channel->setHorizontalHeaderLabels(headerList);
    ui->tableView_channel->setColumnCount(headerList.size());
    ui->tableView_channel->setSortingEnabled(false);
    ui->tableView_channel->setHeaderCheckable(false);
    //column width
    ui->tableView_channel->setColumnWidth(ChannelSortColumnChannel, 100);

    onLanguageChanged();
}

PageGroupStatus::~PageGroupStatus()
{
    delete ui;
}

void PageGroupStatus::initializeData()
{
    //Group
    m_is_group_enable = (get_param_int(SQLITE_FILE_NAME, PARAM_DISK_MODE, 0) == 1);
    //eSATA Mode
    m_is_sata_storage = (get_param_int(SQLITE_FILE_NAME, PARAM_ESATA_TYPE, 0) == 0);

    //
    m_groupMap.clear();
    groupInfo group_array[MAX_MSDK_GROUP_NUM];
    memset(&group_array, 0, sizeof(groupInfo) * MAX_MSDK_GROUP_NUM);
    int groupCount = 0;
    read_groups(SQLITE_FILE_NAME, group_array, &groupCount);
    for (int i = 0; i < groupCount; ++i) {
        const groupInfo &group = group_array[i];
        m_groupMap.insert(group.groupid, group);
    }

    ui->tabBar->setCurrentTab(0);
    sortByGroup();
    sortByChannel();

    if (m_is_group_enable) {
        sendMessage(REQUEST_FLAG_GET_MSFS_DISKINFO, nullptr, 0);
        //showWait();
    }
}

bool PageGroupStatus::canAutoLogout()
{
    return false;
}

void PageGroupStatus::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_MSFS_DISKINFO:
        ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(message);
        break;
    }
}

void PageGroupStatus::ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message)
{
    //closeWait();

    struct resp_get_msfs_diskinfo *diskinfo_array = (struct resp_get_msfs_diskinfo *)message->data;
    int diskCount = message->header.size / sizeof(struct resp_get_msfs_diskinfo);

    QMap<int, QString> groupStringMap;
    for (int i = 0; i < diskCount; ++i) {
        const resp_get_msfs_diskinfo &disk = diskinfo_array[i];
        if (disk.disk_type == DISK_TYPE_GLOBAL_SPARE || disk.disk_type == DISK_TYPE_IN_RAID || disk.disk_type == DISK_TYPE_USB || (disk.disk_type == DISK_TYPE_ESATA && !m_is_sata_storage)) {
            continue;
        }
        int group = disk.disk_group;
        if (group >= 0 && group < MAX_MSDK_GROUP_NUM) {
            QString &groupString = groupStringMap[group];
            if (m_is_group_enable) {
                groupString.append(QString("%1,").arg(disk.disk_port));
            }
        }
    }
    for (auto iter = groupStringMap.constBegin(); iter != groupStringMap.constEnd(); ++iter) {
        int row = iter.key();
        QString text = iter.value();
        if (text.isEmpty()) {
            text = "-";
        } else {
            text.chop(1);
        }
        ui->tableView_group->setItemText(row, GroupSortColumnDisk, text);
    }
}

void PageGroupStatus::onLanguageChanged()
{
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PageGroupStatus::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void PageGroupStatus::sortByGroup()
{
    ui->tableView_group->setRowCount(m_groupMap.size());
    int row = 0;
    for (auto iter = m_groupMap.constBegin(); iter != m_groupMap.constEnd(); ++iter) {
        const auto &group = iter.value();
        ui->tableView_group->setItemIntValue(row, GroupSortColumnGroup, group.groupid + 1);
        ui->tableView_group->setItemText(row, GroupSortColumnDisk, "-");

        QString channelString;
        if (m_is_group_enable) {
            QString channelMask(group.chnMaskl);
            for (int i = 0; i < channelMask.size(); ++i) {
                if (channelMask.at(i) == QChar('1')) {
                    channelString.append(QString("%1,").arg(i + 1));
                }
            }
            if (!channelString.isEmpty()) {
                channelString.chop(1);
            }
        }
        if (channelString.isEmpty()) {
            channelString = "-";
        }

        ui->tableView_group->setItemText(row, GroupSortColumnChannel, channelString);
        row++;
    }
}

void PageGroupStatus::sortByChannel()
{
    int channelCount = qMsNvr->maxChannel();
    ui->tableView_channel->setRowCount(channelCount);
    int row = 0;
    for (int i = 0; i < channelCount; ++i) {
        ui->tableView_channel->setItemIntValue(row, ChannelSortColumnChannel, i + 1);

        QString groupString;
        if (m_is_group_enable) {
            for (auto iter = m_groupMap.constBegin(); iter != m_groupMap.constEnd(); ++iter) {
                const auto &group = iter.value();
                if (group.chnMaskl[i] == '1') {
                    groupString.append(QString("%1,").arg(group.groupid + 1));
                }
            }
        }

        if (groupString.isEmpty()) {
            groupString = "-";
        } else {
            groupString.chop(1);
        }
        ui->tableView_channel->setItemText(row, ChannelSortColumnGroup, groupString);
        row++;
    }
}

void PageGroupStatus::on_pushButton_back_clicked()
{
    emit sig_back();
}
