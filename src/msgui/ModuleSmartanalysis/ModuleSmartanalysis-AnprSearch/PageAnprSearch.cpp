#include "PageAnprSearch.h"
#include "ui_PageAnprSearch.h"
#include "MessageBox.h"
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

PageAnprSearch::PageAnprSearch(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::AnprAnalysis)
{
    ui->setupUi(this);

    ui->checkBoxGroup_channel->setCountFromChannelName(qMsNvr->maxChannel());
    connect(ui->checkBoxGroup_channel, SIGNAL(checkBoxClicked()), this, SLOT(onChannelCheckBoxClicked()));

    ui->labelLicensePlateType->setText(GET_TEXT("ANPR/103093", "License Plate Type"));

    ui->comboBoxLicensePlateType->clear();
    ui->comboBoxLicensePlateType->addItem(GET_TEXT("ANPR/103038", "All"), MAX_ANPR_MODE);
    ui->comboBoxLicensePlateType->addItem(GET_TEXT("ANPR/103039", "Black"), ANPR_BLACK);
    ui->comboBoxLicensePlateType->addItem(GET_TEXT("ANPR/103040", "White"), ANPR_WHITE);
    ui->comboBoxLicensePlateType->addItem(GET_TEXT("ANPR/103094", "Visitor"), ANPR_VISTOR);

    ui->comboBoxPlateColor->beginEdit();
    ui->comboBoxPlateColor->clear();
    ui->comboBoxPlateColor->addItem(GET_TEXT("ANPR/103038", "All"), ANPR_COLOR_ALL);
    ui->comboBoxPlateColor->addItem(GET_TEXT("LOG/64076", "N/A"), ANPR_COLOR_NONE);
    ui->comboBoxPlateColor->addItem(GET_TEXT("ANPR/103104", "Black"), ANPR_COLOR_BLACK);
    ui->comboBoxPlateColor->addItem(GET_TEXT("ANPR/103105", "Blue"), ANPR_COLOR_BLUE);
    ui->comboBoxPlateColor->addItem(GET_TEXT("ANPR/103106", "Cyan"), ANPR_COLOR_CYAN);
    ui->comboBoxPlateColor->addItem(GET_TEXT("ANPR/103107", "Gray"), ANPR_COLOR_GRAY);
    ui->comboBoxPlateColor->addItem(GET_TEXT("ANPR/103108", "Green"), ANPR_COLOR_GREEN);
    ui->comboBoxPlateColor->addItem(GET_TEXT("ANPR/103109", "Red"), ANPR_COLOR_RED);
    ui->comboBoxPlateColor->addItem(GET_TEXT("ANPR/103110", "White"), ANPR_COLOR_WHITE);
    ui->comboBoxPlateColor->addItem(GET_TEXT("ANPR/103111", "Yellow"), ANPR_COLOR_YELLOW);
    ui->comboBoxPlateColor->endEdit();

    ui->comboBoxVehicleType->beginEdit();
    ui->comboBoxVehicleType->clear();
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/103038", "All"), ANPR_VEHICLE_ALL);
    ui->comboBoxVehicleType->addItem(GET_TEXT("LOG/64076", "N/A"), ANPR_VEHICLE_NONE);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/103112", "Car"), ANPR_VEHICLE_CAR);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/169079", "SUV"), ANPR_VEHICLE_SUV);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/169078", "Van"), ANPR_VEHICLE_VAN);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/103114", "Bus"), ANPR_VEHICLE_BUS);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/169082", "Tow Truck"), ANPR_VEHICLE_TOWTRUCK);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/103115", "Truck"), ANPR_VEHICLE_TRUCK);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/169084", "Fire Engine"), ANPR_VEHICLE_FIREENGINE);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/169085", "Ambulance"), ANPR_VEHICLE_AMBULANCE);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/169090", "Motorbike"), ANPR_VEHICLE_MOTORCYCLE);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/169086", "Bicycle"), ANPR_VEHICLE_BICYCLE);
    ui->comboBoxVehicleType->addItem(GET_TEXT("ANPR/169088", "Others"), ANPR_VEHICLE_OTHER);
    ui->comboBoxVehicleType->endEdit();

    ui->comboBoxVehicleColor->beginEdit();
    ui->comboBoxVehicleColor->clear();
    ui->comboBoxVehicleColor->addItem(GET_TEXT("ANPR/103038", "All"), ANPR_COLOR_ALL);
    ui->comboBoxVehicleColor->addItem(GET_TEXT("LOG/64076", "N/A"), ANPR_COLOR_NONE);
    ui->comboBoxVehicleColor->addItem(GET_TEXT("ANPR/103104", "Black"), ANPR_COLOR_BLACK);
    ui->comboBoxVehicleColor->addItem(GET_TEXT("ANPR/103105", "Blue"), ANPR_COLOR_BLUE);
    ui->comboBoxVehicleColor->addItem(GET_TEXT("ANPR/103106", "Cyan"), ANPR_COLOR_CYAN);
    ui->comboBoxVehicleColor->addItem(GET_TEXT("ANPR/103107", "Gray"), ANPR_COLOR_GRAY);
    ui->comboBoxVehicleColor->addItem(GET_TEXT("ANPR/103108", "Green"), ANPR_COLOR_GREEN);
    ui->comboBoxVehicleColor->addItem(GET_TEXT("ANPR/103109", "Red"), ANPR_COLOR_RED);
    ui->comboBoxVehicleColor->addItem(GET_TEXT("ANPR/103110", "White"), ANPR_COLOR_WHITE);
    ui->comboBoxVehicleColor->addItem(GET_TEXT("ANPR/103111", "Yellow"), ANPR_COLOR_YELLOW);
    ui->comboBoxVehicleColor->endEdit();

    ui->comboBoxSpeed->beginEdit();
    ui->comboBoxSpeed->clear();
    ui->comboBoxSpeed->addItem(GET_TEXT("ANPR/103038", "All"), ANPR_SPEED_ALL);
    ui->comboBoxSpeed->addItem(GET_TEXT("LOG/64076", "N/A"), ANPR_SPEED_NONE);
    ui->comboBoxSpeed->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/103309", "More Than"), ANPR_SPEED_MORE);
    ui->comboBoxSpeed->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/103310", "Less Than"), ANPR_SPEED_LESS);
    ui->comboBoxSpeed->endEdit();

    ui->comboBoxDirection->beginEdit();
    ui->comboBoxDirection->clear();
    ui->comboBoxDirection->addItem(GET_TEXT("ANPR/103038", "All"), ANPR_DIRECTION_ALL);
    ui->comboBoxDirection->addItem(GET_TEXT("LOG/64076", "N/A"), ANPR_DIRECTION_NONE);
    ui->comboBoxDirection->addItem(GET_TEXT("ANPR/103056", "Away"), ANPR_DIRECTION_ALWAYS);
    ui->comboBoxDirection->addItem(GET_TEXT("ANPR/103055", "Approach"), ANPR_DIRECTION_APPROACH);
    ui->comboBoxDirection->endEdit();

    ui->lineEditSpeed->setCheckMode(MyLineEdit::RangeCheck, 0, 999);
    QRegExp numrx(QString("[0-9]*"));
    QValidator *numvalidator = new QRegExpValidator(numrx, this);
    ui->lineEditSpeed->setValidator(numvalidator);
    ui->lineEditSpeed->setVisible(false);
    ui->labelSppedText->setVisible(false);

    ui->label_size->hide();
    ui->label_playTime->clear();

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
            connect(item, SIGNAL(itemClicked(int)), this, SLOT(onItemClicked(int)));
            connect(item, SIGNAL(itemChecked(int, bool)), this, SLOT(onItemChecked(int, bool)));
            ui->gridLayout_item->addWidget(item, row, column);

            m_itemList.append(item);
        }
    }

    //slider
    connect(ui->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

    //page
    connect(ui->lineEdit_page, SIGNAL(textEdited(QString)), this, SLOT(onPageEdited(QString)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onRealPlaybackTime()));
    m_timer->setInterval(1000);

    m_progress = new ProgressDialog(this);
    connect(m_progress, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));

    onLanguageChanged();
}

