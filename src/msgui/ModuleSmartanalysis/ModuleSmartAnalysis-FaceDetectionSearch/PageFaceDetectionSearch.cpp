#include "PageFaceDetectionSearch.h"
#include "ui_PageFaceDetectionSearch.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileModel.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"
#include "myqt.h"
#include <QDir>
#include <QElapsedTimer>
#include <qmath.h>

PageFaceDetectionSearch::PageFaceDetectionSearch(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::FaceDetectionSearch)
{
    ui->setupUi(this);

    ui->checkBoxGroupChannel->setCountFromChannelName(qMsNvr->maxChannel());
    connect(ui->checkBoxGroupChannel, SIGNAL(checkBoxClicked()), this, SLOT(onChannelCheckBoxClicked()));

    ui->comboBoxGender->beginEdit();
    ui->comboBoxGender->clear();
    ui->comboBoxGender->addItem(GET_TEXT("COMMON/1006", "All"), 2);
    ui->comboBoxGender->addItem(GET_TEXT("FACE/141043", "Male"), 0);
    ui->comboBoxGender->addItem(GET_TEXT("FACE/141044", "Female"), 1);
    ui->comboBoxGender->addItem(GET_TEXT("FACE/141055", "N/A"), -1);
    ui->comboBoxGender->endEdit();

    ui->comboBoxAge->beginEdit();
    ui->comboBoxAge->clear();
    ui->comboBoxAge->addItem(GET_TEXT("COMMON/1006", "All"), 3);
    ui->comboBoxAge->addItem(GET_TEXT("FACE/141040", "Child"), 0);
    ui->comboBoxAge->addItem(GET_TEXT("FACE/141041", "Adult"), 1);
    ui->comboBoxAge->addItem(GET_TEXT("FACE/141042", "Elderly"), 2);
    ui->comboBoxAge->addItem(GET_TEXT("FACE/141055", "N/A"), -1);
    ui->comboBoxAge->endEdit();

    ui->comboBoxGlasses->beginEdit();
    ui->comboBoxGlasses->clear();
    ui->comboBoxGlasses->addItem(GET_TEXT("COMMON/1006", "All"), 2);
    ui->comboBoxGlasses->addItem(GET_TEXT("FACE/141048", "Yes"), 1);
    ui->comboBoxGlasses->addItem(GET_TEXT("FACE/141045", "No"), 0);
    ui->comboBoxGlasses->addItem(GET_TEXT("FACE/141055", "N/A"), -1);
    ui->comboBoxGlasses->endEdit();

    ui->comboBoxMask->beginEdit();
    ui->comboBoxMask->clear();
    ui->comboBoxMask->addItem(GET_TEXT("COMMON/1006", "All"), 2);
    ui->comboBoxMask->addItem(GET_TEXT("FACE/141048", "Yes"), 1);
    ui->comboBoxMask->addItem(GET_TEXT("FACE/141045", "No"), 0);
    ui->comboBoxMask->addItem(GET_TEXT("FACE/141055", "N/A"), -1);
    ui->comboBoxMask->endEdit();

    ui->comboBoxCap->beginEdit();
    ui->comboBoxCap->clear();
    ui->comboBoxCap->addItem(GET_TEXT("COMMON/1006", "All"), 2);
    ui->comboBoxCap->addItem(GET_TEXT("FACE/141048", "Yes"), 1);
    ui->comboBoxCap->addItem(GET_TEXT("FACE/141045", "No"), 0);
    ui->comboBoxCap->addItem(GET_TEXT("FACE/141055", "N/A"), -1);
    ui->comboBoxCap->endEdit();

    //
    for (int row = 0; row < 4; ++row) {
        ui->gridLayout_item->setRowStretch(row, 1);
    }
    for (int column = 0; column < 5; ++column) {
        ui->gridLayout_item->setColumnStretch(column, 1);
    }
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 5; ++column) {
            int index = row * 5 + column;
            AnprItemWidget *item = new AnprItemWidget(this);
            item->setIndex(index);
            item->setInfoVisible(false);
            item->setIsFace(true);
            connect(item, SIGNAL(itemClicked(int)), this, SLOT(onItemClicked(int)));
            connect(item, SIGNAL(itemChecked(int, bool)), this, SLOT(onItemChecked(int, bool)));
            ui->gridLayout_item->addWidget(item, row, column);

            m_itemList.append(item);
        }
    }
    ui->labelPlayTime->clear();
    //slider
    connect(ui->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onRealPlaybackTime()));
    m_timer->setInterval(1000);

    m_progress = new ProgressDialog(this);
    connect(m_progress, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_RETRIEVE, this);

    onLanguageChanged();
}

PageFaceDetectionSearch::~PageFaceDetectionSearch()
{
    delete ui;
}

void PageFaceDetectionSearch::initializeData()
{
    ui->stackedWidget->setCurrentIndex(0);

    ui->dateEditStart->setDate(QDate::currentDate());
    ui->timeEditStart->setTime(QTime(0, 0, 0));
    ui->dateEditEnd->setDate(QDate::currentDate());
    ui->timeEditEnd->setTime(QTime(23, 59, 59));

    ui->checkBoxGroupChannel->clearCheck();
    ui->comboBoxAge->setCurrentIndex(0);
    ui->comboBoxGender->setCurrentIndex(0);
    ui->comboBoxGlasses->setCurrentIndex(0);
    ui->comboBoxMask->setCurrentIndex(0);
    ui->comboBoxCap->setCurrentIndex(0);
}

