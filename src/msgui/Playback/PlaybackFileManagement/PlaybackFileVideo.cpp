#include "PlaybackFileVideo.h"
#include "ui_AbstractPlaybackFile.h"
#include "BasePlayback.h"
#include "DownloadPanel.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackSplit.h"
#include "centralmessage.h"
#include "MsGlobal.h"
#include "MyFileSystemDialog.h"
#include <qmath.h>

extern "C" {

}

PlaybackFileVideo::PlaybackFileVideo(QWidget *parent)
    : AbstractPlaybackFile(parent)
{
    initializeTableView();
}

int PlaybackFileVideo::waitForCutPlayback(const QDateTime &begin, const QDateTime &end)
{
    m_alreadyExport = false;

    const QList<int> channelList = BasePlayback::channelCheckedList();

    req_search_common_backup common_backup;
    memset(&common_backup, 0, sizeof(req_search_common_backup));
    if (BasePlayback::playbackType() == SplitPlayback) {
        int channel = PlaybackSplit::instance()->channel();
        BasePlayback::makeChannelMask(channel, common_backup.chnMaskl, sizeof(common_backup.chnMaskl));
        common_backup.chnNum = 1;
    } else {
        BasePlayback::makeChannelMask(channelList, common_backup.chnMaskl, sizeof(common_backup.chnMaskl));
        common_backup.chnNum = channelList.count();
    }
    common_backup.enType = BasePlayback::playbackStream();
    common_backup.enEvent = REC_EVENT_ALL;
    common_backup.enState = SEG_STATE_ALL;
    snprintf(common_backup.pStartTime, sizeof(common_backup.pStartTime), "%s", begin.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(common_backup.pEndTime, sizeof(common_backup.pEndTime), "%s", end.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_COM_BACKUP, (void *)&common_backup, sizeof(req_search_common_backup));

    int result = m_eventLoop.exec();
    return result;
}

bool PlaybackFileVideo::hasNotExportedFile() const
{
    if (m_fileMap.isEmpty()) {
        return false;
    }
    if (m_alreadyExport) {
        return false;
    }
    return true;
}

void PlaybackFileVideo::exportVideo()
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
        const resp_search_common_backup &common_backup = ui->tableView->itemData(row, ColumnChannel, FileInfoRole).value<resp_search_common_backup>();
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
    m_alreadyExport = true;
    ShowMessageBox(GET_TEXT("PLAYBACK/80135", "Please check download status in download progress interface. "));
}

void PlaybackFileVideo::exportAllVideo()
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
        const resp_search_common_backup &common_backup = ui->tableView->itemData(row, ColumnChannel, FileInfoRole).value<resp_search_common_backup>();
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
    m_alreadyExport = true;
    ShowMessageBox(GET_TEXT("PLAYBACK/80135", "Please check download status in download progress interface. "));
}

void PlaybackFileVideo::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_COM_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_SEARCH_COM_PAGE:
        ON_RESPONSE_FLAG_GET_SEARCH_COM_PAGE(message);
        message->accept();
        break;
    }
}

void PlaybackFileVideo::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message)
{
    m_currentSid = -1;
    m_currentCount = 0;

    resp_search_common_backup *common_backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (!common_backup_array) {
        qWarning() << QString("PlaybackFileVideo::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, resp_search_common_backup is null.");
        m_eventLoop.exit(PlaybackError_NullBackup);
    } else {
        if (common_backup_array->allCnt == 0) {
            qDebug() << QString("PlaybackFileVideo::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, resp_search_common_backup is empty.");
            m_eventLoop.exit(PlaybackError_EmptyBackup);
        } else {
            m_currentCount += count;
            m_currentSid = common_backup_array->sid;
            qMsDebug() << "all count:" << common_backup_array->allCnt;

            for (int i = 0; i < count; ++i) {
                const resp_search_common_backup &common_backup = common_backup_array[i];
                //退出回放后要释放sid
                s_tempSidMap.insert(common_backup.sid, 0);
                //
                PlaybackFileKey fileKey(common_backup);
                m_fileMap.insert(fileKey, common_backup);
            }

            if (m_currentCount < common_backup_array->allCnt) {
                //继续搜索
                struct rep_get_search_backup search_backup;
                memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
                search_backup.sid = m_currentSid;
                search_backup.npage = m_currentCount / 100 + 1;
                MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_SEARCH_COM_PAGE, (void *)&search_backup, sizeof(struct rep_get_search_backup));
            } else {
                m_eventLoop.exit(PlaybackError_NoError);
            }
        }
    }
}

