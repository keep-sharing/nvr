#include "PagePosSearch.h"
#include "ui_PagePosSearch.h"
#include "DynamicDisplayData.h"
#include "EventLoop.h"
#include "FileSize.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsMessage.h"
#include "MsWaitting.h"
#include "MyFileSystemDialog.h"
#include "myqt.h"
#include "progressdialog.h"
#include <qmath.h>

const int BackupIndexRole = Qt::UserRole + 100;

PagePosSearch::PagePosSearch(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PosSearch)
{
    ui->setupUi(this);
    ui->lineEditPosContent->setMaxLength(256);

    //table list
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("POS/130001", "POS No.");
    headerList << GET_TEXT("LOG/64005", "Time");
    headerList << GET_TEXT("POS/130024", "POS Information");
    headerList << GET_TEXT("PTZCONFIG/36022", "Play");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderChecked(bool)));
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnPlay, new ItemButtonDelegate(QPixmap(":/retrieve/retrieve/playbutton.png"), this));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortableForColumn(ColumnPlay, false);
    ui->tableView->setSortableForColumn(ColumnPos, SortFilterProxyModel::SortInt);
    //
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnPos, 100);
    ui->tableView->setColumnWidth(ColumnTime, 200);
    ui->tableView->setColumnWidth(ColumnInfo, 500);

    //slider
    connect(ui->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

    //page
    connect(ui->lineEditPage, SIGNAL(textEdited(QString)), this, SLOT(onPageEdited(QString)));

    //
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onRealPlaybackTime()));
    m_timer->setInterval(1000);

    //
    ui->toolButtonPos->setChecked(true);
    ui->commonVideo->setPosVisible(true);

    //
    m_progress = new ProgressDialog(this);
    connect(m_progress, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));

    ui->toolButtonPause->hide();

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_RETRIEVE, this);

    onLanguageChanged();
}

PagePosSearch::~PagePosSearch()
{
    delete ui;
}

void PagePosSearch::initializeData()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->dateEditStart->setDate(QDate::currentDate());
    ui->timeEditStart->setTime(QTime(0, 0));
    ui->dateEditEnd->setDate(QDate::currentDate());
    ui->timeEditEnd->setTime(QTime(23, 59, 59));
    ui->checkBoxGroupChannel->clearCheck();

    ui->checkBoxGroupChannel->setCountFromPosName(MAX_POS_CLIENT);
    ui->lineEditPosContent->clear();
}

bool PagePosSearch::isCloseable()
{
    return true;
}

bool PagePosSearch::isChangeable()
{
    return true;
}

void PagePosSearch::closePage()
{
    MsWaittingContainer wait(ui->commonVideo);
    waitForStopCommonBackup();
    waitForCloseCommonBackup();
    waitForClosePosBackup();

    m_posBackupList = QList<RespSearchPosBackup>();
}

bool PagePosSearch::canAutoLogout()
{
    bool ok = true;
    if (m_timer->isActive()) {
        ok = false;
    }
    return ok;
}

void PagePosSearch::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_POS_OPEN:
        ON_RESPONSE_FLAG_SEARCH_POS_OPEN(message);
        break;
    case RESPONSE_FLAG_SEARCH_POS_PAGE:
        ON_RESPONSE_FLAG_SEARCH_POS_PAGE(message);
        break;
    case RESPONSE_FLAG_SEARCH_POS_DETAILS:
        ON_RESPONSE_FLAG_SEARCH_POS_DETAILS(message);
        break;
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        break;
        //
    case RESPONSE_FLAG_SEARCH_COM_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_PLAY_COM_BACKUP:
        ON_RESPONSE_FLAG_PLAY_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_PLAYPAUSE_COM_BACKUP:
        ON_RESPONSE_FLAG_PLAYPAUSE_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_PLAYRESTART_COM_BACKUP:
        ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_PLAYSTOP_COM_BACKUP:
        ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_SEARCH_COM_BACKUP_CLOSE:
        ON_RESPONSE_FLAG_SEARCH_COM_BACKUP_CLOSE(message);
        break;
    case RESPONSE_FLAG_PLAYSEEK_BACKUP:
        ON_RESPONSE_FLAG_PLAYSEEK_BACKUP(message);
        break;
    case RESPONSE_FLAG_PLAY_COM_PICTURE:
        ON_RESPONSE_FLAG_PLAY_COM_PICTURE(message);
        break;
        //
    case RESPONSE_FLAG_GET_EXPORT_DISK:
        ON_RESPONSE_FLAG_GET_EXPORT_DISK(message);
        break;
    case RESPONSE_FLAG_SEARCH_POS_EXPORT:
        ON_RESPONSE_FLAG_SEARCH_POS_EXPORT(message);
        break;
    }
}

