#include "storagequota.h"
#include "ui_storagequota.h"
#include "MsDevice.h"
#include "MsDisk.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
StorageQuota::StorageQuota(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::StorageQuota)
{
    ui->setupUi(this);

    ui->comboBoxEnable->clear();
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    //channel
    ui->comboBoxChannel->clear();
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBoxChannel->addItem(QString::number(i + 1), i);
    }

    //initialize table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("ACTION/56007", "Channel");
    headerList << GET_TEXT("CHANNELMANAGE/30009", "Channel Name");
    headerList << GET_TEXT("STORAGEMODE/95102", "Used Record Capacity (GB)");
    headerList << GET_TEXT("STORAGEMODE/95103", "Used Snapshot Capacity (GB)");
    headerList << GET_TEXT("STORAGEMODE/95104", "Record Quota (GB)");
    headerList << GET_TEXT("STORAGEMODE/95105", "Snapshot Quota (GB)");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->hideColumn(ColumnCheck);
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);

    ui->lineEditRecordQuota->setCheckMode(MyLineEdit::SpecialRangeCheck, 4, 16384, 0);
    ui->lineEditSnapshotQuota->setCheckMode(MyLineEdit::SpecialRangeCheck, 4, 16384, 0);
    onLanguageChanged();
}

StorageQuota::~StorageQuota()
{
    delete ui;
}

void StorageQuota::initializeData()
{
    ui->comboBoxEnable->setCurrentIndexFromData(false);

    //showWait();
    ui->tableView->clearSort();
    sendMessage(REQUEST_FLAG_GET_MSFS_QUOTA, nullptr, 0);
    //m_eventLoop.exec();
    //closeWait();

    //
    //showQuotaInfo();
}

void StorageQuota::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_MSFS_QUOTA:
        ON_RESPONSE_FLAG_GET_MSFS_QUOTA(message);
        break;
    case RESPONSE_FLAG_SET_MSFS_QUOTA:
        ON_RESPONSE_FLAG_SET_MSFS_QUOTA(message);
        break;
    }
}

void StorageQuota::resizeEvent(QResizeEvent *)
{
    resizeTableView();
}

void StorageQuota::showEvent(QShowEvent *)
{
    resizeTableView();
}

void StorageQuota::ON_RESPONSE_FLAG_GET_MSFS_QUOTA(MessageReceive *message)
{
    memset(&m_quota_info, 0, sizeof(m_quota_info));

    req_quota *quota_info = static_cast<req_quota *>(message->data);
    if (!quota_info) {
        qMsWarning() << "data is nullptr";
    } else {
        memcpy(&m_quota_info, quota_info, sizeof(m_quota_info));
        ui->comboBoxEnable->setCurrentIndexFromData(m_quota_info.enable);
    }

    m_eventLoop.exit();
}

void StorageQuota::ON_RESPONSE_FLAG_SET_MSFS_QUOTA(MessageReceive *message)
{
    Q_UNUSED(message)

    //closeWait();
}

void StorageQuota::resizeTableView()
{
    //column width
    int w = (ui->tableView->width() - 400) / 4;
    ui->tableView->setColumnWidth(ColumnChannel, 100);
    ui->tableView->setColumnWidth(ColumnChannelName, 200);
    ui->tableView->setColumnWidth(ColumnUsedRecord, w);
    ui->tableView->setColumnWidth(ColumnUsedSnapshot, w);
    ui->tableView->setColumnWidth(ColumnRecordQuota, w);
    ui->tableView->setColumnWidth(ColumnSnapshotQuota, w);
}

void StorageQuota::showQuotaInfo()
{
    const quota_info_t &info = m_quota_info.quota_conf[m_currentChannel];
    ui->lineEditUsedRecord->setText(QString("%1").arg(info.vidUsd));
    ui->lineEditUsedSnapshot->setText(QString("%1").arg(info.picUsd));
    if (info.vidQta != MF_U32(-1)) {
        ui->lineEditRecordQuota->setText(QString("%1").arg(info.vidQta));
    }
    if (info.picQta != MF_U32(-1)) {
        ui->lineEditSnapshotQuota->setText(QString("%1").arg(info.picQta));
    }

    ui->tableView->clearContent();
    ui->tableView->setRowCount(qMsNvr->maxChannel());
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        const quota_info_t &info = m_quota_info.quota_conf[i];
        ui->tableView->setItemIntValue(i, ColumnChannel, i + 1);
        ui->tableView->setItemText(i, ColumnChannelName, qMsNvr->channelName(i));
        ui->tableView->setItemToolTip(i, ColumnChannelName, ui->tableView->itemText(i, ColumnChannelName));
        ui->tableView->setItemIntValue(i, ColumnUsedRecord, info.vidUsd);
        ui->tableView->setItemIntValue(i, ColumnUsedSnapshot, info.picUsd);
        if (info.vidQta != MF_U32(-1)) {
            ui->tableView->setItemIntValue(i, ColumnRecordQuota, info.vidQta);
        } else {
            ui->tableView->setItemText(i, ColumnRecordQuota, "");
        }
        if (info.picQta != MF_U32(-1)) {
            ui->tableView->setItemIntValue(i, ColumnSnapshotQuota, info.picQta);
        } else {
            ui->tableView->setItemText(i, ColumnSnapshotQuota, "");
        }
    }
}

