#include "PlaybackFileLock.h"
#include "ui_AbstractPlaybackFile.h"
#include "AnimateToast.h"
#include "BasePlayback.h"
#include "DownloadPanel.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"
#include "PlaybackBar.h"
#include "PlaybackSplit.h"
#include "centralmessage.h"
#include <qmath.h>

extern "C" {

}

struct CommonLockFileInfo {
    int channel = 0;
    QString beginDateTime;
    QString endDateTime;
};

PlaybackFileLock::PlaybackFileLock(QWidget *parent)
    : AbstractPlaybackFile(parent)
{
    m_waitting = new MsWaitting(this);

    initializeTableView();
}

void PlaybackFileLock::updateTableList()
{
    AbstractPlaybackFile::updateTableList();

    ui->tableView->clearContent();
    int row = 0;
    auto iter = m_fileMap.constBegin();
    for (iter += m_pageIndex * 100; iter != m_fileMap.constEnd(); ++iter) {
        const auto &key = iter.key();
        const auto &value = iter.value();
        ui->tableView->setItemIntValue(row, ColumnChannel, value.chnid + 1);
        ui->tableView->setItemData(row, ColumnChannel, QVariant::fromValue(key), FileKeyRole);
        ui->tableView->setItemText(row, ColumnTime, key.dateTimeString());
        ui->tableView->setItemBytesValue(row, ColumnSize, value.size);
        ui->tableView->setItemData(row, ColumnLock, value.isLock, ItemCheckedRole);

        row++;
        if (row >= 100) {
            break;
        }
    }

    dealSelected();
}

void PlaybackFileLock::clear()
{
    m_fileMap.clear();
    ui->tableView->clearContent();

    dealSelected();
}

int PlaybackFileLock::addLockFile(const resp_search_common_backup &common_backup)
{
    PlaybackFileKey fileKey(common_backup);
    m_fileMap.insert(fileKey, common_backup);
    return 0;
}