PageAnprSearch::~PageAnprSearch()
{
    delete ui;
}

void PageAnprSearch::initializeData()
{
    ui->stackedWidget->setCurrentIndex(0);

    ui->lineEdit_licensePlate->clear();
    ui->dateEdit_start->setDate(QDate::currentDate());
    ui->timeEdit_start->setTime(QTime(0, 0, 0));
    ui->dateEdit_end->setDate(QDate::currentDate());
    ui->timeEdit_end->setTime(QTime(23, 59, 59));

    ui->checkBoxGroup_channel->clearCheck();
    ui->comboBoxLicensePlateType->setCurrentIndex(0);
    ui->comboBoxPlateColor->setCurrentIndex(0);
    ui->comboBoxVehicleType->setCurrentIndex(0);
    ui->comboBoxVehicleColor->setCurrentIndex(0);
    ui->comboBoxSpeed->setCurrentIndex(0);
    ui->comboBoxDirection->setCurrentIndex(0);
}

bool PageAnprSearch::isCloseable()
{
    if (m_isExporting) {
        return false;
    } else {
        return true;
    }
}

void PageAnprSearch::closePage()
{
    qDebug() << QString("AnprAnalysis::closeSetting, begin");

    m_isAbaoutToQuit = true;

    stopAndClearCommonPlay();
    clearTable();
    closeAnprSearch();

    qDebug() << QString("AnprAnalysis::closeSetting, end");
}

bool PageAnprSearch::canAutoLogout()
{
    bool ok = true;
    if (m_timer->isActive()) {
        ok = false;
    }
    return ok;
}

NetworkResult PageAnprSearch::dealNetworkCommond(const QString &commond)
{
    if (ui->stackedWidget->currentIndex() == 0) {
        return NetworkReject;
    }

    qDebug() << "AnprAnalysis::dealNetworkCommond," << commond;

    NetworkResult result = NetworkReject;
    if (commond.startsWith("Video_Play")) {
        const QString &state = ui->toolButton_play->property("state").toString();
        if (state == "play") {
            QMetaObject::invokeMethod(this, "on_toolButton_play_clicked", Qt::QueuedConnection);
            result = NetworkAccept;
        }
    } else if (commond.startsWith("Video_Pause")) {
        const QString &state = ui->toolButton_play->property("state").toString();
        if (state == "pause") {
            QMetaObject::invokeMethod(this, "on_toolButton_play_clicked", Qt::QueuedConnection);
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

void PageAnprSearch::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_ANPR_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP(message);
        break;
    case RESPONSE_FLAG_SEARCH_ANPR_BACKUP_PAGE:
        ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP_PAGE(message);
        break;
    case RESPONSE_FLAG_SEARCH_ANPR_BACKUP_BIGIMG:
        ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP_BIGIMG(message);
        break;
    case RESPONSE_FLAG_SEARCH_COM_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_PLAY_COM_PICTURE:
        ON_RESPONSE_FLAG_PLAY_COM_PICTURE(message);
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
    case RESPONSE_FLAG_EXPORT_ANPR_BACKUP:
        ON_RESPONSE_FLAG_EXPORT_ANPR_BACKUP(message);
        break;
    case RESPONSE_FLAG_GET_EXPORT_DISK:
        ON_RESPONSE_FLAG_GET_EXPORT_DISK(message);
        break;
    }
}

void PageAnprSearch::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        break;
    }
}