void StorageQuota::setQuotaEnable(bool enable)
{
    ui->comboBoxChannel->setEnabled(enable);
    ui->lineEditRecordQuota->setEnabled(enable);
    ui->lineEditSnapshotQuota->setEnabled(enable);
}

void StorageQuota::onLanguageChanged()
{
    ui->labelQuota->setText(GET_TEXT("STORAGEMODE/95101", "Quota"));
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelUsedRecord->setText(GET_TEXT("STORAGEMODE/95102", "Used Record Capacity (GB)"));
    ui->labelUsedSnapshot->setText(GET_TEXT("STORAGEMODE/95103", "Used Snapshot Capacity (GB)"));
    ui->labelRecordQuota->setText(GET_TEXT("STORAGEMODE/95104", "Record Quota (GB)"));
    ui->labelSnapshotQuota->setText(GET_TEXT("STORAGEMODE/95105", "Snapshot Quota (GB)"));
    ui->labelNote2->setText(GET_TEXT("STORAGEMODE/95109", "Note: 0GB means no quota, whose priority is lower than the one has quota."));

    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void StorageQuota::on_comboBoxEnable_indexSet(int index)
{
    int value = ui->comboBoxEnable->itemData(index).toInt();
    setQuotaEnable(value);
}

void StorageQuota::on_comboBoxChannel_activated(int index)
{
    bool valid = ui->lineEditRecordQuota->checkValid();
    valid &= ui->lineEditSnapshotQuota->checkValid();
    if (valid) {
        m_currentChannel = index;
        showQuotaInfo();
    } else {
        ui->comboBoxChannel->setCurrentIndexFromData(m_currentChannel);
    }
}

void StorageQuota::on_lineEditRecordQuota_editingFinished()
{
    quota_info_t &info = m_quota_info.quota_conf[m_currentChannel];
    if (ui->lineEditRecordQuota->text().isEmpty()) {
        info.vidQta = -1;
    } else {
        info.vidQta = ui->lineEditRecordQuota->text().toInt();
    }

    showQuotaInfo();
}

void StorageQuota::on_lineEditSnapshotQuota_editingFinished()
{
    quota_info_t &info = m_quota_info.quota_conf[m_currentChannel];
    if (ui->lineEditSnapshotQuota->text().isEmpty()) {
        info.picQta = -1;
    } else {
        info.picQta = ui->lineEditSnapshotQuota->text().toInt();
    }

    showQuotaInfo();
}

void StorageQuota::on_pushButtonCopy_clicked()
{
    if (ui->comboBoxEnable->currentIntData() == 1) {
        bool valid = ui->lineEditRecordQuota->checkValid();
        valid &= ui->lineEditSnapshotQuota->checkValid();
        if (!valid) {
            return;
        }
    }
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        const quota_info_t &info = m_quota_info.quota_conf[m_currentChannel];
        QList<int> copyList = copy.checkedList(false);
        for (int i = 0; i < copyList.size(); ++i) {
            int channel = copyList.at(i);
            quota_info_t &temp_info = m_quota_info.quota_conf[channel];
            temp_info.picQta = info.picQta;
            temp_info.vidQta = info.vidQta;
        }
        showQuotaInfo();
        on_pushButtonApply_clicked();
    }
}

void StorageQuota::on_pushButtonApply_clicked()
{
    if (ui->comboBoxEnable->currentIntData() == 1) {
        bool valid = ui->lineEditRecordQuota->checkValid();
        valid = ui->lineEditSnapshotQuota->checkValid() && valid;
        if (!valid) {
            return;
        }
    }
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        quota_info_t &info = m_quota_info.quota_conf[i];
        if (info.vidQta == MF_U32(-1)) {
            info.vidQta = 0;
        }
        if (info.picQta == MF_U32(-1)) {
            info.picQta = 0;
        }
    }
    m_quota_info.enable = ui->comboBoxEnable->currentData().toInt();

    //
    sendMessage(REQUEST_FLAG_SET_MSFS_QUOTA, &m_quota_info, sizeof(m_quota_info));
    //
    //showWait();
}

void StorageQuota::on_pushButtonBack_clicked()
{
    back();
}