void PagePosSearch::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        break;
    }
}

void PagePosSearch::ON_RESPONSE_FLAG_SEARCH_POS_OPEN(MessageReceive *message)
{
    RespSearchPosBackup *backupArray = static_cast<RespSearchPosBackup *>(message->data);
    if (!backupArray) {
        gEventLoopExit(-1);
        return;
    }

    int count = message->size() / sizeof(RespSearchPosBackup);
    for (int i = 0; i < count; ++i) {
        m_posBackupList.append(backupArray[i]);
    }

    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_SEARCH_POS_PAGE(MessageReceive *message)
{
    RespSearchPosBackup *backupArray = static_cast<RespSearchPosBackup *>(message->data);
    if (!backupArray) {
        qMsWarning() << message;
        gEventLoopExit(-1);
        return;
    }

    int count = message->size() / sizeof(RespSearchPosBackup);
    for (int i = 0; i < count; ++i) {
        m_posBackupList.append(backupArray[i]);
    }

    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_SEARCH_POS_DETAILS(MessageReceive *message)
{
    m_detailPosText = QString((char *)message->data);
    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message)
{
    if (!m_progress->isVisible()) {
        return;
    }

    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        return;
    }

    m_progress->setProgressValue(progressinfo->percent);
    m_searchSid = progressinfo->searchid;
}

void PagePosSearch::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message)
{
    resp_search_common_backup *backupArray = (resp_search_common_backup *)message->data;
    if (!backupArray) {
        gEventLoopExit(-1);
        return;
    }
    if (backupArray->allCnt < 1) {
        gEventLoopExit(-2);
        return;
    }

    int count = message->size() / sizeof(struct resp_search_common_backup);
    m_playSid = backupArray->sid;
    for (int i = 0; i < count; ++i) {
        const resp_search_common_backup &backup = backupArray[i];
        const QDateTime &startDateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        const QDateTime &endDateTime = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
        if (i == 0) {
            m_currentStartTime = startDateTime;
        }
        if (i == count - 1) {
            m_currentEndTime = endDateTime;
        }
        qMsDebug() << QString("count: %1, index: %2, channel: %3, sid: %4, start time: %5, end time: %6")
                          .arg(count)
                          .arg(i)
                          .arg(backup.chnid)
                          .arg(backup.sid)
                          .arg(QString(backup.pStartTime))
                          .arg(QString(backup.pEndTime));
        m_playBackupList.append(backup);
    }
    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_PLAYPAUSE_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP_CLOSE(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_PLAYSEEK_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void PagePosSearch::ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message)
{
    gEventLoopExit(0);
    if (m_isPlaying) {
        return;
    }
    QPixmap pixmap = QPixmap::fromImage(message->image1);
    ui->commonVideo->showPixmap(pixmap);
}

void PagePosSearch::ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message)
{
    struct resp_usb_info *usb_info_list = (struct resp_usb_info *)message->data;
    int count = message->header.size / sizeof(struct resp_usb_info);

    m_exportDiskFreeSize = 0;
    for (int i = 0; i < count; ++i) {
        const resp_usb_info &usb_info = usb_info_list[i];
        if (usb_info.port == m_exportDiskPort && usb_info.type != DISK_TYPE_NAS) {
            m_exportDiskFreeSize = usb_info.free;
            break;
        }
    }
    m_waitForCheckDisk.exit();
}

void PagePosSearch::ON_RESPONSE_FLAG_SEARCH_POS_EXPORT(MessageReceive *message)
{
    if (!message->data) {
        m_waitForExport.exit(-1);
        return;
    }
    int result = (*((int *)message->data));
    m_waitForExport.exit(result);
}