bool PageFaceDetectionSearch::isCloseable()
{
    if (m_isExporting) {
        return false;
    } else {
        return true;
    }
}

void PageFaceDetectionSearch::closePage()
{
    m_isAbaoutToQuit = true;

    stopAndClearCommonPlay();
    clearTable();
    closeFaceSearch();
}

NetworkResult PageFaceDetectionSearch::dealNetworkCommond(const QString &commond)
{
    if (ui->stackedWidget->currentIndex() == 0) {
        return NetworkReject;
    }

    qDebug() << "FaceDetectionSearch::dealNetworkCommond," << commond;

    NetworkResult result = NetworkReject;
    if (commond.startsWith("Video_Play")) {
        const QString &state = ui->toolButtonPlay->property("state").toString();
        if (state == "play") {
            QMetaObject::invokeMethod(this, "on_toolButtonPlay_clicked", Qt::QueuedConnection);
            result = NetworkAccept;
        }
    } else if (commond.startsWith("Video_Pause")) {
        const QString &state = ui->toolButtonPlay->property("state").toString();
        if (state == "pause") {
            QMetaObject::invokeMethod(this, "on_toolButtonPlay_clicked", Qt::QueuedConnection);
            result = NetworkAccept;
        }
    } else if (commond.startsWith("Video_Stop")) {
        QMetaObject::invokeMethod(this, "on_toolButton_stop_clicked", Qt::QueuedConnection);
        result = NetworkAccept;
    } else if (commond.startsWith("Dial_Insid_Add")) {
        //选中下一个
        QMetaObject::invokeMethod(this, "on_toolButton_next_clicked", Qt::QueuedConnection);
        result = NetworkAccept;
    } else if (commond.startsWith("Dial_Insid_Sub")) {
        //选中上一个
        QMetaObject::invokeMethod(this, "on_toolButton_previous_clicked", Qt::QueuedConnection);
        result = NetworkAccept;
    }

    return result;
}

void PageFaceDetectionSearch::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_FACE_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP(message);
        break;
    case RESPONSE_FLAG_EXPORT_FACE_BACKUP:
        ON_RESPONSE_FLAG_EXPORT_FACE_BACKUP(message);
        break;
    case RESPONSE_FLAG_SEARCH_FACE_BACKUP_PAGE:
        ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP_PAGE(message);
        break;
    case RESPONSE_FLAG_SEARCH_FACE_BACKUP_BIGIMG:
        ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP_BIGIMG(message);
        break;
    case RESPONSE_FLAG_SEARCH_COM_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_PLAY_COM_BACKUP:
        ON_RESPONSE_FLAG_PLAY_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_PLAYRESTART_COM_BACKUP:
        ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_PLAYSTOP_COM_BACKUP:
        ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_GET_EXPORT_DISK:
        ON_RESPONSE_FLAG_GET_EXPORT_DISK(message);
        break;
    }
}