void PageAnprSearch::ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP(MessageReceive *message)
{
    qDebug() << "====AnprAnalysis::ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP, begin====";

    // m_progressSid = -1;
    // m_progress->hideProgress();

    resp_search_anpr_backup *anpr_backup = (resp_search_anpr_backup *)message->data;
    if (!anpr_backup) {
        qDebug() << "----data:" << anpr_backup;
        ShowMessageBox(GET_TEXT("ANPR/103075", "No matching items."));
        return;
    }
    qDebug() << "----index:" << anpr_backup->index;
    qDebug() << "----allCnt:" << anpr_backup->allCnt;
    qDebug() << "----bImageSize:" << anpr_backup->bImageSize;
    qDebug() << "----sImageSize:" << anpr_backup->sImageSize;

    AnprBackupInfo info(message);
    m_anprMap.insert(anpr_backup->index, info);

    //page
    m_pageCount = qCeil(anpr_backup->allCnt / 20.0);
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(anpr_backup->allCnt).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setMaxPage(m_pageCount);
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    ui->stackedWidget->setCurrentIndex(1);
    m_allCount = anpr_backup->allCnt;
    m_anprSearchSid = anpr_backup->sid;
    updateTable(m_searchedCount, message->image2);
    ui->commonVideo->showPixmap(QPixmap::fromImage(message->image1));

    m_searchedCount++;
    if (m_allCount > 1) {
        rep_get_search_backup search_backup;
        search_backup.sid = anpr_backup->sid;
        search_backup.npage = anpr_backup->index + 1;
        Q_UNUSED(search_backup)
    } else {
        QMetaObject::invokeMethod(this, "onItemClicked", Qt::QueuedConnection, Q_ARG(int, 0));
    }

    m_isAbaoutToQuit = false;
    m_isAboutToShowMessage = true;
    qDebug() << "====AnprAnalysis::ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP, end====";
}

void PageAnprSearch::ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP_PAGE(MessageReceive *message)
{
    resp_search_anpr_backup *anpr_backup = (resp_search_anpr_backup *)message->data;

    qDebug() << "====AnprAnalysis::ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP_PAGE====";
    qDebug() << "----index:" << anpr_backup->index;
    qDebug() << "----allCnt:" << anpr_backup->allCnt;
    qDebug() << "----bImageSize:" << anpr_backup->bImageSize;
    qDebug() << "----sImageSize:" << anpr_backup->sImageSize;

    m_anprMap.insert(anpr_backup->index, AnprBackupInfo(message));
    updateTable(m_searchedCount, message->image2);
    m_searchedCount++;
    if (m_searchedCount >= 20 || anpr_backup->index + 1 >= m_allCount) {
        QMetaObject::invokeMethod(this, "onItemClicked", Qt::QueuedConnection, Q_ARG(int, 0));
    } else {
        rep_get_search_backup search_backup;
        search_backup.sid = anpr_backup->sid;
        search_backup.npage = anpr_backup->index + 1;
        Q_UNUSED(search_backup)
    }
}

void PageAnprSearch::ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP_BIGIMG(MessageReceive *message)
{
    resp_search_anpr_backup *anpr_backup = (resp_search_anpr_backup *)message->data;
    if (!anpr_backup) {
        qDebug() << QString("AnprAnalysis::ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP_BIGIMG, data is null");
        ui->commonVideo->hidePixmap();
        m_waitForSearch.exit(-1);
        return;
    }
    qDebug() << QString("AnprAnalysis::ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP_BIGIMG, index: %1, all count: %2, size: %3").arg(anpr_backup->index).arg(anpr_backup->allCnt).arg(anpr_backup->bImageSize);

    ui->commonVideo->showPixmap(QPixmap::fromImage(message->image1));
    m_waitForSearch.exit(0);
}

void PageAnprSearch::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message)
{
    if (m_isExporting) {
        return;
    }

    if (!m_progress->isVisible()) {
        return;
    }

    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        qWarning() << "AnprAnalysis::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, data is null.";
        return;
    }
    qDebug() << QString("AnprAnalysis::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, %1").arg(progressinfo->percent);

    m_progress->setProgressValue(progressinfo->percent);
    m_progressSid = progressinfo->searchid;
}

