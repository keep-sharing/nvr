#include "PlaybackPictureList.h"
#include "ui_PlaybackList.h"
#include "centralmessage.h"
#include "DownloadPanel.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "msuser.h"
#include "MyFileSystemDialog.h"
#include "PicturePlay.h"
#include <QtDebug>
#include <qmath.h>

extern "C" {

}

const int PictureInfoRole = Qt::UserRole + 50;

PlaybackPictureList *PlaybackPictureList::s_playbackPictureList = nullptr;

PlaybackPictureList::PlaybackPictureList(QWidget *parent)
    : PlaybackList(parent)
{
    s_playbackPictureList = this;
    ui->widget_time->hide();

    m_channelTextDelegate = new ItemTextDelegate(this);
    m_timeTextDelegate = new ItemTextDelegate(this);

    //
    initializeTreeView();
}

PlaybackPictureList::~PlaybackPictureList()
{
    s_playbackPictureList = nullptr;
}

PlaybackPictureList *PlaybackPictureList::instance()
{
    return s_playbackPictureList;
}

void PlaybackPictureList::initializeData()
{
    PicturePlay::instance()->clear();
}

void PlaybackPictureList::closePlayback()
{
    on_toolButton_close_clicked();
}

bool PlaybackPictureList::playPrevious()
{
    int row = ui->treeView->currentIndex().row();
    if (row > 0) {
        row--;
        ui->treeView->setCurrentRow(row);
        playItem(row);
        return true;
    } else {
        return false;
    }
}

bool PlaybackPictureList::playNext()
{
    int row = ui->treeView->currentIndex().row();
    if (row < ui->treeView->rowCount() - 1) {
        row++;
        ui->treeView->setCurrentRow(row);
        playItem(row);
        return true;
    } else {
        return false;
    }
}

NetworkResult PlaybackPictureList::dealNetworkCommond(const QString &commond)
{
    qDebug() << "PlaybackPictureList::dealNetworkCommond," << commond;

    NetworkResult result = NetworkReject;
    if (commond.startsWith("Video_Play")) {
        playItem(m_currentRow);
        result = NetworkAccept;
    } else if (commond.startsWith("Dial_Insid_Add")) {
        //选中下一个
        result = playNext() ? NetworkAccept : NetworkReject;
    } else if (commond.startsWith("Dial_Insid_Sub")) {
        //选中上一个
        result = playPrevious() ? NetworkAccept : NetworkReject;
    }

    return result;
}

void PlaybackPictureList::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PlaybackPictureList::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_PIC_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_PIC_BACKUP(message);
        break;
    case RESPONSE_FLAG_GET_SEARCH_PIC_PAGE:
        ON_RESPONSE_FLAG_GET_SEARCH_PIC_PAGE(message);
        break;
    case RESPONSE_FLAG_PLAY_PIC_BACKUP:
        ON_RESPONSE_FLAG_PLAY_PIC_BACKUP(message);
        break;
    default:
        break;
    }
}

void PlaybackPictureList::ON_RESPONSE_FLAG_SEARCH_PIC_BACKUP(MessageReceive *message)
{
    m_pageIndex = 0;
    m_pageCount = 0;

    //
    struct resp_search_picture_backup *picture_backup_array = (struct resp_search_picture_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_picture_backup);
    if (!picture_backup_array) {
        qWarning() << "PlaybackPictureList::ON_RESPONSE_FLAG_SEARCH_PIC_BACKUP, data is null.";
        return;
    }
    m_searchSid = picture_backup_array->sid;
    m_backupList.clear();
    for (int i = 0; i < count; ++i) {
        m_backupList.append(picture_backup_array[i]);
    }

    //page
    m_allCount = picture_backup_array->allCnt;
    if (m_allCount > MAX_SEARCH_BACKUP_COUNT) {
        m_allCount = MAX_SEARCH_BACKUP_COUNT;
    }
    m_pageCount = qCeil(m_allCount / 100.0);
    ui->lineEdit_page->setMaxPage(m_pageCount);
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_allCount).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    updateTreeList();
}