void PageFaceDetectionSearch::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        break;
    }
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP(MessageReceive *message)
{
    qDebug() << "====FaceDetection::ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP, begin====";

    //m_progressSid = -1;
    //m_progress->hideProgress();

    resp_search_face_backup *faceBackup = static_cast<resp_search_face_backup *>(message->data);
    if (!faceBackup) {
        qDebug() << "----data:" << faceBackup;
        ShowMessageBox(GET_TEXT("ANPR/103075", "No matching items."));
        return;
    }
    qDebug() << "----index:" << faceBackup->index;
    qDebug() << "----allCnt:" << faceBackup->allCnt;
    qDebug() << "----bImageSize:" << faceBackup->bImgSize;
    qDebug() << "----sImageSize:" << faceBackup->sImgSize;

    FaceBackupInfo info(message);
    m_faceMap.insert(faceBackup->index, info);

    //image
    QImage sImg = QImage::fromData(static_cast<uchar *>(message->data) + sizeof(resp_search_face_backup) + faceBackup->bImgSize, faceBackup->sImgSize);

    //page
    m_pageCount = qCeil(faceBackup->allCnt / 20.0);
    ui->labelPage->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(faceBackup->allCnt).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEditPage->setMaxPage(m_pageCount);
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    ui->stackedWidget->setCurrentIndex(1);
    m_allCount = faceBackup->allCnt;
    m_faceSearchSid = faceBackup->sid;
    updateTable(m_searchedCount, sImg);
    ui->commonVideo->showPixmap(QPixmap::fromImage(message->image1));
    m_searchedCount++;
    if (m_allCount > 1) {
        rep_get_search_backup search_backup;
        search_backup.sid = faceBackup->sid;
        search_backup.npage = faceBackup->index + 1;
        Q_UNUSED(search_backup)

        //showWait();
    } else {
        QMetaObject::invokeMethod(this, "onItemClicked", Qt::QueuedConnection, Q_ARG(int, 0));
    }
    m_isAbaoutToQuit = false;
    m_isAboutToShowMessage = true;
    qDebug() << "====FaceDetection::ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP, end====";
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_EXPORT_FACE_BACKUP(MessageReceive *message)
{
    if (!message->data) {
        m_waitForExport.exit(-1);
        return;
    }
    int result = *static_cast<int *>(message->data);
    m_waitForExport.exit(result);
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP_PAGE(MessageReceive *message)
{
    resp_search_face_backup *faceBackup = static_cast<resp_search_face_backup *>(message->data);
    //image
    QImage sImg = QImage::fromData(static_cast<uchar *>(message->data) + sizeof(resp_search_face_backup) + faceBackup->bImgSize, faceBackup->sImgSize);
    qDebug() << "====FaceDetection::ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP_PAGE====";
    qDebug() << "----index:" << faceBackup->index;
    qDebug() << "----allCnt:" << faceBackup->allCnt;
    qDebug() << "----bImageSize:" << faceBackup->bImgSize;
    qDebug() << "----sImageSize:" << faceBackup->sImgSize;

    m_faceMap.insert(faceBackup->index, FaceBackupInfo(message));

    updateTable(m_searchedCount, sImg);
    m_searchedCount++;
    if (m_searchedCount >= 20 || faceBackup->index + 1 >= m_allCount) {
        QMetaObject::invokeMethod(this, "onItemClicked", Qt::QueuedConnection, Q_ARG(int, 0));
    } else {
        rep_get_search_backup search_backup;
        search_backup.sid = faceBackup->sid;
        search_backup.npage = faceBackup->index + 1;
        Q_UNUSED(search_backup)
    }
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP_BIGIMG(MessageReceive *message)
{
    resp_search_face_backup *faceBackup = static_cast<resp_search_face_backup *>(message->data);
    if (!faceBackup) {
        qDebug() << QString("FaceDetection::ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP_BIGIMG, data is null");
        ui->commonVideo->hidePixmap();
        m_waitForSearch.exit(-1);
        return;
    }
    qDebug() << QString("FaceDetection::ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP_BIGIMG, index: %1, all count: %2, size: %3").arg(faceBackup->index).arg(faceBackup->allCnt).arg(faceBackup->bImgSize);
    QImage bImg = QImage::fromData(static_cast<uchar *>(message->data) + sizeof(resp_search_face_backup), faceBackup->bImgSize);
    qDebug() << "gjst" << bImg.bits();
    ui->commonVideo->showPixmap(QPixmap::fromImage(bImg));
    m_waitForSearch.exit(0);
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message)
{
    qDebug() << QString("FaceDetection::ON_RESPONSE_FLAG_GET_EXPORT_DISK");
    struct resp_usb_info *usb_info_list = static_cast<struct resp_usb_info *>(message->data);
    int count = message->header.size / static_cast<int>(sizeof(struct resp_usb_info));
    m_exportDiskFreeSize = 0;
    for (int i = 0; i < count; ++i) {
        const resp_usb_info &usb_info = usb_info_list[i];
        if (usb_info.port == m_exportDiskPort && usb_info.type != DISK_TYPE_NAS) {
            m_exportDiskFreeSize = static_cast<int>(usb_info.free);
            break;
        }
    }
    m_waitForCheckDisk.exit();
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message)
{
    if (m_isExporting) {
        return;
    }

    if (!m_progress->isVisible()) {
        return;
    }

    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        qWarning() << "FaceDetection::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, data is null.";
        return;
    }
    qDebug() << QString("FaceDetection::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, %1").arg(progressinfo->percent);

    //m_progress->setProgressValue(progressinfo->percent);
    //m_progressSid = progressinfo->searchid;
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message)
{
    resp_search_common_backup *common_backup_array = static_cast<resp_search_common_backup *>(message->data);
    int count = message->header.size / static_cast<int>(sizeof(struct resp_search_common_backup));

    //
    if (!common_backup_array) {
        qWarning() << QString("FaceDetection::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, resp_search_common_backup is null.");
    } else {
        if (common_backup_array->allCnt == 0) {
            qDebug() << QString("FaceDetection::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, resp_search_common_backup is empty.");
        } else {
            for (int i = 0; i < count; ++i) {
                const resp_search_common_backup &common_backup = common_backup_array[i];
                const QDateTime &startDateTime = QDateTime::fromString(common_backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
                const QDateTime &endDateTime = QDateTime::fromString(common_backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
                if (i == 0) {
                    m_currentStartTime = startDateTime;
                }
                if (i == count - 1) {
                    m_currentEndTime = endDateTime;
                }
                qDebug() << QString("FaceDetection::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, count: %1, index: %2, channel: %3, sid: %4, start time: %5, end time: %6")
                                .arg(count)
                                .arg(i)
                                .arg(common_backup.chnid)
                                .arg(common_backup.sid)
                                .arg(QString(common_backup.pStartTime))
                                .arg(QString(common_backup.pEndTime));
                m_backupList.append(common_backup);
            }
        }
    }

    if (m_backupList.isEmpty()) {
        m_waitForSearch.exit(-1);
    } else {
        ui->labelPlayTime->setText(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
        ui->timeSlider->setTimeRange(m_currentStartTime, m_currentEndTime);
        m_currentTime = m_currentStartTime;

        //
        m_currentBackupSid = m_backupList.first().sid;
        m_waitForSearch.exit(0);
    }
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "FaceDetection::ON_RESPONSE_FLAG_PLAY_COM_BACKUP, data is null.";
        return;
    }
    int playinfo = *static_cast<int *>(message->data);
    switch (playinfo) {
    case -2:
        //搜索后磁盘被移除了等导致录像不存在。
        ShowMessageBox("Stream is invalid.");
        return;
    }

    m_timer->start();
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message);

    m_timer->start();
}

void PageFaceDetectionSearch::ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)

    m_waitForStopPlay.exit();
}

void PageFaceDetectionSearch::showWait()
{
    AbstractSettingPage::showWait(ui->widgetItemContent);
}

void PageFaceDetectionSearch::showVideoWait()
{
    AbstractSettingPage::showWait(ui->commonVideo);
}

void PageFaceDetectionSearch::updateVideoInfo()
{
    qDebug() << QString("FaceDetection::updateVideoInfo, begin");

    if (!m_faceMap.contains(m_currentIndex)) {
        qDebug() << QString("FaceDetection::updateVideoInfo, end, error index: %1").arg(m_currentIndex);
        return;
    }

    const FaceBackupInfo &info = m_faceMap.value(m_currentIndex);

    m_currentChannel = info.channel;
    ui->commonVideo->showCurrentChannel(m_currentChannel);

    m_currentStartTime = info.dateTime.addSecs(-10);
    m_currentEndTime = info.dateTime.addSecs(10);
    m_currentTime = m_currentStartTime;

    ui->labelPlayTime->setText(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setTimeRange(m_currentStartTime, m_currentEndTime);
    ui->timeSlider->setCurrentDateTime(m_currentStartTime);

    //
    m_currentFileType = FILE_TYPE_MAIN;
    req_search_common_backup common_backup;
    memset(&common_backup, 0, sizeof(req_search_common_backup));
    MyQt::makeChannelMask(m_currentChannel, common_backup.chnMaskl, sizeof(common_backup.chnMaskl));
    common_backup.chnNum = 1;
    common_backup.enType = m_currentFileType;
    common_backup.enEvent = REC_EVENT_ALL;
    common_backup.enState = SEG_STATE_ALL;
    snprintf(common_backup.pStartTime, sizeof(common_backup.pStartTime), "%s", m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(common_backup.pEndTime, sizeof(common_backup.pEndTime), "%s", m_currentEndTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    qDebug() << QString("FaceDetection::updateVideoInfo, REQUEST_FLAG_SEARCH_COM_PLAYBACK_OPEN, channel: %1, start time: %2, end time: %3")
                    .arg(m_currentChannel)
                    .arg(QString(common_backup.pStartTime))
                    .arg(QString(common_backup.pEndTime));
    sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP, &common_backup, sizeof(req_search_common_backup));
    int searchMainResult = m_waitForSearch.exec();
    if (searchMainResult < 0) {
        m_currentFileType = FILE_TYPE_SUB;
        common_backup.enType = m_currentFileType;
        sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP, &common_backup, sizeof(req_search_common_backup));
        int searchSubResult = m_waitForSearch.exec();
        Q_UNUSED(searchSubResult)
    }

    qDebug() << QString("FaceDetection::updateVideoInfo, end");
}

void PageFaceDetectionSearch::updateBigPicture()
{
    int index = m_pageIndex * 20 + m_selectedItemIndex;
    if (index < 0) {
        return;
    }

    //
    rep_get_search_backup search_backup;
    search_backup.sid = m_faceSearchSid;
    search_backup.npage = index;
    qDebug() << QString("Face Detection::updateBigPicture, REQUEST_FLAG_SEARCH_FACE_BACKUP_BIGIMG, index: %1, sid: %2").arg(index).arg(search_backup.sid);
    sendMessage(REQUEST_FLAG_SEARCH_FACE_BACKUP_BIGIMG, &search_backup, sizeof(rep_get_search_backup));

    m_waitForSearch.exec();
}

void PageFaceDetectionSearch::stopCommonPlay()
{
    if (m_playSid < 0) {
        return;
    }

    m_timer->stop();
    ui->labelPlayTime->setText(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
    m_currentTime = m_currentStartTime;
    ui->timeSlider->setCurrentDateTime(m_currentTime);

    const QString &strState = ui->toolButtonPlay->property("state").toString();
    if (strState == "pause") {
        ui->toolButtonPlay->setProperty("state", "play");
        ui->toolButtonPlay->style()->unpolish(ui->toolButtonPlay);
        ui->toolButtonPlay->style()->polish(ui->toolButtonPlay);
        ui->toolButtonPlay->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
        ui->toolButtonPlay->update();
    }

    sendMessage(REQUEST_FLAG_PLAYSTOP_COM_BACKUP, &m_playSid, sizeof(int));
    m_playSid = -1;

    m_waitForStopPlay.exec();
}

void PageFaceDetectionSearch::stopAndClearCommonPlay()
{
    stopCommonPlay();
    //
    if (!m_backupList.isEmpty()) {
        const resp_search_common_backup &common_backup = m_backupList.first();
        qDebug() << QString("Face Detection::updateVideoInfo, REQUEST_FLAG_SEARCH_COM_BACKUP_CLOSE, sid: %1").arg(common_backup.sid);
        sendMessage(REQUEST_FLAG_SEARCH_FACE_BACKUP_CLOSE, &common_backup.sid, sizeof(int));
    }
    m_backupList.clear();
    m_currentBackupSid = -1;
}

void PageFaceDetectionSearch::searchFacePage(int page)
{
    qDebug() << QString("Face Detection:searchAnprPage, page index: %1").arg(page);

    int index = page * 20;
    if (index >= m_allCount) {
        qWarning() << QString("Face Detection::searchAnprPage, invalid page: %1, all count: %2").arg(page).arg(m_allCount);
        return;
    }

    m_faceMap.clear();
    m_searchedCount = 0;
    clearTable();
    ui->labelPage->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_allCount).arg(m_pageIndex + 1).arg(m_pageCount));

    rep_get_search_backup search_backup;
    search_backup.sid = m_faceSearchSid;
    search_backup.npage = index;
    Q_UNUSED(search_backup)
}

int PageFaceDetectionSearch::selectedItemIndex() const
{
    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        if (item->isInfoVisible() && item->isSelected()) {
            return item->index();
        }
    }
    return -1;
}