void PageAnprSearch::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message)
{
    resp_search_common_backup *common_backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    //
    if (!common_backup_array) {
        qWarning() << QString("AnprAnalysis::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, resp_search_common_backup is null.");
    } else {
        if (common_backup_array->allCnt == 0) {
            qDebug() << QString("AnprAnalysis::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, resp_search_common_backup is empty.");
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
                qDebug() << QString("AnprAnalysis::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP, count: %1, index: %2, channel: %3, sid: %4, start time: %5, end time: %6")
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
        ui->label_playTime->setText(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
        ui->timeSlider->setTimeRange(m_currentStartTime, m_currentEndTime);
        m_currentTime = m_currentStartTime;

        //
        m_currentBackupSid = m_backupList.first().sid;
        m_waitForSearch.exit(0);
    }
}

void PageAnprSearch::ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message)
{
    qDebug() << QString("AnprAnalysis::ON_RESPONSE_FLAG_PLAY_COM_PICTURE, begin");

    m_waitForSearch.exit();
    if (m_playSid >= 0) {
        qDebug() << QString("AnprAnalysis::ON_RESPONSE_FLAG_PLAY_COM_PICTURE, return");
        return;
    }
    QPixmap pixmap = QPixmap::fromImage(message->image1);
    ui->commonVideo->showPixmap(pixmap);

    qDebug() << QString("AnprAnalysis::ON_RESPONSE_FLAG_PLAY_COM_PICTURE, end");
}

void PageAnprSearch::ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "AnprAnalysis::ON_RESPONSE_FLAG_PLAY_COM_BACKUP, data is null.";
        return;
    }
    int playinfo = (*((int *)message->data));
    switch (playinfo) {
    case -2:
        //搜索后磁盘被移除了等导致录像不存在。
        ShowMessageBox("Stream is invalid.");
        return;
    }

    m_timer->start();
}

void PageAnprSearch::ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message);

    m_timer->start();
}

void PageAnprSearch::ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)

    m_waitForStopPlay.exit();
}

void PageAnprSearch::ON_RESPONSE_FLAG_EXPORT_ANPR_BACKUP(MessageReceive *message)
{
    if (!message->data) {
        m_waitForExport.exit(-1);
        return;
    }
    int result = (*((int *)message->data));
    m_waitForExport.exit(result);
}

void PageAnprSearch::ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message)
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

void PageAnprSearch::showWait()
{
    AbstractSettingPage::showWait(ui->widget_itemContent);
}

void PageAnprSearch::showVideoWait()
{
    AbstractSettingPage::showWait(ui->commonVideo);
}

void PageAnprSearch::clearTable()
{
    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        item->setInfoVisible(false);
        item->setSelected(false);
        item->setChecked(false);
    }
    ui->checkBox_all->setChecked(false);
}

void PageAnprSearch::updateTable(int index, const QImage &smallImage)
{
    if (index < 0 || index >= m_itemList.size()) {
        return;
    }

    AnprItemWidget *item = m_itemList.at(index);
    int infoIndex = m_pageIndex * 20 + index;
    if (m_anprMap.contains(infoIndex)) {
        const AnprBackupInfo &info = m_anprMap.value(infoIndex);
        item->setItemInfo(info.channel, info.dateTime, smallImage);
        item->setInfoVisible(true);
    } else {
        item->setInfoVisible(false);
    }
}

void PageAnprSearch::updateVideoInfo()
{
    qDebug() << QString("AnprAnalysis::updateVideoInfo, begin");

    if (!m_anprMap.contains(m_currentIndex)) {
        qDebug() << QString("AnprAnalysis::updateVideoInfo, end, error index: %1").arg(m_currentIndex);
        return;
    }

    const AnprBackupInfo &info = m_anprMap.value(m_currentIndex);

    m_currentChannel = info.channel;
    ui->commonVideo->showCurrentChannel(m_currentChannel);

    m_currentStartTime = info.dateTime.addSecs(-10);
    m_currentEndTime = info.dateTime.addSecs(10);
    m_currentTime = m_currentStartTime;
    ui->label_playTime->setText(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
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
    qDebug() << QString("AnprAnalysis::updateVideoInfo, REQUEST_FLAG_SEARCH_COM_PLAYBACK_OPEN, channel: %1, start time: %2, end time: %3")
                    .arg(m_currentChannel)
                    .arg(QString(common_backup.pStartTime))
                    .arg(QString(common_backup.pEndTime));
    sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP, (void *)&common_backup, sizeof(req_search_common_backup));
    int searchMainResult = m_waitForSearch.exec();

    int searchSubResult = 0;
    if (searchMainResult < 0) {
        m_currentFileType = FILE_TYPE_SUB;
        common_backup.enType = m_currentFileType;
        sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP, (void *)&common_backup, sizeof(req_search_common_backup));
        searchSubResult = m_waitForSearch.exec();
    }
    if (searchSubResult < 0) {
    }

    qDebug() << QString("AnprAnalysis::updateVideoInfo, end");
}

void PageAnprSearch::updateBigPicture()
{
    int index = m_pageIndex * 20 + m_selectedItemIndex;
    if (index < 0) {
        return;
    }

    //
    rep_get_search_backup search_backup;
    search_backup.sid = m_anprSearchSid;
    search_backup.npage = index;
    qDebug() << QString("AnprAnalysis::updateBigPicture, REQUEST_FLAG_SEARCH_ANPR_BACKUP_BIGIMG, index: %1, sid: %2").arg(index).arg(search_backup.sid);
    sendMessage(REQUEST_FLAG_SEARCH_ANPR_BACKUP_BIGIMG, (void *)&search_backup, sizeof(rep_get_search_backup));

    m_waitForSearch.exec();
}