int PlaybackFileLock::waitForLockAll(const QDateTime &dateTime)
{
    qMsDebug() << QString("begin");

    int succeedCount = 0;

    QList<CommonLockFileInfo> infoList;
    if (BasePlayback::playbackType() == SplitPlayback) {
        QMap<int, QDateTime> splitDateTimeMap = BasePlayback::s_playbackBar->splitDateTimeMap();
        for (auto iter = splitDateTimeMap.constBegin(); iter != splitDateTimeMap.constEnd(); ++iter) {
            int channel = PlaybackSplit::instance()->channel();
            int sid = iter.key();
            QDateTime dateTime = iter.value();
            const resp_search_common_backup &search_backup = PlaybackSplit::instance()->findCommonBackup(sid, dateTime);
            if (search_backup.chnid < 0) {
                AnimateToast::s_animateToast->addItem(QString("%1-%2").arg(channel + 1).arg(sid + 1), GET_TEXT("PLAYBACK/80115", "No record"));
                continue;
            }
            CommonLockFileInfo info;
            info.channel = channel;
            info.beginDateTime = QString(search_backup.pStartTime);
            info.endDateTime = QString(search_backup.pEndTime);
            infoList.append(info);
        }
    } else {
        const QList<int> &channelList = BasePlayback::channelCheckedList();
        for (int i = 0; i < channelList.size(); ++i) {
            int channel = channelList.at(i);
            const resp_search_common_backup &search_backup = BasePlayback::findCommonBackup(channel, dateTime);
            if (search_backup.chnid < 0) {
                AnimateToast::s_animateToast->addItem(QString::number(channel + 1), GET_TEXT("PLAYBACK/80115", "No record"));
                continue;
            }
            CommonLockFileInfo info;
            info.channel = channel;
            info.beginDateTime = QString(search_backup.pStartTime);
            info.endDateTime = QString(search_backup.pEndTime);
            infoList.append(info);
        }
    }

    //
    for (int i = 0; i < infoList.size(); ++i) {
        const CommonLockFileInfo &info = infoList.at(i);

        //search
        req_search_common_backup req_search_backup;
        memset(&req_search_backup, 0, sizeof(req_search_common_backup));
        BasePlayback::makeChannelMask(info.channel, req_search_backup.chnMaskl, sizeof(req_search_backup.chnMaskl));
        req_search_backup.chnNum = 1;
        req_search_backup.enType = BasePlayback::playbackStream();
        req_search_backup.enEvent = REC_EVENT_ALL;
        req_search_backup.enState = SEG_STATE_ALL;
        strcpy(req_search_backup.pStartTime, info.beginDateTime.toStdString().c_str());
        strcpy(req_search_backup.pEndTime, info.endDateTime.toStdString().c_str());
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_COM_BACKUP, (void *)&req_search_backup, sizeof(req_search_common_backup));

        // int search_result = m_eventLoop.exec();
        // if (search_result != 0 || m_search_common_backup_list.isEmpty()) {
        //     AnimateToast::s_animateToast->addItem(QString::number(info.channel + 1), GET_TEXT("PLAYBACK/80115", "No record"));
        //     continue;
        // }

        //lock
        if (m_search_common_backup_list.size() > 1) {
            qMsWarning() << QString("m_search_common_backup_list, size: %1").arg(m_search_common_backup_list.size());
            for (int i = 0; i < m_search_common_backup_list.size(); ++i) {
                const resp_search_common_backup &temp_search_common_backup = m_search_common_backup_list.at(i);
                qMsWarning() << QString("temp_search_common_backup, index: %1, channel: %2, start time: %3, end time: %4")
                                    .arg(i)
                                    .arg(temp_search_common_backup.chnid)
                                    .arg(temp_search_common_backup.pStartTime)
                                    .arg(temp_search_common_backup.pEndTime);
            }
        }
        for (int i = 0; i < m_search_common_backup_list.size(); ++i) {
            const resp_search_common_backup &temp_search_common_backup = m_search_common_backup_list.at(i);

            PlaybackFileKey fileKey(temp_search_common_backup);

            if (temp_search_common_backup.isLock) {
                m_fileMap.insert(fileKey, temp_search_common_backup);
                succeedCount++;
                continue;
            }
            //更新录像信息，更新录像显示界面
            BasePlayback::setCommonBackupLock(temp_search_common_backup, 1);

            //
            rep_lock_common_backup lockinfo;
            memset(&lockinfo, 0, sizeof(rep_lock_common_backup));
            lockinfo.chnid = temp_search_common_backup.chnid;
            lockinfo.sid = temp_search_common_backup.sid;
            lockinfo.isLock = 1;
            lockinfo.size = temp_search_common_backup.size;
            strcpy(lockinfo.pStartTime, temp_search_common_backup.pStartTime);
            strcpy(lockinfo.pEndTime, temp_search_common_backup.pEndTime);
            qMsDebug() << QString("\nREQUEST_FLAG_LOCK_COM_BACKUP, channel: %1, pStartTime: %2, pEndTime: %3")
                              .arg(lockinfo.chnid)
                              .arg(lockinfo.pStartTime)
                              .arg(lockinfo.pEndTime);
            MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_LOCK_COM_BACKUP, (void *)&lockinfo, sizeof(rep_lock_common_backup));

            // int lock_result = m_eventLoop.exec();
            // if (lock_result != 0) {
            //     AnimateToast::s_animateToast->addItem(QString::number(info.channel + 1), "Lock failed");
            // } else {
            //     m_fileMap.insert(fileKey, temp_search_common_backup);
            //     m_fileMap[fileKey].isLock = true;
            //     succeedCount++;
            // }
        }
    }
    qMsDebug() << QString("end");
    return succeedCount;
}