void PageFaceDetectionSearch::seekLivePlayback(const QDateTime &dateTime)
{
    struct req_seek_time seekinfo;
    memset(&seekinfo, 0, sizeof(struct req_seek_time));
    seekinfo.chnid = m_currentChannel;
    seekinfo.sid = m_playSid;
    snprintf(seekinfo.pseektime, sizeof(seekinfo.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_PLAYSEEK_BACKUP, (void *)&seekinfo, sizeof(struct req_seek_time));
}

void PageFaceDetectionSearch::backupFile(const QList<int> &backupList)
{
    //
    const QString &strPath = MyFileSystemDialog::instance()->exportFace();
    if (strPath.isEmpty()) {
        qDebug() << QString("Face Detection::backupFile, path is empty.");
        return;
    }
    const int &anprExportType = MyFileSystemDialog::instance()->anprExportType();
    const int &streamType = MyFileSystemDialog::instance()->streamType();
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
    //m_progress->showProgress();
    //
    if (m_faceMap.isEmpty()) {
        qWarning() << QString("Face Detection::backupFile, m_faceMap is empty.");
        //m_progress->hideProgress();
        ShowMessageBox(QString("Download failed."));
        return;
    }
    const FaceBackupInfo &firstInfo = m_faceMap.constBegin().value();
    QString plateListName = QString("FACE_log_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    //
    qDebug() << QString("Face Detectionsis::backupFile, begin, size: %1").arg(backupList.size());
    int complatedCount = 0;
    int result = 0;
    for (int i = 0; i < backupList.size(); ++i) {
        if (m_isExportCancel) {
            result |= 2;
            qDebug() << QString("Face Detection::backupFile, cancel");
            break;
        }

        //m_progress->setProgressValue((qreal)complatedCount / backupList.size() * 100);

        req_export_face_backup export_face_backup;
        memset(&export_face_backup, 0, sizeof(req_export_face_backup));
        export_face_backup.sid = firstInfo.sid;
        export_face_backup.index = backupList.at(i);
        export_face_backup.exportType = anprExportType;
        export_face_backup.streamType = streamType;
        export_face_backup.fileType = fileType;
        snprintf(export_face_backup.logfileName, sizeof(export_face_backup.logfileName), "%s", plateListName.toStdString().c_str());
        snprintf(export_face_backup.devPath, sizeof(export_face_backup.devPath), "%s", strPath.toStdString().c_str());
        sendMessage(REQUEST_FLAG_EXPORT_FACE_BACKUP, &export_face_backup, sizeof(rep_export_anpr_backup));

        int backResult = m_waitForExport.exec();
        complatedCount++;

        //
        const int anprBackType = MyFileSystemDialog::instance()->anprExportBackType();
        if (backResult == 0) {
            result |= SMARTANALYSIS_BACKUP_SUCCESS;
        } else if (backResult == anprBackType) {
            result |= SMARTANALYSIS_BACKUP_FAILED;
        } else {
            result |= SMARTANALYSIS_BACKUP_SOMESUCCESS;
        }
    }
    qDebug() << QString("Face Detection::backupFile, end");

    //
    m_isExporting = false;
    m_isExportCancel = false;
    //m_progress->hideProgress();
    if (result == SMARTANALYSIS_BACKUP_SUCCESS) {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145000", "Backup successfully."));
    } else if (result == SMARTANALYSIS_BACKUP_SOMESUCCESS) {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145001", "Failed to backup some results."));
    } else {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145002", "Failed to backup."));
    }
}

void PageFaceDetectionSearch::clearTable()
{
    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        item->setInfoVisible(false);
        item->setSelected(false);
        item->setChecked(false);
    }
    ui->checkBoxResultAll->setChecked(false);
}

void PageFaceDetectionSearch::updateTable(int index, const QImage &smallImage)
{
    if (index < 0 || index >= m_itemList.size()) {
        return;
    }

    AnprItemWidget *item = m_itemList.at(index);
    int infoIndex = m_pageIndex * 20 + index;
    if (m_faceMap.contains(infoIndex)) {
        const FaceBackupInfo &info = m_faceMap.value(infoIndex);
        item->setItemInfo(info.channel, info.dateTime, smallImage);
        item->setInfoVisible(true);
    } else {
        item->setInfoVisible(false);
    }
}

void PageFaceDetectionSearch::closeFaceSearch()
{
    if (m_faceSearchSid != -1) {
        sendMessage(REQUEST_FLAG_SEARCH_FACE_BACKUP_CLOSE, &m_faceSearchSid, sizeof(int));

        m_faceSearchSid = -1;
    }
}

void PageFaceDetectionSearch::onLanguageChanged()
{
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelAge->setText(GET_TEXT("FACE/141035", "Age"));
    ui->labelGlasses->setText(GET_TEXT("FACE/141037", "Glasses"));
    ui->labelMask->setText(GET_TEXT("FACE/141038", "Mask"));
    ui->labelCap->setText(GET_TEXT("FACE/141039", "Cap"));
    ui->labelGender->setText(GET_TEXT("FACE/141034", "Gender"));
    ui->labelStartTime->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->labelEndTime->setText(GET_TEXT("LOG/64002", "End Time"));
    ui->pushButtonSearch->setText(GET_TEXT("ANPR/103037", "Search"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->toolButtonStop->setToolTip(GET_TEXT("COMMONBACKUP/100044", "Stop"));
    ui->toolButtonPlay->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
    ui->toolButtonPrevious->setToolTip(GET_TEXT("COMMONBACKUP/100047", "Previous"));
    ui->toolButtonNext->setToolTip(GET_TEXT("COMMONBACKUP/100048", "Next"));

    ui->pushButtonGo->setText(GET_TEXT("PLAYBACK/80065", "Go"));
    ui->pushButtonBackupAll->setText(GET_TEXT("COMMONBACKUP/100002", "Backup All"));
    ui->pushButtonBackup->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->pushButtonResultBack->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->toolButtonFirstPage->setToolTip(GET_TEXT("PLAYBACK/80061", "First Page"));
    ui->toolButtonPreviousPage->setToolTip(GET_TEXT("PLAYBACK/80062", "Previous Page"));
    ui->toolButtonNextPage->setToolTip(GET_TEXT("PLAYBACK/80063", "Next Page"));
    ui->toolButtonLastPage->setToolTip(GET_TEXT("PLAYBACK/80064", "Last Page"));

    ui->labelResultAge->setText(GET_TEXT("FACE/141035", "Age"));
    ui->labelResultGlasses->setText(GET_TEXT("FACE/141037", "Glasses"));
    ui->labelResultMask->setText(GET_TEXT("FACE/141038", "Mask"));
    ui->labelResultCap->setText(GET_TEXT("FACE/141039", "Cap"));
    ui->labelResultGender->setText(GET_TEXT("FACE/141034", "Gender"));
    ui->labelFaceCaptureResult->setText(GET_TEXT("FACE/141065", "Face Capture Results"));
}

void PageFaceDetectionSearch::onSearchCanceled()
{
    if (m_isExporting) {
        m_isExportCancel = true;
        return;
    }

    if (m_progressSid >= 0) {
        sendMessage(REQUEST_FLAG_SEARCH_FACE_BACKUP_CANCEL, &m_progressSid, sizeof(m_progressSid));
        //m_progressSid = -1;
    }
}

void PageFaceDetectionSearch::onChannelCheckBoxClicked()
{
}

void PageFaceDetectionSearch::onItemClicked(int index)
{
    qDebug() << QString("FaceDetection::onItemClicked, index: %1, begin").arg(index);
    m_selectedItemIndex = index;

    AnprItemWidget *clickedItem = m_itemList.at(index);
    if (!clickedItem->isInfoVisible()) {
        qDebug() << QString("FaceDetection::onItemClicked, index: %1, return").arg(index);
        return;
    }

    //showWait();
    stopAndClearCommonPlay();

    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);

        int infoIndex = m_pageIndex * 20 + i;
        if (i == index) {
            m_currentIndex = infoIndex;
            item->setSelected(true);
        } else {
            item->setSelected(false);
        }
    }
    //
    if (m_faceMap.contains(m_currentIndex)) {
        //更新大图信息
        updateBigPicture();
        //
        const FaceBackupInfo &info = m_faceMap.value(m_currentIndex);
        ui->labelResultChannel->setText(QString("%1: %2").arg(GET_TEXT("CHANNELMANAGE/30008", "Channel")).arg(info.channel + 1));
        ui->labelResultTime->setText(QString("%1: %2").arg(GET_TEXT("LOG/64005", "Time")).arg(info.dateTime.toString("yyyy-MM-dd HH:mm:ss")));
        ui->labelResultAge->setText(QString("%1: %2").arg(GET_TEXT("FACE/141035", "Age")).arg(info.ageType));
        ui->labelResultGender->setText(QString("%1: %2").arg(GET_TEXT("FACE/141034", "Gender")).arg(info.genderType));
        ui->labelResultGlasses->setText(QString("%1: %2").arg(GET_TEXT("FACE/141037", "Glasses")).arg(info.glassesType));
        ui->labelResultMask->setText(QString("%1: %2").arg(GET_TEXT("FACE/141038", "Mask")).arg(info.maskType));
        ui->labelResultCap->setText(QString("%1: %2").arg(GET_TEXT("FACE/141039", "Cap")).arg(info.capType));

        //更新视频信息
        ui->labelPlayTime->clear();
        ui->timeSlider->setTimeRange(info.dateTime.addSecs(-10), info.dateTime.addSecs(10));
        ui->commonVideo->showCurrentChannel(info.channel);
    }

    //
    //closeWait();
    qDebug() << QString("Face Detection::onItemClicked, index: %1, end").arg(index);

    if (m_allCount >= MAX_SEARCH_BACKUP_COUNT && m_isAboutToShowMessage) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100042", "The search results is over %1, and the first %1 will be displayed.").arg(MAX_SEARCH_BACKUP_COUNT));
        m_isAboutToShowMessage = false;
    }
}