void PageAnprSearch::searchAnprPage(int page)
{
    qDebug() << QString("AnprAnalysis::searchAnprPage, page index: %1").arg(page);

    int index = page * 20;
    if (index >= m_allCount) {
        qWarning() << QString("AnprAnalysis::searchAnprPage, invalid page: %1, all count: %2").arg(page).arg(m_allCount);
        return;
    }

    m_anprMap.clear();
    m_searchedCount = 0;
    clearTable();
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_allCount).arg(m_pageIndex + 1).arg(m_pageCount));

    rep_get_search_backup search_backup;
    search_backup.sid = m_anprSearchSid;
    search_backup.npage = index;
    Q_UNUSED(search_backup)

    //showWait();
}

void PageAnprSearch::closeBackup()
{
    if (m_currentBackupSid != -1) {
        sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP_CLOSE, &m_currentBackupSid, sizeof(int));

        m_currentBackupSid = -1;
    }
}

void PageAnprSearch::closeAnprSearch()
{
    if (m_anprSearchSid != -1) {
        sendMessage(REQUEST_FLAG_SEARCH_ANPR_BACKUP_CLOSE, &m_anprSearchSid, sizeof(int));

        m_anprSearchSid = -1;
    }
}

void PageAnprSearch::searchCommonPicture()
{
    if (m_backupList.isEmpty()) {
        return;
    }
    //
    const resp_search_common_backup &common_backup = m_backupList.first();

    struct rep_play_common_backup playinfo;
    memset(&playinfo, 0, sizeof(struct rep_play_common_backup));
    playinfo.chnid = common_backup.chnid;
    playinfo.sid = common_backup.sid;
    playinfo.flag = 1;
    snprintf(playinfo.pStartTime, sizeof(playinfo.pStartTime), "%s", common_backup.pStartTime);
    snprintf(playinfo.pEndTime, sizeof(playinfo.pEndTime), "%s", common_backup.pEndTime);
    qDebug() << QString("AnprAnalysis::searchCommonPicture, REQUEST_FLAG_PLAY_COM_PICTURE, channel: %1, sid: %2, start time: %3, end time: %4")
                    .arg(playinfo.chnid)
                    .arg(playinfo.sid)
                    .arg(QString(playinfo.pStartTime))
                    .arg(QString(playinfo.pEndTime));
    sendMessage(REQUEST_FLAG_PLAY_COM_PICTURE, (void *)&playinfo, sizeof(struct rep_play_common_backup));

    m_waitForSearch.exec();
}

void PageAnprSearch::stopCommonPlay()
{
    if (m_playSid < 0) {
        return;
    }

    m_timer->stop();
    ui->label_playTime->setText(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
    m_currentTime = m_currentStartTime;
    ui->timeSlider->setCurrentDateTime(m_currentTime);

    const QString &strState = ui->toolButton_play->property("state").toString();
    if (strState == "pause") {
        ui->toolButton_play->setProperty("state", "play");
        ui->toolButton_play->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
        ui->toolButton_play->style()->unpolish(ui->toolButton_play);
        ui->toolButton_play->style()->polish(ui->toolButton_play);
        ui->toolButton_play->update();
    }

    sendMessage(REQUEST_FLAG_PLAYSTOP_COM_BACKUP, &m_playSid, sizeof(int));
    m_playSid = -1;

    m_waitForStopPlay.exec();
}

void PageAnprSearch::stopAndClearCommonPlay()
{
    stopCommonPlay();
    //
    if (!m_backupList.isEmpty()) {
        const resp_search_common_backup &common_backup = m_backupList.first();
        qDebug() << QString("AnprAnalysis::updateVideoInfo, REQUEST_FLAG_SEARCH_COM_BACKUP_CLOSE, sid: %1").arg(common_backup.sid);
        sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP_CLOSE, (void *)&(common_backup.sid), sizeof(int));
    }
    m_backupList.clear();
    m_currentBackupSid = -1;
}

int PageAnprSearch::selectedItemIndex() const
{
    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        if (item->isInfoVisible() && item->isSelected()) {
            return item->index();
        }
    }
    return -1;
}

QString PageAnprSearch::directionString(int value) const
{
    QString text;
    switch (value) {
    case 0:
        //text = QString(PARAM_MS_ANPR_DERECTION_0);
        text = "N/A";
        break;
    case 1:
        //text = QString(PARAM_MS_ANPR_DERECTION_1);
        text = GET_TEXT("ANPR/103055", "Approach");
        break;
    case 2:
        //text = QString(PARAM_MS_ANPR_DERECTION_2);
        text = GET_TEXT("ANPR/103056", "Away");
        break;
    default:
        text = "N/A";
        break;
    }
    return text;
}

