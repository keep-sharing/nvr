#include "storagegroup.h"
#include "ui_storagegroup.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "MsDevice.h"
#include "MsDisk.h"
#include "MsLanguage.h"

StorageGroup::StorageGroup(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::StorageGroup)
{
    ui->setupUi(this);

    ui->checkBoxGroup_channel->setCount(qMsNvr->maxChannel());
    connect(ui->checkBoxGroup_channel, SIGNAL(checkBoxClicked()), this, SLOT(onChannelCheckBoxClicked()));

    ui->comboBoxEnable->beginEdit();
    ui->comboBoxEnable->clear();
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxEnable->endEdit();
    onLanguageChanged();
}

StorageGroup::~StorageGroup()
{
    delete ui;
}

void StorageGroup::initializeData()
{
    //eSATA Mode
    BaseDiskInfo::is_sata_storage = (get_param_int(SQLITE_FILE_NAME, PARAM_ESATA_TYPE, 0) == 0);
    //Group
    BaseDiskInfo::is_group_enable = (get_param_int(SQLITE_FILE_NAME, PARAM_DISK_MODE, 0) == 1);

    ui->comboBoxEnable->setCurrentIndexFromData(BaseDiskInfo::is_group_enable);

    //
    //showWait();
    sendMessage(REQUEST_FLAG_GET_MSFS_DISKINFO, nullptr, 0);

    int groupCount = 0;
    struct groupInfo group_array[MAX_MSDK_GROUP_NUM];
    memset(group_array, 0, sizeof(struct groupInfo) * MAX_MSDK_GROUP_NUM);
    read_groups(SQLITE_FILE_NAME, group_array, &groupCount);
    for (int i = 0; i < groupCount; ++i) {
        const struct groupInfo &group = group_array[i];
        m_groupMap.insert(group.groupid, group);
    }
}

void StorageGroup::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_MSFS_DISKINFO:
        ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(message);
        break;
    case RESPONSE_FLAG_SET_RECORD_UPDATE:
        ON_RESPONSE_FLAG_SET_RECORD_UPDATE(message);
        break;
    case RESPONSE_FLAG_SET_MSFS_MODE:
        ON_RESPONSE_FLAG_SET_MSFS_MODE(message);
        break;
    }
}

void StorageGroup::ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message)
{
    //closeWait();

    ui->comboBox_group->clear();
    for (int i = 0; i < MAX_MSDK_GROUP_NUM; ++i) {
        ui->comboBox_group->addItem(QString("%1").arg(i + 1), i);
    }

    struct resp_get_msfs_diskinfo *msfs_diskinfo_array = (struct resp_get_msfs_diskinfo *)message->data;
    int disk_count = message->header.size / sizeof(struct resp_get_msfs_diskinfo);
    for (int group = 0; group < MAX_MSDK_GROUP_NUM; ++group) {
        bool isGroupValid = false;
        for (int i = 0; i < disk_count; ++i) {
            const resp_get_msfs_diskinfo &msfs_diskinfo = msfs_diskinfo_array[i];
            if (msfs_diskinfo.disk_group == group && msfs_diskinfo.disk_type != DISK_TYPE_GLOBAL_SPARE && msfs_diskinfo.disk_type != DISK_TYPE_IN_RAID && msfs_diskinfo.disk_type != DISK_TYPE_USB && (msfs_diskinfo.disk_type != DISK_TYPE_ESATA || BaseDiskInfo::is_sata_storage)) {
                isGroupValid = true;
                break;
            }
        }
        if (!isGroupValid) {
            ui->comboBox_group->removeItem(ui->comboBox_group->findData(group));
        }
    }

    //
    on_comboBox_group_activated(ui->comboBox_group->currentIndex());
}

void StorageGroup::ON_RESPONSE_FLAG_SET_RECORD_UPDATE(MessageReceive *message)
{
    Q_UNUSED(message)

    //closeWait();
}

void StorageGroup::ON_RESPONSE_FLAG_SET_MSFS_MODE(MessageReceive *message)
{
    Q_UNUSED(message)

    //closeWait();
}

void StorageGroup::onLanguageChanged()
{
    ui->label_groupMode->setText(GET_TEXT("GROUP/94000", "Group Mode"));
    ui->label_note->setText(GET_TEXT("GROUP/94001", "Note: You can set disk into other groups at Disk interface after Group enabled."));
    ui->label_group->setText(GET_TEXT("DISK/92004", "Group"));
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void StorageGroup::onChannelCheckBoxClicked()
{
    int groupID = ui->comboBox_group->currentData().toInt();
    if (m_groupMap.contains(groupID)) {
        struct groupInfo &group = m_groupMap[groupID];
        const QString &checkedMask = ui->checkBoxGroup_channel->checkedMask();
        snprintf(group.chnMaskl, sizeof(group.chnMaskl), "%s", checkedMask.toStdString().c_str());
    }
}

void StorageGroup::on_comboBoxEnable_indexSet(int index)
{
    if (ui->comboBoxEnable->itemData(index).toInt() == 0) {
        ui->comboBox_group->setEnabled(false);
        ui->checkBoxGroup_channel->setEnabled(false);
    } else {
        ui->comboBox_group->setEnabled(true);
        ui->checkBoxGroup_channel->setEnabled(true);
    }
}

void StorageGroup::on_comboBox_group_activated(int index)
{
    if (index < 0) {
        ui->checkBoxGroup_channel->clearCheck();
        return;
    }

    int groupID = ui->comboBox_group->itemData(index).toInt();
    qMsDebug() << "index:" << index << ", group id:" << groupID;
    if (m_groupMap.contains(groupID)) {
        const struct groupInfo &group = m_groupMap.value(groupID);
        qDebug() << QString("Group, id: %1, mask: %2").arg(groupID).arg(group.chnMaskl);
        ui->checkBoxGroup_channel->setCheckedFromString(group.chnMaskl);
    } else {
        ui->checkBoxGroup_channel->clearCheck();
    }
}

void StorageGroup::on_pushButton_apply_clicked()
{
    if (ui->comboBoxEnable->currentData().toInt() == 1) {
        if (!BaseDiskInfo::is_group_enable) {
            int disk_mode = 1;
            set_param_int(SQLITE_FILE_NAME, PARAM_DISK_MODE, disk_mode);
            sendMessage(REQUEST_FLAG_SET_MSFS_MODE, (void *)&(disk_mode), sizeof(int));
        }
        struct req_group_info group_info;
        for (auto iter = m_groupMap.constBegin(); iter != m_groupMap.constEnd(); ++iter) {
            struct groupInfo group = iter.value();
            write_group_st(SQLITE_FILE_NAME, &group);

            group_info.id = group.groupid;
            snprintf(group_info.chnMaskl, sizeof(group_info.chnMaskl), "%s", group.chnMaskl);
            snprintf(group_info.chnMaskh, sizeof(group_info.chnMaskh), "%s", group.chnMaskh);
            sendMessage(REQUEST_FLAG_SET_MSFS_GROUP, (void *)&group_info, sizeof(struct req_group_info));
        }
        sendMessage(REQUEST_FLAG_SET_RECORD_UPDATE, NULL, 0);
        //showWait();
    } else {
        int disk_mode = 0;
        set_param_int(SQLITE_FILE_NAME, PARAM_DISK_MODE, disk_mode);
        sendMessage(REQUEST_FLAG_SET_MSFS_MODE, (void *)&(disk_mode), sizeof(int));
        //showWait();
    }
}

void StorageGroup::on_pushButton_back_clicked()
{
    emit sig_back();
}