int PlaybackFileLock::waitForLock(const QDateTime &dateTime)
{
    int succeedCount = 0;

    QList<CommonLockFileInfo> infoList;
    if (BasePlayback::playbackType() == SplitPlayback) {
        int channel = PlaybackSplit::instance()->channel();
        int sid = PlaybackSplit::instance()->selectedSid();
        QDateTime dateTime = BasePlayback::s_playbackBar->splitDateTime();
        const resp_search_common_backup &search_backup = PlaybackSplit::instance()->findCommonBackup(sid, dateTime);
        if (search_backup.chnid < 0) {
            AnimateToast::s_animateToast->addItem(QString("%1-%2").arg(channel + 1).arg(sid + 1), GET_TEXT("PLAYBACK/80115", "No record"));
        } else {
            CommonLockFileInfo info;
            info.channel = channel;
            info.beginDateTime = QString(search_backup.pStartTime);
            info.endDateTime = QString(search_backup.pEndTime);
            infoList.append(info);
        }
    } else {
        int channel = BasePlayback::currentChannel();
        const resp_search_common_backup &search_backup = BasePlayback::findCommonBackup(channel, dateTime);
        if (search_backup.chnid < 0) {
            AnimateToast::s_animateToast->addItem(QString::number(channel + 1), GET_TEXT("PLAYBACK/80115", "No record"));
        } else {
            CommonLockFileInfo info;
            info.channel = channel;
            info.beginDateTime = QString(search_backup.pStartTime);
            info.endDateTime = QString(search_backup.pEndTime);
            infoList.append(info);
        }
    }

    //
    for (int i = 0; i < infoList.size(); ++i) {
        const CommonLockFileInfo &info = infoList.at(i);

        //search
        req_search_common_backup req_search_backup;
        memset(&req_search_backup, 0, sizeof(req_search_common_backup));
        BasePlayback::makeChannelMask(info.channel, req_search_backup.chnMaskl, sizeof(req_search_backup.chnMaskl));
        req_search_backup.chnNum = 1;
        req_search_backup.enType = BasePlayback::playbackStream();
        req_search_backup.enEvent = REC_EVENT_ALL;
        req_search_backup.enState = SEG_STATE_ALL;
        strcpy(req_search_backup.pStartTime, info.beginDateTime.toStdString().c_str());
        strcpy(req_search_backup.pEndTime, info.endDateTime.toStdString().c_str());
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_COM_BACKUP, (void *)&req_search_backup, sizeof(req_search_common_backup));

        // int search_result = m_eventLoop.exec();
        // if (search_result != 0 || m_search_common_backup_list.isEmpty()) {
        //     AnimateToast::s_animateToast->addItem(QString::number(info.channel + 1), GET_TEXT("PLAYBACK/80115", "No record"));
        //     continue;
        // }

        //lock
        if (m_search_common_backup_list.size() > 1) {
            qMsWarning() << QString("m_search_common_backup_list, size: %1").arg(m_search_common_backup_list.size());
            for (int i = 0; i < m_search_common_backup_list.size(); ++i) {
                const resp_search_common_backup &temp_search_common_backup = m_search_common_backup_list.at(i);
                qMsWarning() << QString("temp_search_common_backup, index: %1, channel: %2, start time: %3, end time: %4")
                                    .arg(i)
                                    .arg(temp_search_common_backup.chnid)
                                    .arg(temp_search_common_backup.pStartTime)
                                    .arg(temp_search_common_backup.pEndTime);
            }
        }
        for (int i = 0; i < m_search_common_backup_list.size(); ++i) {
            const resp_search_common_backup &temp_search_common_backup = m_search_common_backup_list.at(i);

            PlaybackFileKey fileKey(temp_search_common_backup);

            if (temp_search_common_backup.isLock) {
                m_fileMap.insert(fileKey, temp_search_common_backup);
                succeedCount++;
                continue;
            }
            //更新录像信息，更新录像显示界面
            BasePlayback::setCommonBackupLock(temp_search_common_backup, 1);

            //
            rep_lock_common_backup lockinfo;
            memset(&lockinfo, 0, sizeof(rep_lock_common_backup));
            lockinfo.chnid = temp_search_common_backup.chnid;
            lockinfo.sid = temp_search_common_backup.sid;
            lockinfo.isLock = 1;
            lockinfo.size = temp_search_common_backup.size;
            strcpy(lockinfo.pStartTime, temp_search_common_backup.pStartTime);
            strcpy(lockinfo.pEndTime, temp_search_common_backup.pEndTime);
            qMsDebug() << QString("\nREQUEST_FLAG_LOCK_COM_BACKUP, channel: %1, pStartTime: %2, pEndTime: %3")
                              .arg(lockinfo.chnid)
                              .arg(lockinfo.pStartTime)
                              .arg(lockinfo.pEndTime);
            MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_LOCK_COM_BACKUP, (void *)&lockinfo, sizeof(rep_lock_common_backup));

            // int lock_result = m_eventLoop.exec();
            // if (lock_result != 0) {
            //     AnimateToast::s_animateToast->addItem(QString::number(info.channel + 1), "Lock failed");
            // } else {
            //     m_fileMap.insert(fileKey, temp_search_common_backup);
            //     m_fileMap[fileKey].isLock = true;
            //     succeedCount++;
            // }
        }
    }
    return succeedCount;
}

