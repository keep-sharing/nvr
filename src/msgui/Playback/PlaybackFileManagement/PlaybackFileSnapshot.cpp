#include "PlaybackFileSnapshot.h"
#include "ui_AbstractPlaybackFile.h"
#include "AnimateToast.h"
#include "BasePlayback.h"
#include "DownloadPanel.h"
#include "LogWrite.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"
#include "PlaybackSplit.h"
#include "SubControl.h"
#include "centralmessage.h"
#include "msuser.h"
#include <QtDebug>
#include <qmath.h>

extern "C" {

}

PlaybackFileSnapshot::PlaybackFileSnapshot(QWidget *parent)
    : AbstractPlaybackFile(parent)
{
    initializeTableView();
}

void PlaybackFileSnapshot::updateTableList()
{
    AbstractPlaybackFile::updateTableList();

    ui->tableView->clearContent();
    int row = 0;
    auto iter = m_fileMap.constBegin();
    for (iter += m_pageIndex * 100; iter != m_fileMap.constEnd(); ++iter) {
        const auto &key = iter.key();
        const auto &value = iter.value();
        ui->tableView->setItemIntValue(row, ColumnChannel, value.chnid + 1);
        ui->tableView->setItemData(row, ColumnChannel, QVariant::fromValue(value), FileInfoRole);
        ui->tableView->setItemIntValue(row, ColumnDisk, value.port);
        ui->tableView->setItemText(row, ColumnTime, key.startDateTimeString());
        ui->tableView->setItemBytesValue(row, ColumnSize, value.size);

        row++;
        if (row >= 100) {
            break;
        }
    }

    dealSelected();
}

void PlaybackFileSnapshot::clear()
{
    m_fileMap.clear();
    ui->tableView->clearContent();

    dealSelected();
}

int PlaybackFileSnapshot::waitForSnapshot(int winid)
{
    m_alreadyExport = false;
    m_currentCount = 1;
    m_currentIndex = 0;
    m_snapshotSucceedCount = 0;

    int sid = WIN_SID(winid);
    if (BasePlayback::playbackType() == SplitPlayback) {
        req_split_snaphost split_snaphost;
        memset(&split_snaphost, 0, sizeof(split_snaphost));
        split_snaphost.chnid = PlaybackSplit::instance()->channel();
        split_snaphost.screen = SubControl::instance()->currentScreen();
        for (int i = 0; i < MAX_LEN_16; ++i) {
            if (i == sid) {
                split_snaphost.sid[i] = sid;
            } else {
                split_snaphost.sid[i] = -1;
            }
        }
        split_snaphost.uSecTime[sid] = BasePlayback::playbackDateTime().toMSecsSinceEpoch() * 1000;

        qDebug() << "====PlaybackFileSnapshot::waitForSnapshot====";
        qDebug() << "----time:" << split_snaphost.uSecTime[sid];
        qDebug() << "----screen:" << split_snaphost.screen;
        qDebug() << "----sid:" << split_snaphost.sid[sid];
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SNAPHOST_SPLIT_PLAYBACK, (void *)&split_snaphost, sizeof(split_snaphost));
    } else {
        int ch = BasePlayback::currentChannel();
        req_snapshot_playback_time snapshot_playback_time;
        memset(&snapshot_playback_time, 0, sizeof(struct req_snapshot_playback_time));
        snapshot_playback_time.screen = SubControl::instance()->currentScreen();
        for (int i = 0; i < MAX_LEN_64; ++i) {
            if (i == ch) {
                snapshot_playback_time.winSidChn[i] = sid;
            } else {
                snapshot_playback_time.winSidChn[i] = -1;
            }
        }

        snprintf(snapshot_playback_time.ptime, sizeof(snapshot_playback_time.ptime), "%s", BasePlayback::playbackDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        qDebug() << "====PlaybackFileSnapshot::waitForSnapshot====";
        qDebug() << "----time:" << snapshot_playback_time.ptime;
        qDebug() << "----screen:" << snapshot_playback_time.screen;
        qDebug() << "----winSidChn:" << snapshot_playback_time.winSidChn[ch];
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_PLAYBACK_SNAPSHOT, (void *)&snapshot_playback_time, sizeof(req_snapshot_playback_time));
    }

    // QTimer timer;
    // timer.setSingleShot(true);
    // connect(&timer, SIGNAL(timeout()), &m_eventLoop, SLOT(quit()));
    // timer.start(30 * 1000);
    // m_eventLoop.exec();

    // if (!timer.isActive()) {
    //     return -2;
    // } else {
    //     return m_snapshotSucceedCount;
    // }
    return 0;
}

bool PlaybackFileSnapshot::hasNotExportedFile() const
{
    if (m_fileMap.isEmpty()) {
        return false;
    }
    if (m_alreadyExport) {
        return false;
    }
    return true;
}