void PlaybackPictureList::ON_RESPONSE_FLAG_GET_SEARCH_PIC_PAGE(MessageReceive *message)
{
    struct resp_search_picture_backup *picture_backup_array = (struct resp_search_picture_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_picture_backup);
    if (!picture_backup_array) {
        qWarning() << "PlaybackPictureList::ON_RESPONSE_FLAG_GET_SEARCH_PIC_PAGE, data is null.";
        return;
    }

    m_backupList.clear();
    for (int i = 0; i < count; ++i) {
        m_backupList.append(picture_backup_array[i]);
    }

    //page
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_allCount).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    updateTreeList();
}

void PlaybackPictureList::ON_RESPONSE_FLAG_PLAY_PIC_BACKUP(MessageReceive *message)
{
    PicturePlay::instance()->showImage(message);
}

QStringList PlaybackPictureList::treeHeaders()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("LOG/64005", "Time");
    headerList << GET_TEXT("PLAYBACK/80089", "Play");
    return headerList;
}

void PlaybackPictureList::initializeTreeView()
{
    const QStringList &headerList = treeHeaders();
    ui->treeView->setHorizontalHeaderLabels(headerList);
    ui->treeView->setColumnCount(headerList.count());
    ui->treeView->setHorizontalHeaderCheckable(true);
    ui->treeView->setColumnWidth(ColumnCheck, 30);
    ui->treeView->setColumnWidth(ColumnChannel, 30);
    ui->treeView->setColumnWidth(ColumnTime, 150);
    ui->treeView->setItemDelegateForColumn(ColumnChannel, m_channelTextDelegate);
    ui->treeView->setItemDelegateForColumn(ColumnTime, m_timeTextDelegate);
    ui->treeView->setItemDelegateForColumn(ColumnCheck, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this));
    ui->treeView->setItemDelegateForColumn(ColumnPlay, new ItemButtonDelegate(QPixmap(":/playback/playback/event_play.png"), QSize(18, 18), this));
    connect(ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
}

void PlaybackPictureList::updateTreeList()
{
    m_currentRow = -1;
    ui->treeView->clearContent();

    ui->treeView->setRowCount(m_backupList.count());
    for (int i = 0; i < m_backupList.count(); ++i) {
        const resp_search_picture_backup &picture_backup = m_backupList.at(i);
        ui->treeView->setItemText(i, ColumnChannel, QString::number(picture_backup.chnid + 1));
        ui->treeView->setItemData(i, ColumnChannel, QVariant::fromValue(picture_backup), PictureInfoRole);
        const QDateTime &dateTime = QDateTime::fromString(picture_backup.pTime, "yyyy-MM-dd HH:mm:ss");
        ui->treeView->setItemText(i, ColumnTime, QString("%1").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));

        ItemKey itemKey(i, m_pageIndex);
        if (itemKey == m_currentItemKey) {
            setItemTextColor(i, "#0AA8E3");
        }
    }

    if (m_backupList.isEmpty()) {
        PicturePlay::instance()->clear();
    } else {
        PicturePlay::instance()->initializeData();
    }
}

void PlaybackPictureList::playItem(int row)
{
    const resp_search_picture_backup &backup = ui->treeView->itemData(row, ColumnChannel, PictureInfoRole).value<resp_search_picture_backup>();

    //PicturePlay::instance()->showImageInfo();

    struct rep_play_picture_backup play_picture_backup;
    memset(&play_picture_backup, 0, sizeof(struct rep_play_picture_backup));
    play_picture_backup.chnid = backup.chnid;
    play_picture_backup.sid = backup.sid;
    play_picture_backup.port = backup.port;
    strcpy(play_picture_backup.pTime, backup.pTime);
    play_picture_backup.pts = backup.pts;
    sendMessage(REQUEST_FLAG_PLAY_PIC_BACKUP, (void *)&play_picture_backup, sizeof(struct rep_play_picture_backup));

    m_currentItemKey = ItemKey(row, m_pageIndex);
    clearAllItemTextColor();
    setItemTextColor(row, "#0AA8E3");
}