void PageFaceDetectionSearch::onItemChecked(int index, bool checked)
{
    Q_UNUSED(index)
    Q_UNUSED(checked)

    int visibleCount = 0;
    int checkedCount = 0;
    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        if (item->isInfoVisible()) {
            visibleCount++;

            if (item->isChecked()) {
                checkedCount++;
            }
        }
    }
    if (checkedCount == 0) {
        ui->checkBoxResultAll->setCheckState(Qt::Unchecked);
    } else if (checkedCount == visibleCount) {
        ui->checkBoxResultAll->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxResultAll->setCheckState(Qt::PartiallyChecked);
    }
}

void PageFaceDetectionSearch::onRealPlaybackTime()
{
    m_currentTime = m_currentTime.addSecs(1);
    if (m_currentTime >= m_currentEndTime) {
        m_currentTime = m_currentEndTime;
        on_toolButtonPlay_clicked();
    }
    ui->labelPlayTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setCurrentDateTime(m_currentTime);
}

void PageFaceDetectionSearch::onSliderValueChanged(int value)
{
    m_currentTime = QDateTime::fromTime_t(value);
    ui->labelPlayTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));
    if (m_playSid < 0) {
        return;
    }

    seekLivePlayback(m_currentTime);
}

void PageFaceDetectionSearch::on_toolButtonStop_clicked()
{
    showVideoWait();

    stopCommonPlay();
    ui->commonVideo->showPixmap();

    //closeWait();
}