void PlaybackFileSnapshot::exportSnapshot()
{
    //
    bool hasSelected = false;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (ui->tableView->isItemChecked(row)) {
            hasSelected = true;
            break;
        }
    }
    if (!hasSelected) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        return;
    }

    //
    const QString &directory = MyFileSystemDialog::instance()->exportPicture();
    if (directory.isEmpty()) {
        return;
    }
    const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int pictureResolution = MyFileSystemDialog::instance()->pictureResolution();

    //
    QList<resp_snapshot_state> exportList;
    qint64 totalBytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (!ui->tableView->isItemChecked(row)) {
            continue;
        }
        const resp_snapshot_state &snapshot_state = ui->tableView->itemData(row, ColumnChannel, FileInfoRole).value<resp_snapshot_state>();
        totalBytes += snapshot_state.size;

        exportList.append(snapshot_state);
    }

    if (totalBytes > deviceFreeSize) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

    for (int i = 0; i < exportList.size(); ++i) {
        const resp_snapshot_state &exportBackup = exportList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid = gDownload->availableId();
        auto_backup.chnid = exportBackup.chnid;
        auto_backup.snap_res = pictureResolution;
        auto_backup.filetype = 3;
        auto_backup.filesize = 0;
        snprintf(auto_backup.pStartTime, sizeof(auto_backup.pStartTime), "%s", QDateTime::fromTime_t(exportBackup.starTime).toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        snprintf(auto_backup.dev_name, sizeof(auto_backup.dev_name), "%s", deviceName.toStdString().c_str());
        snprintf(auto_backup.dev_path, sizeof(auto_backup.dev_path), "%s", directory.toStdString().c_str());

        char fileName[128];
        //get_auto_backup_snaphots_filename(&auto_backup, fileName, sizeof(fileName));
        snprintf(auto_backup.filename, sizeof(auto_backup.filename), "%s", fileName);

        gDownload->appendItem(auto_backup);
    }
    m_alreadyExport = true;
    ShowMessageBox(GET_TEXT("PLAYBACK/80135", "Please check download status in download progress interface. "));
}

void PlaybackFileSnapshot::exportAllSnapshot()
{
    if (m_fileMap.isEmpty()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        return;
    }
    //
    const QString &directory = MyFileSystemDialog::instance()->exportPicture();
    if (directory.isEmpty()) {
        return;
    }
    const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int pictureResolution = MyFileSystemDialog::instance()->pictureResolution();

    //
    QList<resp_snapshot_state> exportList;
    qint64 totalBytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        const resp_snapshot_state &snapshot_state = ui->tableView->itemData(row, ColumnChannel, FileInfoRole).value<resp_snapshot_state>();
        totalBytes += snapshot_state.size;

        exportList.append(snapshot_state);
    }

    if (totalBytes > deviceFreeSize) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

    for (int i = 0; i < exportList.size(); ++i) {
        const resp_snapshot_state &exportBackup = exportList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid = gDownload->availableId();
        auto_backup.chnid = exportBackup.chnid;
        auto_backup.snap_res = pictureResolution;
        auto_backup.filetype = 3;
        auto_backup.filesize = 0;
        snprintf(auto_backup.pStartTime, sizeof(auto_backup.pStartTime), "%s", QDateTime::fromTime_t(exportBackup.starTime).toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        snprintf(auto_backup.dev_name, sizeof(auto_backup.dev_name), "%s", deviceName.toStdString().c_str());
        snprintf(auto_backup.dev_path, sizeof(auto_backup.dev_path), "%s", directory.toStdString().c_str());

        char fileName[128];
        //get_auto_backup_snaphots_filename(&auto_backup, fileName, sizeof(fileName));
        snprintf(auto_backup.filename, sizeof(auto_backup.filename), "%s", fileName);

        gDownload->appendItem(auto_backup);
    }
    m_alreadyExport = true;
    ShowMessageBox(GET_TEXT("PLAYBACK/80135", "Please check download status in download progress interface. "));
}

void PlaybackFileSnapshot::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    switch (message->type()) {
    case RESPONSE_FLAG_SET_PLAYBACK_SNAPSHOT:
        ON_RESPONSE_FLAG_SET_PLAYBACK_SNAPSHOT(message);
        message->accept();
        break;
    case RESPONSE_FLAG_SNAPHOST_SPLIT_PLAYBACK:
        ON_RESPONSE_FLAG_SNAPHOST_SPLIT_PLAYBACK(message);
        message->accept();
        break;
    }
}