void PagePosSearch::updateBackupTable()
{
    ui->labelPage->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_allCount).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));
    //
    ui->timeSlider->setRange(0, 0);
    m_currentRow = -1;
    ui->tableView->clearContent();

    ui->tableView->setRowCount(m_posBackupList.size());
    int row = 0;
    for (int i = 0; i < m_posBackupList.size(); ++i) {
        const auto &backup = m_posBackupList.at(i);
        //
        QString text(backup.data);
        //
        ui->tableView->setItemData(row, ColumnCheck, false, ItemCheckedRole);
        ui->tableView->setItemData(row, ColumnCheck, i, BackupIndexRole);
        ui->tableView->setItemIntValue(row, ColumnPos, backup.posId + 1);
        ui->tableView->setItemText(row, ColumnTime, backup.pTime);
        //表格无法很好的显示换行，这里把换行改成空格
        QString displayText = text.left(100).replace("\n", " ");
        ui->tableView->setItemText(row, ColumnInfo, displayText);
        //ToolTip最多显示30行
        int p = text.indexOf("\n");
        int count = 0;
        while (p >= 0) {
            count++;
            if (count > 29) {
                break;
            }
            p = text.indexOf("\n", p + 1);
        }
        if (count > 29) {
            ui->tableView->setItemToolTip(row, ColumnInfo, text.left(p).trimmed() + "\n...");
        } else {
            ui->tableView->setItemToolTip(row, ColumnInfo, text.trimmed());
        }
        row++;
    }
    ui->tableView->reorder();
    setCurrentBackupItem(0);
}

void PagePosSearch::setCurrentBackupItem(int row)
{
    //
    ui->tableView->selectRow(row);
    //
    waitForSearchCommonBackup();
    if (m_playBackupList.isEmpty()) {
        MessageBox::queuedInformation(GET_TEXT("DISK/92033", "No record files currently."));
        return;
    }
    //
    waitForSearchCommonBackupPicture();
    //
    updateBackupInfo();
}

void PagePosSearch::updateBackupInfo()
{
    const auto &playBackup = m_playBackupList.first();
    ui->commonVideo->showCurrentChannel(playBackup.chnid);
    m_currentStartTime = QDateTime::fromString(playBackup.pStartTime, "yyyy-MM-dd HH:mm:ss");
    m_currentEndTime = QDateTime::fromString(playBackup.pEndTime, "yyyy-MM-dd HH:mm:ss");
    m_currentTime = m_currentStartTime;
    ui->labelPlayTime->setText(playBackup.pStartTime);
    ui->timeSlider->setTimeRange(m_currentStartTime, m_currentEndTime);
    //
    const auto &row = ui->tableView->currentRow();
    const auto &posIndex = ui->tableView->itemData(row, ColumnCheck, BackupIndexRole).toInt();
    const auto &posBackup = m_posBackupList.at(posIndex);
    ui->labelInfo->setText(QString("%1%2").arg(GET_TEXT("POS/130036", "Time: ")).arg(posBackup.pTime));
    if (posBackup.dataSize > MAX_LEN_4096) {
        RespSearchMsfsDetails searchDetails;
        searchDetails.sid = posBackup.sid;
        searchDetails.index = m_pageIndex * 100 + posIndex;
        Q_UNUSED(searchDetails)
        gEventLoopExec();
        ui->plainTextEditPosInformation->setPlainText(m_detailPosText);
    } else {
        ui->plainTextEditPosInformation->setPlainText(posBackup.data);
    }
}

void PagePosSearch::clearCurrentInfo()
{
    ui->commonVideo->clearPos();
    ui->plainTextEditPosInformation->clear();
}