void PlaybackFileLock::exportLock()
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
    const QString &directory = MyFileSystemDialog::instance()->exportVideo();
    if (directory.isEmpty()) {
        return;
    }
    const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int &fileType = MyFileSystemDialog::instance()->fileType();

    //
    QList<rep_export_common_backup> exportList;
    qint64 totalBytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (!ui->tableView->isItemChecked(row)) {
            continue;
        }
        const PlaybackFileKey &file_key = ui->tableView->itemData(row, ColumnChannel, FileKeyRole).value<PlaybackFileKey>();
        const resp_search_common_backup &common_backup = m_fileMap.value(file_key);
        totalBytes += common_backup.size;

        rep_export_common_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_common_backup));
        exportBackup.chnid = common_backup.chnid;
        exportBackup.sid = common_backup.sid;
        exportBackup.filesize = common_backup.size;
        exportBackup.fileno = common_backup.fileno;
        exportBackup.filetype = fileType;
        snprintf(exportBackup.pStartTime, sizeof(exportBackup.pStartTime), "%s", common_backup.pStartTime);
        snprintf(exportBackup.pEndTime, sizeof(exportBackup.pEndTime), "%s", common_backup.pEndTime);
        snprintf(exportBackup.dev_name, sizeof(exportBackup.dev_name), "%s", deviceName.toStdString().c_str());
        snprintf(exportBackup.dev_path, sizeof(exportBackup.dev_path), "%s", directory.toStdString().c_str());

        exportList.append(exportBackup);
    }

    if (totalBytes > deviceFreeSize) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

    for (int i = 0; i < exportList.size(); ++i) {
        const rep_export_common_backup &exportBackup = exportList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid = gDownload->availableId();
        auto_backup.chnid = exportBackup.chnid;
        auto_backup.enType = BasePlayback::playbackStream();
        auto_backup.filetype = exportBackup.filetype;
        auto_backup.filesize = exportBackup.filesize;
        auto_backup.fileno = exportBackup.fileno;
        snprintf(auto_backup.pStartTime, sizeof(auto_backup.pStartTime), "%s", exportBackup.pStartTime);
        snprintf(auto_backup.pEndTime, sizeof(auto_backup.pEndTime), "%s", exportBackup.pEndTime);
        snprintf(auto_backup.dev_name, sizeof(auto_backup.dev_name), "%s", exportBackup.dev_name);
        snprintf(auto_backup.dev_path, sizeof(auto_backup.dev_path), "%s", exportBackup.dev_path);

        //char fileName[128];
        //get_auto_backup_vedio_filename(&auto_backup, fileName, sizeof(fileName));
        //snprintf(auto_backup.filename, sizeof(auto_backup.filename), "%s", fileName);

        //gDownload->appendItem(auto_backup);
    }
    ShowMessageBox(GET_TEXT("PLAYBACK/80135", "Please check download status in download progress interface. "));
}