void PageFaceDetectionSearch::on_toolButtonPlay_clicked()
{
    ui->toolButtonPlay->setAttribute(Qt::WA_UnderMouse, false);
    ui->toolButtonPlay->clearFocus();
    if (m_backupList.isEmpty()) {
        showVideoWait();
        updateVideoInfo();
        //closeWait();
        if (m_backupList.isEmpty()) {
            ShowMessageBox(GET_TEXT("DISK/92033", "No record files currently."));
            return;
        }
    }

    const QString &strState = ui->toolButtonPlay->property("state").toString();
    if (strState == "play") {
        ui->toolButtonPlay->setToolTip(GET_TEXT("COMMONBACKUP/100046", "Pause"));
        ui->toolButtonPlay->setProperty("state", "pause");
        ui->toolButtonPlay->style()->unpolish(ui->toolButtonPlay);
        ui->toolButtonPlay->style()->polish(ui->toolButtonPlay);
        ui->toolButtonPlay->update();

        if (m_playSid < 0) {
            const resp_search_common_backup &backup = m_backupList.first();

            m_playSid = backup.sid;
            ui->commonVideo->hidePixmap();
            ui->commonVideo->playbackVideo(backup.chnid);

            struct rep_play_common_backup playinfo;
            memset(&playinfo, 0, sizeof(struct rep_play_common_backup));
            playinfo.chnid = backup.chnid;
            playinfo.sid = backup.sid;
            snprintf(playinfo.pStartTime, sizeof(playinfo.pStartTime), "%s", backup.pStartTime);
            snprintf(playinfo.pEndTime, sizeof(playinfo.pEndTime), "%s", backup.pEndTime);
            snprintf(playinfo.pPlayTime, sizeof(playinfo.pPlayTime), "%s", m_currentTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
            qDebug() << QString("REQUEST_FLAG_PLAY_COM_BACKUP, start time: %1, end time: %2, play time: %3").arg(playinfo.pStartTime).arg(playinfo.pEndTime).arg(playinfo.pPlayTime);
            sendMessage(REQUEST_FLAG_PLAY_COM_BACKUP, &playinfo, sizeof(struct rep_play_common_backup));
        } else {
            if (m_currentTime >= m_currentEndTime) {
                //play from start time
                m_currentTime = m_currentStartTime;
                seekLivePlayback(m_currentTime);
            }
            sendMessage(REQUEST_FLAG_PLAYRESTART_COM_BACKUP, &m_playSid, sizeof(int));
        }
    } else if (strState == "pause") {
        m_timer->stop();

        ui->toolButtonPlay->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
        ui->toolButtonPlay->setProperty("state", "play");
        ui->toolButtonPlay->style()->unpolish(ui->toolButtonPlay);
        ui->toolButtonPlay->style()->polish(ui->toolButtonPlay);
        ui->toolButtonPlay->update();

        sendMessage(REQUEST_FLAG_PLAYPAUSE_COM_BACKUP, &m_playSid, sizeof(int));
    }
}

void PageFaceDetectionSearch::on_toolButtonPrevious_clicked()
{
    int itemIndex = selectedItemIndex();
    if (itemIndex > 0) {
        stopCommonPlay();

        itemIndex--;
        onItemClicked(itemIndex);
    }
}

void PageFaceDetectionSearch::on_toolButtonNext_clicked()
{
    int itemIndex = selectedItemIndex();
    if (itemIndex < 20 - 1) {
        stopCommonPlay();

        itemIndex++;
        onItemClicked(itemIndex);
    }
}

void PageFaceDetectionSearch::on_checkBoxResultAll_clicked(bool checked)
{
    if (ui->checkBoxResultAll->checkState() == Qt::PartiallyChecked) {
        ui->checkBoxResultAll->setCheckState(Qt::Checked);
    }

    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        if (item->isInfoVisible()) {
            item->setChecked(checked);
        }
    }
}