void PagePosSearch::waitForSearchCommonBackup()
{
    int row = ui->tableView->currentRow();
    const int &index = ui->tableView->itemData(row, ColumnCheck, BackupIndexRole).toInt();
    const auto &backup = m_posBackupList.at(index);

    QDateTime backupDateTime = QDateTime::fromString(backup.pTime, "yyyy-MM-dd HH:mm:ss");
    if (backup.receipt) {
        QDateTime receiptDateTime = QDateTime::fromString(backup.pHeadTime, "yyyy-MM-dd HH:mm:ss");
        m_currentStartTime = receiptDateTime.addSecs(-10);
    } else {
        m_currentStartTime = backupDateTime.addSecs(-10);
    }
    m_currentEndTime = backupDateTime.addSecs(10);
    m_currentTime = m_currentStartTime;
    ui->labelPlayTime->setText(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setTimeRange(m_currentStartTime, m_currentEndTime);
    ui->timeSlider->setCurrentDateTime(m_currentStartTime);

    m_playBackupList.clear();

    req_search_common_backup commonBackup;
    memset(&commonBackup, 0, sizeof(req_search_common_backup));
    MyQt::makeChannelMask(backup.chnid, commonBackup.chnMaskl, sizeof(commonBackup.chnMaskl));
    commonBackup.chnNum = 1;
    commonBackup.enType = FILE_TYPE_MAIN;
    commonBackup.enEvent = REC_EVENT_ALL;
    commonBackup.enState = SEG_STATE_ALL;
    snprintf(commonBackup.pStartTime, sizeof(commonBackup.pStartTime), "%s", m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(commonBackup.pEndTime, sizeof(commonBackup.pEndTime), "%s", m_currentEndTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP, &commonBackup, sizeof(commonBackup));
    gEventLoopExec();

#if 0
    if (m_playBackupList.isEmpty()) {
        commonBackup.enType = FILE_TYPE_SUB;
        sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP, &commonBackup, sizeof(commonBackup));
        gEventLoopExec();
    }
#endif

    m_streamType = commonBackup.enType;
}

void PagePosSearch::waitForPlayCommonBackup()
{
    m_currentTime = m_currentStartTime;

    struct rep_play_common_backup playinfo;
    memset(&playinfo, 0, sizeof(struct rep_play_common_backup));
    playinfo.chnid = m_playChannel;
    playinfo.sid = m_playSid;
    snprintf(playinfo.pStartTime, sizeof(playinfo.pStartTime), "%s", m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(playinfo.pEndTime, sizeof(playinfo.pEndTime), "%s", m_currentEndTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(playinfo.pPlayTime, sizeof(playinfo.pPlayTime), "%s", m_currentTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    qMsDebug() << QString("REQUEST_FLAG_PLAY_COM_BACKUP, chnid: %1, sid: %2, pStartTime: %3, pEndTime: %4, pPlayTime: %5")
                      .arg(playinfo.chnid)
                      .arg(playinfo.sid)
                      .arg(playinfo.pStartTime)
                      .arg(playinfo.pEndTime)
                      .arg(playinfo.pPlayTime);
    sendMessage(REQUEST_FLAG_PLAY_COM_BACKUP, (void *)&playinfo, sizeof(struct rep_play_common_backup));
    gEventLoopExec();
}

void PagePosSearch::waitForPauseCommonBackup()
{
    if (m_playSid < 0) {
        return;
    }
    m_timer->stop();
    ui->commonVideo->setPosPaused(true);
    sendMessage(REQUEST_FLAG_PLAYPAUSE_COM_BACKUP, &m_playSid, sizeof(int));
    gEventLoopExec();
}

void PagePosSearch::waitForResumeCommonBackup()
{
    if (m_playSid < 0) {
        return;
    }
    sendMessage(REQUEST_FLAG_PLAYRESTART_COM_BACKUP, &m_playSid, sizeof(int));
    gEventLoopExec();
}

void PagePosSearch::waitForStopCommonBackup()
{
    if (!m_isPlaying) {
        return;
    }

    sendMessage(REQUEST_FLAG_PLAYSTOP_COM_BACKUP, &m_playSid, sizeof(int));
    gEventLoopExec();

    m_timer->stop();
    ui->commonVideo->setPosPaused(true);
    ui->labelPlayTime->setText(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
    m_currentTime = m_currentStartTime;
    ui->timeSlider->setCurrentDateTime(m_currentTime);

    ui->toolButtonPlay->show();
    ui->toolButtonPause->hide();

    m_isPlaying = false;
}

void PagePosSearch::waitForCloseCommonBackup()
{
    if (m_playSid < 0) {
        return;
    }

    sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP_CLOSE, &m_playSid, sizeof(m_playSid));
    gEventLoopExec();

    m_playSid = -1;
}

void PagePosSearch::waitForSeekCommonBackup(const QDateTime &dateTime)
{
    struct req_seek_time seekinfo;
    memset(&seekinfo, 0, sizeof(struct req_seek_time));
    seekinfo.chnid = m_playChannel;
    seekinfo.sid = m_playSid;
    snprintf(seekinfo.pseektime, sizeof(seekinfo.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_PLAYSEEK_BACKUP, &seekinfo, sizeof(seekinfo));
    gEventLoopExec();
}

void PagePosSearch::waitForSearchCommonBackupPicture()
{
    if (m_playBackupList.isEmpty()) {
        return;
    }

    const auto &backup = m_playBackupList.first();

    struct rep_play_common_backup playInfo;
    memset(&playInfo, 0, sizeof(struct rep_play_common_backup));
    playInfo.chnid = backup.chnid;
    playInfo.sid = backup.sid;
    playInfo.flag = 1;
    playInfo.enType = m_streamType;
    snprintf(playInfo.pStartTime, sizeof(playInfo.pStartTime), "%s", backup.pStartTime);
    snprintf(playInfo.pEndTime, sizeof(playInfo.pEndTime), "%s", backup.pEndTime);
    qMsDebug() << QString("REQUEST_FLAG_PLAY_COM_PICTURE, channel: %1, sid: %2, start time: %3, end time: %4")
                      .arg(backup.chnid)
                      .arg(backup.sid)
                      .arg(QString(backup.pStartTime))
                      .arg(QString(backup.pEndTime));
    sendMessage(REQUEST_FLAG_PLAY_COM_PICTURE, &playInfo, sizeof(playInfo));
    gEventLoopExec();
}

/**
 * @brief PosSearch::swaitForSearchPosPage
 * @param page: 1-n
 */
void PagePosSearch::waitForSearchPosPage(int page)
{
    m_posBackupList.clear();

    struct rep_get_search_backup searchPage;
    memset(&searchPage, 0, sizeof(struct rep_get_search_backup));
    searchPage.sid = m_backupSid;
    searchPage.npage = page;
    qMsDebug() << QString("REQUEST_FLAG_SEARCH_POS_PAGE, %1/%2").arg(searchPage.npage).arg(m_pageCount);
    sendMessage(REQUEST_FLAG_SEARCH_POS_PAGE, &searchPage, sizeof(struct rep_get_search_backup));
    gEventLoopExec();
}

void PagePosSearch::waitForClosePosBackup()
{
    if (!m_posBackupList.isEmpty()) {
        int sid = m_posBackupList.first().sid;
        Q_UNUSED(sid)
    }
}

void PagePosSearch::backupFile(const QList<int> &backupList)
{
    //
    const QString &strPath = MyFileSystemDialog::instance()->exportPos();
    if (strPath.isEmpty()) {
        qDebug() << QString("AnprAnalysis::backupFile, path is empty.");
        return;
    }

    //const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    //const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeSize();
    const int &posExportType = MyFileSystemDialog::instance()->exportPosFileType();
    //const int &streamType = MyFileSystemDialog::instance()->streamType();
    const int &fileType = MyFileSystemDialog::instance()->fileType();
    m_exportDiskPort = MyFileSystemDialog::instance()->currentDevicePort();

    //检查剩余空间
    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    m_waitForCheckDisk.exec();
    if (m_exportDiskFreeSize < MIN_EXPORT_DISK_FREE) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

    m_isExporting = true;
    m_progress->showProgress();
    //
    if (m_posBackupList.isEmpty()) {
        qMsWarning() << QString("m_posBackupList is empty.");
        m_progress->hideProgress();
        ShowMessageBox(QString("Download failed."));
        return;
    }
    const auto &firstInfo = m_posBackupList.first();
    QString posListName = QString("%1_POS List_%2.csv").arg(qMsNvr->deviceName()).arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    //
    qMsDebug() << QString("begin, size: %1").arg(backupList.size());
    int complatedCount = 0;
    int result = 0;
    for (int i = 0; i < backupList.size(); ++i) {
        if (m_isExportCancel) {
            result |= 2;
            qMsDebug() << QString("cancel");
            break;
        }

        m_progress->setProgressValue((qreal)complatedCount / backupList.size() * 100);

        RepExportPosBackup exportPos;
        memset(&exportPos, 0, sizeof(exportPos));
        exportPos.sid = firstInfo.sid;
        exportPos.index = backupList.at(i);
        exportPos.exportType = posExportType;
        exportPos.streamType = FILE_TYPE_MAIN;
        exportPos.avifileType = fileType;
        snprintf(exportPos.logfile_name, sizeof(exportPos.logfile_name), "%s", posListName.toStdString().c_str());
        snprintf(exportPos.dev_path, sizeof(exportPos.dev_path), "%s", strPath.toStdString().c_str());
        sendMessage(REQUEST_FLAG_SEARCH_POS_EXPORT, (void *)&exportPos, sizeof(exportPos));

        int backResult = m_waitForExport.exec();
        complatedCount++;

        //
        if (backResult == 0) {
            result |= SMARTANALYSIS_BACKUP_SUCCESS;
        } else if (backResult == posExportType) {
            result |= SMARTANALYSIS_BACKUP_FAILED;
        } else {
            result |= SMARTANALYSIS_BACKUP_SOMESUCCESS;
        }
    }
    qDebug() << QString("end");

    //
    m_isExporting = false;
    m_isExportCancel = false;
    m_progress->hideProgress();
    if (result == SMARTANALYSIS_BACKUP_SUCCESS) {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145000", "Backup successfully."));
    } else if (result == SMARTANALYSIS_BACKUP_SOMESUCCESS) {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145001", "Failed to backup some results."));
    } else {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145002", "Failed to backup."));
    }
}

void PagePosSearch::onLanguageChanged()
{
    ui->labelPos->setText(GET_TEXT("POS/130001", "POS No."));
    ui->labelPosContent->setText(GET_TEXT("POS/130034", "POS Content"));
    ui->labelStartTime->setText(GET_TEXT("IMAGE/37320", "Start Time"));
    ui->labelEndTime->setText(GET_TEXT("IMAGE/37321", "End Time"));
    ui->pushButtonSearch->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->toolButtonPos->setToolTip(GET_TEXT("POS/130024", "POS Information"));
    ui->toolButtonStop->setToolTip(GET_TEXT("COMMONBACKUP/100044", "Stop"));
    ui->toolButtonPlay->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
    ui->toolButtonPause->setToolTip(GET_TEXT("COMMONBACKUP/100046", "Pause"));
    ui->toolButtonPrevious->setToolTip(GET_TEXT("COMMONBACKUP/100047", "Previous"));
    ui->toolButtonNext->setToolTip(GET_TEXT("COMMONBACKUP/100048", "Next"));

    ui->toolButtonFirstPage->setToolTip(GET_TEXT("PLAYBACK/80061", "First Page"));
    ui->toolButtonPreviousPage->setToolTip(GET_TEXT("PLAYBACK/80062", "Previous Page"));
    ui->toolButtonNextPage->setToolTip(GET_TEXT("PLAYBACK/80063", "Next Page"));
    ui->toolButtonLastPage->setToolTip(GET_TEXT("PLAYBACK/80064", "Last Page"));

    ui->labelPosInformation->setText(GET_TEXT("POS/130035", "POS Information: "));
    ui->pushButtonBackupAll->setText(GET_TEXT("COMMONBACKUP/100002", "Backup All"));
    ui->pushButtonBackup->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->pushButtonResultBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PagePosSearch::on_pushButtonSearch_clicked()
{
    ui->pushButtonSearch->clearUnderMouse();
    return;

    if (!ui->checkBoxGroupChannel->hasChannelSelected()) {
        MessageBox::information(this, GET_TEXT("POS/130025", "Please select at least one POS."));
        return;
    }

    const QDateTime startDateTime(ui->dateEditStart->date(), ui->timeEditStart->time());
    const QDateTime endDateTime(ui->dateEditEnd->date(), ui->timeEditEnd->time());
    if (startDateTime > endDateTime) {
        MessageBox::information(this, GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }

    //
    m_isAbaoutToQuit = false;
    m_posBackupList.clear();
    m_backupSid = -1;

    m_progress->showProgress();

    ReqPosEvtSearch search;
    search.posId = ui->checkBoxGroupChannel->checkedFlags();
    search.receipt = 0;
    snprintf(search.content, sizeof(search.content), ui->lineEditPosContent->text().toLocal8Bit().data());
    snprintf(search.startTime, sizeof(search.startTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(search.endTime, sizeof(search.endTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_SEARCH_POS_OPEN, &search, sizeof(search));
    int result = gEventLoopExec();
    if (result < 0) {
        m_progress->hideProgress();
        MessageBox::information(this, GET_TEXT("ANPR/103075", "No matching items."));
        return;
    }

    //page
    const RespSearchPosBackup &firstBackup = m_posBackupList.first();
    m_allCount = firstBackup.allCnt;
    m_pageCount = qCeil(m_allCount / 100.0);
    m_pageIndex = 0;
    m_backupSid = firstBackup.sid;
    m_progress->hideProgress();

    //
    if (m_allCount >= MAX_SEARCH_BACKUP_COUNT) {
        MessageBox::queuedInformation(GET_TEXT("COMMONBACKUP/100042", "The search results is over %1, and the first %1 will be displayed.").arg(MAX_SEARCH_BACKUP_COUNT));
    }

    //
    ui->labelPage->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_allCount).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEditPage->setMaxPage(m_pageCount);
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    //
    ui->stackedWidget->setCurrentIndex(1);
    updateBackupTable();
}

void PagePosSearch::on_pushButtonBack_clicked()
{
    back();
}

void PagePosSearch::onItemClicked(int row, int column)
{
    Q_UNUSED(row)

    MsWaittingContainer wait(ui->tableView);
    clearCurrentInfo();
    waitForStopCommonBackup();
    waitForCloseCommonBackup();
    //
    waitForSearchCommonBackup();
    if (m_playBackupList.isEmpty()) {
        MessageBox::queuedInformation(GET_TEXT("DISK/92033", "No record files currently."));
        return;
    }
    //
    waitForSearchCommonBackupPicture();
    //
    updateBackupInfo();
    //
    switch (column) {
    case ColumnCheck: {
        break;
    }
    case ColumnPlay: {
        on_toolButtonPlay_clicked();
        break;
    }
    default:
        break;
    }
}

void PagePosSearch::onTableHeaderChecked(bool checked)
{
    Q_UNUSED(checked)
}

void PagePosSearch::onSearchCanceled()
{
    if (m_isExporting) {
        m_isExportCancel = true;
        return;
    }

    if (m_searchSid >= 0) {
        sendMessageOnly(REQUEST_FLAG_SEARCH_POS_CANCEL, (void *)&m_searchSid, sizeof(m_searchSid));
        m_searchSid = -1;
    }
}

void PagePosSearch::onRealPlaybackTime()
{
    m_currentTime = m_currentTime.addSecs(1);
    if (m_currentTime >= m_currentEndTime) {
        m_currentTime = m_currentEndTime;
        on_toolButtonPause_clicked();
    }
    ui->labelPlayTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setCurrentDateTime(m_currentTime);
}

void PagePosSearch::onSliderValueChanged(int value)
{
    m_currentTime = QDateTime::fromTime_t(value);
    ui->labelPlayTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));

    if (m_playSid < 0) {
        return;
    }

    waitForSeekCommonBackup(m_currentTime);
}

void PagePosSearch::on_toolButtonPos_clicked(bool checked)
{
    ui->commonVideo->setPosVisible(checked);
}

void PagePosSearch::on_toolButtonStop_clicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    waitForStopCommonBackup();
    ui->commonVideo->showPixmap();

    ui->toolButtonPlay->show();
    ui->toolButtonPause->hide();
}

void PagePosSearch::on_toolButtonPlay_clicked()
{
    if (m_playSid < 0) {
        return;
    }

    MsWaittingContainer wait(ui->commonVideo);

    if (m_isPlaying) {
        if (m_currentTime >= m_currentEndTime) {
            m_currentTime = m_currentStartTime;
            waitForSeekCommonBackup(m_currentTime);
        }
        waitForResumeCommonBackup();
    } else {
        const resp_search_common_backup &backup = m_playBackupList.first();
        m_playChannel = backup.chnid;
        ui->commonVideo->hidePixmap();
        ui->commonVideo->playbackVideo(m_playChannel);
        waitForPlayCommonBackup();
    }

    ui->toolButtonPlay->hide();
    ui->toolButtonPause->show();
    m_timer->start();
    m_isPlaying = true;
    ui->commonVideo->setPosPaused(false);
}

void PagePosSearch::on_toolButtonPause_clicked()
{
    if (m_playSid < 0) {
        return;
    }

    MsWaittingContainer wait(ui->commonVideo);

    waitForPauseCommonBackup();

    ui->toolButtonPlay->show();
    ui->toolButtonPause->hide();
}

void PagePosSearch::on_toolButtonPrevious_clicked()
{
    int row = ui->tableView->currentRow();
    if (row > 0) {
        row--;
        //
        MsWaittingContainer wait(ui->commonVideo);
        waitForStopCommonBackup();
        waitForCloseCommonBackup();
        setCurrentBackupItem(row);
    }
}

void PagePosSearch::on_toolButtonNext_clicked()
{
    int row = ui->tableView->currentRow();
    if (row < ui->tableView->rowCount() - 1) {
        row++;
        //
        MsWaittingContainer wait(ui->commonVideo);
        waitForStopCommonBackup();
        waitForCloseCommonBackup();
        setCurrentBackupItem(row);
    }
}

void PagePosSearch::onPageEdited(const QString &text)
{
    Q_UNUSED(text)
}

void PagePosSearch::on_toolButtonFirstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    MsWaittingContainer wait(this);
    waitForStopCommonBackup();
    waitForCloseCommonBackup();
    waitForSearchPosPage(m_pageIndex + 1);
    updateBackupTable();
}

void PagePosSearch::on_toolButtonPreviousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    MsWaittingContainer wait(this);
    waitForStopCommonBackup();
    waitForCloseCommonBackup();
    waitForSearchPosPage(m_pageIndex + 1);
    updateBackupTable();
}

void PagePosSearch::on_toolButtonNextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    MsWaittingContainer wait(this);
    waitForStopCommonBackup();
    waitForCloseCommonBackup();
    waitForSearchPosPage(m_pageIndex + 1);
    updateBackupTable();
}

void PagePosSearch::on_toolButtonLastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    MsWaittingContainer wait(this);
    waitForStopCommonBackup();
    waitForCloseCommonBackup();
    waitForSearchPosPage(m_pageIndex + 1);
    updateBackupTable();
}

void PagePosSearch::on_pushButtonGo_clicked()
{
    int page = ui->lineEditPage->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    if (m_pageIndex == page - 1) {
        return;
    }
    m_pageIndex = page - 1;
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    MsWaittingContainer wait(this);
    waitForStopCommonBackup();
    waitForCloseCommonBackup();
    waitForSearchPosPage(m_pageIndex + 1);
    updateBackupTable();
}

void PagePosSearch::on_pushButtonBackupAll_clicked()
{
    //逐行加入列表，导出全部pos信息
    QList<int> list;
    for (int row = 0; row < m_allCount; ++row) {
        list.append(row);
    }
    backupFile(list);
    MyFileSystemDialog::instance()->clearAnprSelected();
}

void PagePosSearch::on_pushButtonBackup_clicked()
{
    QList<int> list;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (ui->tableView->isItemChecked(row, ColumnCheck)) {
            int index = m_pageIndex * 100 + row;
            list.append(index);
        }
    }
    if (list.isEmpty()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        return;
    }

    backupFile(list);
    MyFileSystemDialog::instance()->clearAnprSelected();
}

void PagePosSearch::on_pushButtonResultBack_clicked()
{
    clearCurrentInfo();

    MsWaittingContainer wait(ui->commonVideo);
    waitForStopCommonBackup();
    waitForCloseCommonBackup();
    waitForClosePosBackup();
    ui->stackedWidget->setCurrentIndex(0);
}