void PlaybackFileLock::exportAllLock()
{
    if (m_fileMap.isEmpty()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        return;
    }

    //
    const QString &directory = MyFileSystemDialog::instance()->exportVideo();
    if (directory.isEmpty()) {
        return;
    }
    const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int &fileType = MyFileSystemDialog::instance()->fileType();

    //
    QList<rep_export_common_backup> exportList;
    qint64 totalBytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        const PlaybackFileKey &file_key = ui->tableView->itemData(row, ColumnChannel, FileKeyRole).value<PlaybackFileKey>();
        const resp_search_common_backup &common_backup = m_fileMap.value(file_key);
        totalBytes += common_backup.size;

        rep_export_common_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_common_backup));
        exportBackup.chnid = common_backup.chnid;
        exportBackup.sid = common_backup.sid;
        exportBackup.filesize = common_backup.size;
        exportBackup.fileno = common_backup.fileno;
        exportBackup.filetype = fileType;
        snprintf(exportBackup.pStartTime, sizeof(exportBackup.pStartTime), "%s", common_backup.pStartTime);
        snprintf(exportBackup.pEndTime, sizeof(exportBackup.pEndTime), "%s", common_backup.pEndTime);
        snprintf(exportBackup.dev_name, sizeof(exportBackup.dev_name), "%s", deviceName.toStdString().c_str());
        snprintf(exportBackup.dev_path, sizeof(exportBackup.dev_path), "%s", directory.toStdString().c_str());

        exportList.append(exportBackup);
    }

    if (totalBytes > deviceFreeSize) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

    for (int i = 0; i < exportList.size(); ++i) {
        const rep_export_common_backup &exportBackup = exportList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid = gDownload->availableId();
        auto_backup.chnid = exportBackup.chnid;
        auto_backup.enType = BasePlayback::playbackStream();
        auto_backup.filetype = exportBackup.filetype;
        auto_backup.filesize = exportBackup.filesize;
        auto_backup.fileno = exportBackup.fileno;
        snprintf(auto_backup.pStartTime, sizeof(auto_backup.pStartTime), "%s", exportBackup.pStartTime);
        snprintf(auto_backup.pEndTime, sizeof(auto_backup.pEndTime), "%s", exportBackup.pEndTime);
        snprintf(auto_backup.dev_name, sizeof(auto_backup.dev_name), "%s", exportBackup.dev_name);
        snprintf(auto_backup.dev_path, sizeof(auto_backup.dev_path), "%s", exportBackup.dev_path);

        char fileName[128];
        //get_auto_backup_vedio_filename(&auto_backup, fileName, sizeof(fileName));
        snprintf(auto_backup.filename, sizeof(auto_backup.filename), "%s", fileName);

        gDownload->appendItem(auto_backup);
    }
    ShowMessageBox(GET_TEXT("PLAYBACK/80135", "Please check download status in download progress interface. "));
}

void PlaybackFileLock::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_COM_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(message);
        message->accept();
        break;
    case RESPONSE_FLAG_LOCK_COM_BACKUP:
        ON_RESPONSE_FLAG_LOCK_COM_BACKUP(message);
        message->accept();
        break;
    default:
        break;
    }
}

void PlaybackFileLock::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message)
{
    m_search_common_backup_list.clear();

    resp_search_common_backup *common_backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (!common_backup_array) {
        qWarning() << QString("PlaybackFileLock::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, resp_search_common_backup is null.");
        m_eventLoop.exit(-1);
    } else {
        if (common_backup_array->allCnt == 0) {
            qDebug() << QString("PlaybackFileLock::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, resp_search_common_backup is empty.");
            m_eventLoop.exit(-1);
        } else {
            for (int i = 0; i < count; ++i) {
                const resp_search_common_backup &common_backup = common_backup_array[i];
                qDebug() << QString("PlaybackFileLock::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, sid: %1, channel: %2, start time: %3, end time: %4")
                                .arg(common_backup.sid)
                                .arg(common_backup.chnid)
                                .arg(common_backup.pStartTime)
                                .arg(common_backup.pEndTime);
                //退出回放后要释放sid
                s_tempSidMap.insert(common_backup.sid, 0);
                //过滤掉开始时间和结束时间一样的
                if (strcmp(common_backup.pStartTime, common_backup.pEndTime) == 0) {
                    continue;
                }

                m_search_common_backup_list.append(common_backup);
            }
            m_eventLoop.exit();
        }
    }
}

void PlaybackFileLock::ON_RESPONSE_FLAG_LOCK_COM_BACKUP(MessageReceive *message)
{
    struct resp_common_state *resp = (resp_common_state *)message->data;
    if (resp) {
        qDebug() << QString("PlaybackFileLock::ON_RESPONSE_FLAG_LOCK_COM_BACKUP, channel: %1, result: %2").arg(resp->chnid).arg(resp->state);
        if (resp->state != 0) {
            m_eventLoop.exit(-1);
        } else {
            m_eventLoop.exit();
        }
    } else {
        qDebug() << QString("PlaybackFileLock::ON_RESPONSE_FLAG_LOCK_COM_BACKUP, data is null.");
        m_eventLoop.exit();
    }

    //m_waitting->//closeWait();
}