void PlaybackPictureList::searchPicturePage(int page)
{
    struct rep_get_search_backup pageinfo;
    memset(&pageinfo, 0, sizeof(struct rep_get_search_backup));
    pageinfo.sid = m_backupList.first().sid;
    pageinfo.npage = page;
    qDebug() << QString("PlaybackPictureList::searchPicturePage, sid: %1, page: %2").arg(pageinfo.sid).arg(pageinfo.npage);
    sendMessage(REQUEST_FLAG_GET_SEARCH_PIC_PAGE, (void *)&pageinfo, sizeof(struct rep_get_search_backup));
}

void PlaybackPictureList::setItemTextColor(int row, const QString &color)
{
    ui->treeView->setItemData(row, ColumnChannel, color, ItemTextDelegate::TextColorRole);
    ui->treeView->setItemData(row, ColumnTime, color, ItemTextDelegate::TextColorRole);
}

void PlaybackPictureList::clearAllItemTextColor()
{
    for (int i = 0; i < ui->treeView->rowCount(); ++i) {
        setItemTextColor(i, QString());
    }
}

void PlaybackPictureList::onItemClicked(const QModelIndex &index)
{
    switch (index.column()) {
    case ColumnCheck: {
        bool checked = ui->treeView->isItemChecked(index.row());
        checked = !checked;
        ui->treeView->setItemChecked(index.row(), checked);
        break;
    }
    case ColumnPlay: {
        playItem(index.row());
        break;
    }
    default:
        break;
    }
}

void PlaybackPictureList::onItemDoubleClicked(const QModelIndex &index)
{
    playItem(index.row());
}

void PlaybackPictureList::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    searchPicturePage(m_pageIndex + 1);
}

void PlaybackPictureList::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    searchPicturePage(m_pageIndex + 1);
}

void PlaybackPictureList::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    searchPicturePage(m_pageIndex + 1);
}

void PlaybackPictureList::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    searchPicturePage(m_pageIndex + 1);
}

void PlaybackPictureList::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;
    searchPicturePage(m_pageIndex + 1);
}

void PlaybackPictureList::on_pushButton_export_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_FILEEXPORT)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    //
    bool hasSelected = false;
    for (int row = 0; row < ui->treeView->rowCount(); ++row) {
        if (ui->treeView->isItemChecked(row)) {
            hasSelected = true;
            break;
        }
    }
    if (!hasSelected) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100057", "Please select at least one picture."));
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
    qint64 totalBytes = 0;
    QList<rep_export_picture_backup> exportList;
    for (int i = 0; i < ui->treeView->rowCount(); ++i) {
        if (!ui->treeView->isItemChecked(i)) {
            continue;
        }
        const resp_search_picture_backup &backup = ui->treeView->itemData(i, ColumnChannel, PictureInfoRole).value<resp_search_picture_backup>();
        totalBytes += backup.size;

        rep_export_picture_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_picture_backup));
        exportBackup.chnid = backup.chnid;
        exportBackup.sid = backup.sid;
        exportBackup.port = backup.port;
        exportBackup.pts = backup.pts;
        exportBackup.index = backup.index;
        exportBackup.resolution = pictureResolution;
        snprintf(exportBackup.pTime, sizeof(exportBackup.pTime), "%s", backup.pTime);
        snprintf(exportBackup.dev_name, sizeof(exportBackup.dev_name), "%s", deviceName.toStdString().c_str());
        snprintf(exportBackup.dev_path, sizeof(exportBackup.dev_path), "%s", directory.toStdString().c_str());

        exportList.append(exportBackup);
    }

    if (totalBytes > deviceFreeSize) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

    for (int i = 0; i < exportList.size(); ++i) {
        const rep_export_picture_backup &exportBackup = exportList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid = gDownload->availableId();
        auto_backup.chnid = exportBackup.chnid;
        auto_backup.pts = exportBackup.pts;
        auto_backup.snap_res = exportBackup.resolution;
        auto_backup.filetype = 3;
        auto_backup.filesize = 0;
        snprintf(auto_backup.pStartTime, sizeof(auto_backup.pStartTime), "%s", exportBackup.pTime);
        snprintf(auto_backup.dev_name, sizeof(auto_backup.dev_name), "%s", exportBackup.dev_name);
        snprintf(auto_backup.dev_path, sizeof(auto_backup.dev_path), "%s", exportBackup.dev_path);

        char fileName[128];
        //get_auto_backup_snaphots_filename(&auto_backup, fileName, sizeof(fileName));
        snprintf(auto_backup.filename, sizeof(auto_backup.filename), "%s", fileName);

        gDownload->appendItem(auto_backup);
    }
    ShowMessageBox(GET_TEXT("PLAYBACK/80135", "Please check download status in download progress interface. "));
}