void PageAnprSearch::backupFile(const QList<int> &backupList)
{
    //
    const QString &strPath = MyFileSystemDialog::instance()->exportAnpr();
    if (strPath.isEmpty()) {
        qDebug() << QString("AnprAnalysis::backupFile, path is empty.");
        return;
    }

    //const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    //const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeSize();
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
    if (m_anprMap.isEmpty()) {
        qWarning() << QString("AnprAnalysis::backupFile, m_anprMap is empty.");
        //m_progress->hideProgress();
        ShowMessageBox(QString("Download failed."));
        return;
    }
    struct network netDb;
    memset(&netDb, 0, sizeof(netDb));
    read_network(SQLITE_FILE_NAME, &netDb);
    const AnprBackupInfo &firstInfo = m_anprMap.constBegin().value();
    QString plateListName = QString("%1_log_%2.csv").arg(netDb.host_name).arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    //
    qDebug() << QString("AnprAnalysis::backupFile, begin, size: %1").arg(backupList.size());
    int complatedCount = 0;
    int result = 0;
    for (int i = 0; i < backupList.size(); ++i) {
        if (m_isExportCancel) {
            result |= 2;
            qDebug() << QString("AnprAnalysis::backupFile, cancel");
            break;
        }

        //m_progress->setProgressValue((qreal)complatedCount / backupList.size() * 100);

        rep_export_anpr_backup export_anpr_backup;
        memset(&export_anpr_backup, 0, sizeof(rep_export_anpr_backup));
        export_anpr_backup.sid = firstInfo.sid;
        export_anpr_backup.index = backupList.at(i);
        export_anpr_backup.exportType = anprExportType;
        export_anpr_backup.streamType = streamType;
        export_anpr_backup.avifileType = fileType;
        snprintf(export_anpr_backup.logfile_name, sizeof(export_anpr_backup.logfile_name), "%s", plateListName.toStdString().c_str());
        snprintf(export_anpr_backup.dev_path, sizeof(export_anpr_backup.dev_path), "%s", strPath.toStdString().c_str());
        sendMessage(REQUEST_FLAG_EXPORT_ANPR_BACKUP, (void *)&export_anpr_backup, sizeof(rep_export_anpr_backup));

        int ret = m_waitForExport.exec();
        complatedCount++;
        //
        const int anprBackType = MyFileSystemDialog::instance()->anprExportBackType();
        if (ret == 0) {
            result |= SMARTANALYSIS_BACKUP_SUCCESS;
        } else if (ret == anprBackType) {
            result |= SMARTANALYSIS_BACKUP_FAILED;
        } else {
            result |= SMARTANALYSIS_BACKUP_SOMESUCCESS;
        }
    }
    qDebug() << QString("AnprAnalysis::backupFile, end");

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

void PageAnprSearch::onLanguageChanged()
{
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->label_licensePlate->setText(GET_TEXT("ANPR/103035", "License Plate"));
    ui->label_startTime->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("LOG/64002", "End Time"));
    ui->pushButton_search->setText(GET_TEXT("ANPR/103037", "Search"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->toolButton_stop->setToolTip(GET_TEXT("COMMONBACKUP/100044", "Stop"));
    ui->toolButton_play->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
    ui->toolButton_previous->setToolTip(GET_TEXT("COMMONBACKUP/100047", "Previous"));
    ui->toolButton_next->setToolTip(GET_TEXT("COMMONBACKUP/100048", "Next"));

    ui->label_anprLogs->setText(GET_TEXT("ANPR/103057", "ANPR Logs"));
    ui->pushButton_go->setText(GET_TEXT("PLAYBACK/80065", "Go"));
    ui->pushButton_backupAll->setText(GET_TEXT("COMMONBACKUP/100002", "Backup All"));
    ui->pushButton_backup->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->pushButton_back_2->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->labelPlateColor->setText(GET_TEXT("ANPR/103099", "Plate Color"));
    ui->labelVehicleType->setText(GET_TEXT("ANPR/103100", "Vehicle Type"));
    ui->labelVehicleColor->setText(GET_TEXT("ANPR/103101", "Vehicle Color"));
    ui->labelVehicleSpeed->setText(GET_TEXT("ANPR/103102", "Vehicle Speed"));
    ui->labelDirection->setText(GET_TEXT("SMARTEVENT/55018", "Direction"));

    ui->toolButton_firstPage->setToolTip(GET_TEXT("PLAYBACK/80061", "First Page"));
    ui->toolButton_previousPage->setToolTip(GET_TEXT("PLAYBACK/80062", "Previous Page"));
    ui->toolButton_nextPage->setToolTip(GET_TEXT("PLAYBACK/80063", "Next Page"));
    ui->toolButton_lastPage->setToolTip(GET_TEXT("PLAYBACK/80064", "Last Page"));
}

void PageAnprSearch::onChannelCheckBoxClicked()
{
}

void PageAnprSearch::onSearchCanceled()
{
    if (m_isExporting) {
        m_isExportCancel = true;
        return;
    }

    if (m_progressSid >= 0) {
        sendMessage(REQUEST_FLAG_SEARCH_ANPR_BACKUP_CANCEL, (void *)&m_progressSid, sizeof(m_progressSid));
        m_progressSid = -1;
    }
}

void PageAnprSearch::on_pushButton_search_clicked()
{
    m_pageIndex = 0;
    m_pageCount = 0;
    m_searchedCount = 0;
    m_anprMap.clear();
    if (!ui->lineEditSpeed->checkValid()) {
        return;
    }

    const QList<int> &checkedList = ui->checkBoxGroup_channel->checkedList();
    if (checkedList.isEmpty()) {
        ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel"));
        return;
    }

    if (ui->lineEditSpeed->isVisible() && ui->lineEditSpeed->text().isEmpty()) {
        ShowMessageBox("Please enter the Speed.");
        return;
    }
    const QString &checkedMask = ui->checkBoxGroup_channel->checkedMask();
    const QString &licensePlate = ui->lineEdit_licensePlate->text();

    struct req_search_anpr_backup anpr_backup;
    memset(&anpr_backup, 0, sizeof(req_search_anpr_backup));
    snprintf(anpr_backup.chnMaskl, sizeof(anpr_backup.chnMaskl), "%s", checkedMask.toStdString().c_str());
    snprintf(anpr_backup.keyWord, sizeof(anpr_backup.keyWord), "%s", licensePlate.toStdString().c_str());
    anpr_backup.chnNum = checkedList.count();
    anpr_backup.filter.type = static_cast<ANPR_MODE_TYPE>(ui->comboBoxLicensePlateType->currentData().toInt());
    anpr_backup.filter.plateColor = static_cast<ANPR_COLOR>(ui->comboBoxPlateColor->currentData().toInt());
    anpr_backup.filter.vehicleType = static_cast<ANPR_VEHICLE>(ui->comboBoxVehicleType->currentData().toInt());
    anpr_backup.filter.vehicleColor = static_cast<ANPR_COLOR>(ui->comboBoxVehicleColor->currentData().toInt());
    anpr_backup.filter.speedType = static_cast<ANPR_SPEED_MODE>(ui->comboBoxSpeed->currentData().toInt());
    anpr_backup.filter.direction = static_cast<ANPR_DIRECTION>(ui->comboBoxDirection->currentData().toInt());
    if (anpr_backup.filter.speedType == ANPR_SPEED_MORE || anpr_backup.filter.speedType == ANPR_SPEED_LESS) {
        anpr_backup.filter.speed = ui->lineEditSpeed->text().toInt();
    }
    const QDateTime startDateTime(ui->dateEdit_start->date(), ui->timeEdit_start->time());
    const QDateTime endDateTime(ui->dateEdit_end->date(), ui->timeEdit_end->time());
    if (startDateTime > endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }
    snprintf(anpr_backup.pStartTime, sizeof(anpr_backup.pStartTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(anpr_backup.pEndTime, sizeof(anpr_backup.pEndTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());

    sendMessage(REQUEST_FLAG_SEARCH_ANPR_BACKUP, (void *)&anpr_backup, sizeof(struct req_search_anpr_backup));

    // m_progress->setProgressValue(0);
    // m_progress->showProgress();
}

void PageAnprSearch::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageAnprSearch::onItemClicked(int index)
{
    qDebug() << QString("AnprAnalysis::onItemClicked, index: %1, begin").arg(index);
    m_selectedItemIndex = index;

    AnprItemWidget *clickedItem = m_itemList.at(index);
    if (!clickedItem->isInfoVisible()) {
        qDebug() << QString("AnprAnalysis::onItemClicked, index: %1, return").arg(index);
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
    if (m_anprMap.contains(m_currentIndex)) {
        //更新大图信息
        updateBigPicture();
        //
        const AnprBackupInfo &info = m_anprMap.value(m_currentIndex);
        ui->label_anprChannel->setText(QString("%1: %2").arg(GET_TEXT("CHANNELMANAGE/30008", "Channel")).arg(info.channel + 1));
        ui->label_anprTime->setText(QString("%1: %2").arg(GET_TEXT("LOG/64005", "Time")).arg(info.dateTime.toString("yyyy-MM-dd HH:mm:ss")));
        ui->label_anprPlate->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103035", "License Plate")).arg(info.plate));
        ui->label_anprType->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103041", "Plate Type")).arg(info.typeString().isEmpty() ? "N/A" : info.typeString()));
        ui->labelAnprPlateColor->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103099", "Plate Color")).arg(info.plateColor));
        ui->labelAnprVehicleType->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103100", "Vehicle Type")).arg(info.vehicleType));
        ui->labelAnprVehicleBrand->setText(QString("%1: %2").arg(GET_TEXT("ANPR/169006", "Vehicle Brand")).arg(info.vehicleBrand));
        ui->labelAnprVehicleColor->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103101", "Vehicle Color")).arg(info.vehicleColor));
        if (info.vehicleSpeed < 0) {
            ui->labelAnprSpeed->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103102", "Vehicle Speed")).arg(GET_TEXT("TARGETMODE/103205", "N/A")));
        } else {
            ui->labelAnprSpeed->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103102", "Vehicle Speed")).arg(info.vehicleSpeed));
        }
        ui->label_anprRegion->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103013", "Country / Region")).arg(QString(info.region).isEmpty() ? "N/A" : info.region));
        ui->label_anprDirection->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103022", "Direction")).arg(directionString(info.direction)));
        ui->label_anprRoi->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103023", "ROI_ID")).arg(info.roiId == 0 ? QString("N/A") : QString::number(info.roiId)));
        //更新视频信息
        ui->label_playTime->clear();
        ui->timeSlider->setTimeRange(info.dateTime.addSecs(-10), info.dateTime.addSecs(10));
        ui->commonVideo->showCurrentChannel(info.channel);
    }

    //
    //closeWait();
    qDebug() << QString("AnprAnalysis::onItemClicked, index: %1, end").arg(index);

    if (m_allCount >= MAX_SEARCH_BACKUP_COUNT && m_isAboutToShowMessage) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100042", "The search results is over %1, and the first %1 will be displayed.").arg(MAX_SEARCH_BACKUP_COUNT));
        m_isAboutToShowMessage = false;
    }
}

void PageAnprSearch::onItemChecked(int index, bool checked)
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
        ui->checkBox_all->setCheckState(Qt::Unchecked);
    } else if (checkedCount == visibleCount) {
        ui->checkBox_all->setCheckState(Qt::Checked);
    } else {
        ui->checkBox_all->setCheckState(Qt::PartiallyChecked);
    }
}

void PageAnprSearch::on_checkBox_all_clicked(bool checked)
{
    if (ui->checkBox_all->checkState() == Qt::PartiallyChecked) {
        ui->checkBox_all->setCheckState(Qt::Checked);
    }

    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        if (item->isInfoVisible()) {
            item->setChecked(checked);
        }
    }
}

void PageAnprSearch::onRealPlaybackTime()
{
    m_currentTime = m_currentTime.addSecs(1);
    if (m_currentTime >= m_currentEndTime) {
        m_currentTime = m_currentEndTime;
        on_toolButton_play_clicked();
    }
    ui->label_playTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setCurrentDateTime(m_currentTime);
}

void PageAnprSearch::onSliderValueChanged(int value)
{
    m_currentTime = QDateTime::fromTime_t(value);
    ui->label_playTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));

    if (m_playSid < 0) {
        return;
    }

    seekLivePlayback(m_currentTime);
}