void PlaybackFileLock::onLanguageChanged()
{
    AbstractPlaybackFile::onLanguageChanged();

    ui->tableView->setHorizontalHeaderLabels(tableHeaders());
}

int PlaybackFileLock::pageCount()
{
    int count = m_fileMap.size();
    m_pageCount = qCeil(count / 100.0);
    ui->lineEdit_page->setMaxPage(m_pageCount);
    return m_pageCount;
}

int PlaybackFileLock::itemCount()
{
    int count = m_fileMap.size();
    return count;
}

void PlaybackFileLock::dealSelected()
{
    int count = 0;
    quint64 bytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (ui->tableView->isItemChecked(row)) {
            const PlaybackFileKey &file_key = ui->tableView->itemData(row, ColumnChannel, FileKeyRole).value<PlaybackFileKey>();
            const resp_search_common_backup &common_backup = m_fileMap.value(file_key);
            count++;
            bytes += common_backup.size;
        }
    }
    ui->label_selectedCount->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80106", "Selected Items:")).arg(QString::number(count)));
    ui->label_selectedSize->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80107", "Total Size:")).arg(TableView::bytesString(bytes)));
}

QStringList PlaybackFileLock::tableHeaders()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("COMMONBACKUP/100012", "Start Time-End Time");
    headerList << GET_TEXT("COMMON/1053", "Size");
    headerList << GET_TEXT("LIVEVIEW/20060", "Lock");
    return headerList;
}

void PlaybackFileLock::initializeTableView()
{
    const QStringList &headerList = tableHeaders();
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnLock, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonLock, this));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnSize, SortFilterProxyModel::SortInt);
    //column width
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnChannel, 100);
    ui->tableView->setColumnWidth(ColumnTime, 400);
    ui->tableView->setColumnWidth(ColumnSize, 100);
}

void PlaybackFileLock::onItemClicked(int row, int column)
{
    switch (column) {
    case ColumnCheck: {
        dealSelected();
        break;
    }
    case ColumnLock: {
        bool isLock = ui->tableView->itemData(row, column, ItemCheckedRole).toBool();
        if (isLock) {
            if (MessageBox::question(this, GET_TEXT("COMMONBACKUP/100034", "Record file may be overwritten after unlocking, continue?")) == MessageBox::Cancel) {
                return;
            }
        }

        const PlaybackFileKey &file_key = ui->tableView->itemData(row, ColumnChannel, FileKeyRole).value<PlaybackFileKey>();
        resp_search_common_backup &common_backup = m_fileMap[file_key];

        struct rep_lock_common_backup lockinfo;
        memset(&lockinfo, 0, sizeof(struct rep_lock_common_backup));
        lockinfo.chnid = common_backup.chnid;
        lockinfo.sid = common_backup.sid;
        lockinfo.size = common_backup.size;
        lockinfo.isLock = ((common_backup.isLock) ? 0 : 1);
        strcpy(lockinfo.pStartTime, common_backup.pStartTime);
        strcpy(lockinfo.pEndTime, common_backup.pEndTime);
        qDebug() << "====REQUEST_FLAG_LOCK_COM_BACKUP====";
        qDebug() << "----chnid:" << lockinfo.chnid;
        qDebug() << "----sid:" << lockinfo.sid;
        qDebug() << "----size:" << lockinfo.size;
        qDebug() << "----isLock:" << lockinfo.isLock;
        qDebug() << "----start time:" << lockinfo.pStartTime;
        qDebug() << "----end time:" << lockinfo.pEndTime;
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_LOCK_COM_BACKUP, (void *)&lockinfo, sizeof(struct rep_lock_common_backup));

        m_waitting->execWait();

        common_backup.isLock = lockinfo.isLock;
        ui->tableView->setItemData(row, column, !isLock, ItemCheckedRole);

        //更新录像信息，更新录像显示界面
        BasePlayback::setCommonBackupLock(common_backup, common_backup.isLock);
        break;
    }
    default:
        break;
    }
}