void PageFaceDetectionSearch::on_toolButtonFirstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    stopAndClearCommonPlay();

    searchFacePage(m_pageIndex);
}

void PageFaceDetectionSearch::on_toolButtonPreviousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    stopAndClearCommonPlay();

    searchFacePage(m_pageIndex);
}

void PageFaceDetectionSearch::on_toolButtonNextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    stopAndClearCommonPlay();
    searchFacePage(m_pageIndex);
}

void PageFaceDetectionSearch::on_toolButtonLastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    ui->lineEditPage->setText(QString::number(m_pageIndex + 1));

    stopAndClearCommonPlay();

    searchFacePage(m_pageIndex);
}

void PageFaceDetectionSearch::on_pushButtonGo_clicked()
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

    stopAndClearCommonPlay();

    searchFacePage(m_pageIndex);
}

void PageFaceDetectionSearch::on_pushButtonBackupAll_clicked()
{
    QList<int> list;
    for (int i = 0; i < m_allCount; ++i) {
        list.append(i);
    }
    backupFile(list);
    MyFileSystemDialog::instance()->clearAnprSelected();
}

void PageFaceDetectionSearch::on_pushButtonBackup_clicked()
{
    QList<int> list;
    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        if (item->isInfoVisible() && item->isChecked()) {
            int infoIndex = m_pageIndex * m_itemCountInPage + item->index();
            list.append(infoIndex);
        }
    }
    if (list.isEmpty()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        return;
    }

    backupFile(list);
    MyFileSystemDialog::instance()->clearAnprSelected();
}