void PlaybackFileSnapshot::ON_RESPONSE_FLAG_SET_PLAYBACK_SNAPSHOT(MessageReceive *message)
{
    resp_snapshot_state *snapshot_state_array = (resp_snapshot_state *)message->data;

    if (!snapshot_state_array) {
        qWarning() << "PlaybackFileSnapshot::ON_RESPONSE_FLAG_SET_PLAYBACK_SNAPSHOT, data is null.";
        m_eventLoop.exit();
        return;
    }

    int count = message->header.size / sizeof(struct resp_snapshot_state);
    for (int i = 0; i < count; ++i) {
        resp_snapshot_state state;
        memset(&state, 0, sizeof(state));
        memcpy(&state, &snapshot_state_array[i], sizeof(state));
        if (BasePlayback::playbackType() == SplitPlayback) {
            state.chnid = PlaybackSplit::instance()->channel();
        }
        if (state.chnid < 0) {
            continue;
        }
        qMsDebug() << "snapshot, chnid:" << state.chnid << ", status:" << state.status;
        if (state.status == MF_SUCCESS) {
            PlaybackFileKey fileKey(state);
            m_fileMap.insert(fileKey, state);
            m_snapshotSucceedCount++;
        } else {
            AnimateToast::s_animateToast->addItem(QString::number(state.chnid + 1), GET_TEXT("PLAYBACK/80100", "Snapshot failed"));

            //write log
            struct log_data log_data;
            memset(&log_data, 0, sizeof(struct log_data));
            snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
            log_data.log_data_info.subType = SUB_EXCEPT_PLAYBACK_SNAPSHOT_FAIL;
            log_data.log_data_info.parameter_type = SUB_PARAM_NONE;
            log_data.log_data_info.chan_no = state.chnid + 1;
            MsWriteLog(log_data);
        }
        m_currentIndex++;
        //
        if (m_currentIndex >= m_currentCount) {
            m_eventLoop.exit();
        }
    }
}

void PlaybackFileSnapshot::ON_RESPONSE_FLAG_SNAPHOST_SPLIT_PLAYBACK(MessageReceive *message)
{
    resp_split_pb_snap_state *state = static_cast<resp_split_pb_snap_state *>(message->data);
    if (!state) {
        qMsWarning() << "data is nullptr";
        m_eventLoop.exit();
        return;
    }
    bool isFailed = false;
    for (int i = 0; i < MAX_LEN_16; ++i) {
        if (state->winid[i] != -1) {
            if (state->stateWinid[i] == '1') {

            } else {
                AnimateToast::s_animateToast->addItem(QString("%1-%2").arg(state->chnid + 1).arg(i + 1), GET_TEXT("PLAYBACK/80100", "Snapshot failed"));
                isFailed = true;
            }
            break;
        }
    }
    if (isFailed) {
        m_eventLoop.exit();
    } else {
        //实际截图结果在随后的RESPONSE_FLAG_SET_PLAYBACK_SNAPSHOT消息中
    }
}

void PlaybackFileSnapshot::onLanguageChanged()
{
    AbstractPlaybackFile::onLanguageChanged();

    ui->tableView->setHorizontalHeaderLabels(tableHeaders());
}

int PlaybackFileSnapshot::pageCount()
{
    int count = m_fileMap.size();
    m_pageCount = qCeil(count / 100.0);
    ui->lineEdit_page->setMaxPage(m_pageCount);
    return m_pageCount;
}

int PlaybackFileSnapshot::itemCount()
{
    int count = m_fileMap.size();
    return count;
}

void PlaybackFileSnapshot::dealSelected()
{
    int count = 0;
    quint64 bytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (ui->tableView->isItemChecked(row)) {
            const resp_snapshot_state &snapshot = ui->tableView->itemData(row, ColumnChannel, FileInfoRole).value<resp_snapshot_state>();
            count++;
            bytes += snapshot.size;
        }
    }
    ui->label_selectedCount->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80106", "Selected Items:")).arg(QString::number(count)));
    ui->label_selectedSize->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80107", "Total Size:")).arg(TableView::bytesString(bytes)));
}

QStringList PlaybackFileSnapshot::tableHeaders()
{
    QStringList headers;
    headers << "";
    headers << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headers << GET_TEXT("PLAYBACK/80105", "Disk");
    headers << GET_TEXT("LOG/64005", "Time");
    headers << GET_TEXT("COMMON/1053", "Size");
    return headers;
}

void PlaybackFileSnapshot::initializeTableView()
{
    const QStringList &headerList = tableHeaders();
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnDisk, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnSize, SortFilterProxyModel::SortInt);
    //column width
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnChannel, 100);
    ui->tableView->setColumnWidth(ColumnDisk, 100);
    ui->tableView->setColumnWidth(ColumnTime, 400);
}

void PlaybackFileSnapshot::onItemClicked(int row, int column)
{
    Q_UNUSED(row)

    switch (column) {
    case ColumnCheck: {
        dealSelected();
        break;
    }
    default:
        break;
    }
}