void PlaybackPictureList::on_pushButton_exportAll_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_FILEEXPORT)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
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
    qint64 totalBytes = 0;
    QList<rep_export_picture_backup> exportList;
    for (int i = 0; i < ui->treeView->rowCount(); ++i) {
        const resp_search_picture_backup &backup = ui->treeView->itemData(i, ColumnChannel, PictureInfoRole).value<resp_search_picture_backup>();
        totalBytes += backup.size;

        rep_export_picture_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_picture_backup));
        exportBackup.chnid = backup.chnid;
        exportBackup.sid = backup.sid;
        exportBackup.port = backup.port;
        exportBackup.pts = backup.pts;
        exportBackup.index = backup.index;
        exportBackup.resolution = pictureResolution;
        snprintf(exportBackup.pTime, sizeof(exportBackup.pTime), "%s", backup.pTime);
        snprintf(exportBackup.dev_name, sizeof(exportBackup.dev_name), "%s", deviceName.toStdString().c_str());
        snprintf(exportBackup.dev_path, sizeof(exportBackup.dev_path), "%s", directory.toStdString().c_str());

        exportList.append(exportBackup);
    }

    if (totalBytes > deviceFreeSize) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

    for (int i = 0; i < exportList.size(); ++i) {
        const rep_export_picture_backup &exportBackup = exportList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid = gDownload->availableId();
        auto_backup.chnid = exportBackup.chnid;
        auto_backup.pts = exportBackup.pts;
        auto_backup.snap_res = exportBackup.resolution;
        auto_backup.filetype = 3;
        auto_backup.filesize = 0;
        snprintf(auto_backup.pStartTime, sizeof(auto_backup.pStartTime), "%s", exportBackup.pTime);
        snprintf(auto_backup.dev_name, sizeof(auto_backup.dev_name), "%s", exportBackup.dev_name);
        snprintf(auto_backup.dev_path, sizeof(auto_backup.dev_path), "%s", exportBackup.dev_path);

        char fileName[128];
        //get_auto_backup_snaphots_filename(&auto_backup, fileName, sizeof(fileName));
        snprintf(auto_backup.filename, sizeof(auto_backup.filename), "%s", fileName);

        gDownload->appendItem(auto_backup);
    }
    ShowMessageBox(GET_TEXT("PLAYBACK/80135", "Please check download status in download progress interface. "));
}

void PlaybackPictureList::on_toolButton_close_clicked()
{
    //
    m_backupList.clear();
    if (m_searchSid >= 0) {
        sendMessage(REQUEST_FLAG_SEARCH_PIC_BACKUP_CLOSE, &m_searchSid, sizeof(int));
        m_searchSid = -1;
    }
    //
    PicturePlay::instance()->clear();
    m_currentRow = -1;
    close();
}