void PageAnprSearch::on_toolButton_stop_clicked()
{
    showVideoWait();

    stopCommonPlay();
    ui->commonVideo->showPixmap();

    //closeWait();
}

void PageAnprSearch::on_toolButton_play_clicked()
{
    ui->toolButton_play->setAttribute(Qt::WA_UnderMouse, false);
    ui->toolButton_play->clearFocus();
    if (m_backupList.isEmpty()) {
        showVideoWait();
        updateVideoInfo();
        //closeWait();
        if (m_backupList.isEmpty()) {
            ShowMessageBox(GET_TEXT("DISK/92033", "No record files currently."));
            return;
        }
    }

    const QString &strState = ui->toolButton_play->property("state").toString();
    if (strState == "play") {
        ui->toolButton_play->setToolTip(GET_TEXT("COMMONBACKUP/100046", "Pause"));
        ui->toolButton_play->setProperty("state", "pause");
        ui->toolButton_play->style()->unpolish(ui->toolButton_play);
        ui->toolButton_play->style()->polish(ui->toolButton_play);
        ui->toolButton_play->update();

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
            sendMessage(REQUEST_FLAG_PLAY_COM_BACKUP, (void *)&playinfo, sizeof(struct rep_play_common_backup));
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

        ui->toolButton_play->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
        ui->toolButton_play->setProperty("state", "play");
        ui->toolButton_play->style()->unpolish(ui->toolButton_play);
        ui->toolButton_play->style()->polish(ui->toolButton_play);
        ui->toolButton_play->update();

        sendMessage(REQUEST_FLAG_PLAYPAUSE_COM_BACKUP, &m_playSid, sizeof(int));
    }
}