void PlaybackFileVideo::ON_RESPONSE_FLAG_GET_SEARCH_COM_PAGE(MessageReceive *message)
{
    resp_search_common_backup *common_backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (!common_backup_array) {
        qWarning() << QString("PlaybackFileVideo::ON_RESPONSE_FLAG_GET_SEARCH_COM_PAGE, resp_search_common_backup is null.");
        m_eventLoop.exit(PlaybackError_NullBackup);
    } else {
        if (common_backup_array->allCnt == 0) {
            qDebug() << QString("PlaybackFileVideo::ON_RESPONSE_FLAG_GET_SEARCH_COM_PAGE, resp_search_common_backup is empty.");
            m_eventLoop.exit(PlaybackError_EmptyBackup);
        } else {
            m_currentCount += count;

            for (int i = 0; i < count; ++i) {
                const resp_search_common_backup &common_backup = common_backup_array[i];
                PlaybackFileKey fileKey(common_backup);
                m_fileMap.insert(fileKey, common_backup);
            }

            if (m_currentCount < common_backup_array->allCnt) {
                //继续搜索
                struct rep_get_search_backup search_backup;
                memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
                search_backup.sid = m_currentSid;
                search_backup.npage = m_currentCount / 100 + 1;
                MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_SEARCH_COM_PAGE, (void *)&search_backup, sizeof(struct rep_get_search_backup));
            } else {
                m_eventLoop.exit(PlaybackError_NoError);
            }
        }
    }
}

void PlaybackFileVideo::onLanguageChanged()
{
    AbstractPlaybackFile::onLanguageChanged();

    ui->tableView->setHorizontalHeaderLabels(tableHeaders());
}

int PlaybackFileVideo::pageCount()
{
    int count = m_fileMap.size();
    m_pageCount = qCeil(count / 100.0);
    ui->lineEdit_page->setMaxPage(m_pageCount);
    return m_pageCount;
}

int PlaybackFileVideo::itemCount()
{
    return m_fileMap.size();
}

void PlaybackFileVideo::dealSelected()
{
    int count = 0;
    quint64 bytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (ui->tableView->isItemChecked(row)) {
            const resp_search_common_backup &backup = ui->tableView->itemData(row, ColumnChannel, FileInfoRole).value<resp_search_common_backup>();
            count++;
            bytes += backup.size;
        }
    }
    ui->label_selectedCount->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80106", "Selected Items:")).arg(QString::number(count)));
    ui->label_selectedSize->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80107", "Total Size:")).arg(TableView::bytesString(bytes)));
}

QStringList PlaybackFileVideo::tableHeaders()
{
    QStringList headers;
    headers << "";
    headers << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headers << GET_TEXT("COMMONBACKUP/100012", "Start Time-End Time");
    headers << GET_TEXT("COMMON/1053", "Size");
    return headers;
}

void PlaybackFileVideo::initializeTableView()
{
    const QStringList &headerList = tableHeaders();
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onItemClicked(int, int)));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnSize, SortFilterProxyModel::SortInt);
    //column width
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnChannel, 100);
    ui->tableView->setColumnWidth(ColumnTime, 400);
}

void PlaybackFileVideo::updateTableList()
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
        ui->tableView->setItemData(row, ColumnChannel, QVariant::fromValue(value), FileInfoRole);
        ui->tableView->setItemText(row, ColumnTime, key.dateTimeString());
        ui->tableView->setItemBytesValue(row, ColumnSize, value.size);

        row++;
        if (row >= 100) {
            break;
        }
    }

    dealSelected();
}

void PlaybackFileVideo::clear()
{
    m_fileMap.clear();
    ui->tableView->clearContent();

    dealSelected();
}

void PlaybackFileVideo::onItemClicked(int row, int column)
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