void PageFaceDetectionSearch::on_pushButtonResultBack_clicked()
{
    closePage();

    ui->stackedWidget->setCurrentIndex(0);
    ui->commonVideo->hideVideo();
    emit updateVideoGeometry();
}

void PageFaceDetectionSearch::on_pushButtonSearch_clicked()
{
    m_pageIndex = 0;
    m_pageCount = 0;
    m_searchedCount = 0;
    m_faceMap.clear();
    const QList<int> &checkedList = ui->checkBoxGroupChannel->checkedList();
    if (checkedList.isEmpty()) {
        ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel"));
        return;
    }
    const QDateTime startDateTime(ui->dateEditStart->date(), ui->timeEditStart->time());
    const QDateTime endDateTime(ui->dateEditEnd->date(), ui->timeEditEnd->time());
    if (startDateTime > endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }
    const QString &checkedMask = ui->checkBoxGroupChannel->checkedMask();
    struct req_search_face_backup faceBackup;
    memset(&faceBackup, 0, sizeof(req_search_face_backup));
    snprintf(faceBackup.chnMask, sizeof(faceBackup.chnMask), "%s", checkedMask.toStdString().c_str());
    faceBackup.chnNum = checkedList.count();
    faceBackup.filter.age = static_cast<FACE_AGE>(ui->comboBoxAge->currentIntData());
    faceBackup.filter.gender = ui->comboBoxGender->currentIntData();
    faceBackup.filter.glasses = ui->comboBoxGlasses->currentIntData();
    faceBackup.filter.mask = ui->comboBoxMask->currentIntData();
    faceBackup.filter.cap = ui->comboBoxCap->currentIntData();
    snprintf(faceBackup.pStartTime, sizeof(faceBackup.pStartTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(faceBackup.pEndTime, sizeof(faceBackup.pEndTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());

    sendMessage(REQUEST_FLAG_SEARCH_FACE_BACKUP, &faceBackup, sizeof(struct req_search_face_backup));
    //m_progress->setProgressValue(0);
    //m_progress->showProgress();
}

void PageFaceDetectionSearch::on_pushButtonBack_clicked()
{
    emit sig_back();
}