void PageAnprSearch::on_toolButton_previous_clicked()
{
    int itemIndex = selectedItemIndex();
    if (itemIndex > 0) {
        stopCommonPlay();

        itemIndex--;
        onItemClicked(itemIndex);
    }
}

void PageAnprSearch::on_toolButton_next_clicked()
{
    int itemIndex = selectedItemIndex();
    if (itemIndex < 20 - 1) {
        stopCommonPlay();

        itemIndex++;
        onItemClicked(itemIndex);
    }
}

void PageAnprSearch::onPageEdited(const QString &text)
{
    Q_UNUSED(text)
}

void PageAnprSearch::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    stopAndClearCommonPlay();

    searchAnprPage(m_pageIndex);
}

void PageAnprSearch::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    stopAndClearCommonPlay();

    searchAnprPage(m_pageIndex);
}

void PageAnprSearch::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    stopAndClearCommonPlay();

    searchAnprPage(m_pageIndex);
}

void PageAnprSearch::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    stopAndClearCommonPlay();

    searchAnprPage(m_pageIndex);
}

void PageAnprSearch::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    if (m_pageIndex == page - 1) {
        return;
    }
    m_pageIndex = page - 1;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    stopAndClearCommonPlay();

    searchAnprPage(m_pageIndex);
}

void PageAnprSearch::on_pushButton_backupAll_clicked()
{
    QList<int> list;
    for (int i = 0; i < m_allCount; ++i) {
        list.append(i);
    }
    backupFile(list);
    MyFileSystemDialog::instance()->clearAnprSelected();
}

void PageAnprSearch::on_pushButton_backup_clicked()
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

void PageAnprSearch::on_pushButton_back_2_clicked()
{
    closePage();

    ui->stackedWidget->setCurrentIndex(0);
    ui->commonVideo->hideVideo();
    emit updateVideoGeometry();
}

void PageAnprSearch::seekLivePlayback(const QDateTime &dateTime)
{
    struct req_seek_time seekinfo;
    memset(&seekinfo, 0, sizeof(struct req_seek_time));
    seekinfo.chnid = m_currentChannel;
    seekinfo.sid = m_playSid;
    snprintf(seekinfo.pseektime, sizeof(seekinfo.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_PLAYSEEK_BACKUP, (void *)&seekinfo, sizeof(struct req_seek_time));
}

void PageAnprSearch::on_comboBoxSpeed_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    switch (ui->comboBoxSpeed->currentData().toInt()) {
    case ANPR_SPEED_MORE:
        ui->lineEditSpeed->setVisible(true);
        ui->labelSppedText->setVisible(true);
        break;
    case ANPR_SPEED_LESS:
        ui->lineEditSpeed->setVisible(true);
        ui->labelSppedText->setVisible(true);
        break;
    default:
        ui->lineEditSpeed->setVisible(false);
        ui->labelSppedText->setVisible(false);
        break;
    }
}
