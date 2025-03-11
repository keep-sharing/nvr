#include "PageCommonBackup.h"
#include "ui_PageCommonBackup.h"
#include "DownloadAnimate.h"
#include "DownloadPanel.h"
#include "EventLoop.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileModel.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"
#include <qmath.h>

extern "C" {

#include "msg.h"
}

const int BackupIndexRole = Qt::UserRole + 100;

PageCommonBackup::PageCommonBackup(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::CommonBackup)
{
    ui->setupUi(this);

    ui->checkBoxGroup_channel->setCountFromChannelName(qMsNvr->maxChannel());
    connect(ui->checkBoxGroup_channel, SIGNAL(checkBoxClicked()), this, SLOT(onChannelCheckBoxClicked()));

    ui->comboBox_streamType->clear();
    ui->comboBox_streamType->addItem(GET_TEXT("SYSTEMNETWORK/71208", "Primary Stream"), FILE_TYPE_MAIN);
    ui->comboBox_streamType->addItem(GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"), FILE_TYPE_SUB);

    ui->comboBox_recordType->clear();
    ui->comboBox_recordType->addItem(GET_TEXT("COMMON/1006", "All"), REC_EVENT_ALL);
    ui->comboBox_recordType->addItem(GET_TEXT("PLAYBACK/80053", "Continuous"), REC_EVENT_TIME);
    ui->comboBox_recordType->addItem(GET_TEXT("RECORDMODE/90013", "Event"), REC_EVENT_EVENT);
    ui->comboBox_recordType->addItem(GET_TEXT("VIDEOLOSS/50020", "Motion"), REC_EVENT_MOTION);
    ui->comboBox_recordType->addItem(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"), REC_EVENT_AUDIO_ALARM);
    ui->comboBox_recordType->addItem(GET_TEXT("PLAYBACK/80056", "Emergency"), REC_EVENT_MANUAL);
    ui->comboBox_recordType->addItem(GET_TEXT("SMARTEVENT/55000", "VCA"), REC_EVENT_VCA);
    ui->comboBox_recordType->addItem(GET_TEXT("ANPR/103054", "Smart Analysis"), REC_EVENT_SMART);
    ui->comboBox_recordType->addItem(GET_TEXT("ALARMIN/52001", "Alarm Input"), REC_EVENT_ALARMIN);

    ui->comboBox_fileType->clear();
    ui->comboBox_fileType->addItem(GET_TEXT("COMMON/1006", "All"), SEG_STATE_ALL);
    ui->comboBox_fileType->addItem(GET_TEXT("COMMONBACKUP/100005", "Locked"), SEG_STATE_LOCK);
    ui->comboBox_fileType->addItem(GET_TEXT("LIVEVIEW/20059", "Unlock"), SEG_STATE_NORMAL);

    //
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->toolButton_list, 0);
    buttonGroup->addButton(ui->toolButton_chart, 1);
    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupDisplayModeClicked(int)));

    //table list
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("DISK/92000", "Disk");
    headerList << GET_TEXT("COMMONBACKUP/100012", "Start Time-End Time");
    headerList << GET_TEXT("COMMON/1053", "Size");
    headerList << GET_TEXT("PTZCONFIG/36022", "Play");
    headerList << GET_TEXT("LIVEVIEW/20060", "Lock");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderChecked(bool)));
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnPlay, new ItemButtonDelegate(QPixmap(":/retrieve/retrieve/playbutton.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnLock, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonLock, this));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortableForColumn(ColumnPlay, false);
    ui->tableView->setSortableForColumn(ColumnLock, false);
    ui->tableView->setSortableForColumn(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortableForColumn(ColumnDisk, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnSize, SortFilterProxyModel::SortInt);

    //table chart
    connect(ui->widget_thumb, SIGNAL(itemClicked()), this, SLOT(onTableChartClicked()));
    connect(ui->widget_thumb, SIGNAL(stopPlay()), this, SLOT(on_toolButton_stop_clicked()));

    //slider
    connect(ui->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

    //
    connect(ui->toolButton_firstPage, SIGNAL(clicked(bool)), this, SLOT(onToolButtonFirstPageClicked()));
    connect(ui->toolButton_previousPage, SIGNAL(clicked(bool)), this, SLOT(onToolButtonPreviousPageClicked()));
    connect(ui->toolButton_nextPage, SIGNAL(clicked(bool)), this, SLOT(onToolButtonNextPageClicked()));
    connect(ui->toolButton_lastPage, SIGNAL(clicked(bool)), this, SLOT(onToolButtonLastPageClicked()));
    connect(ui->pushButton_go, SIGNAL(clicked(bool)), this, SLOT(onPushButtonGoClicked()));

    //
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onRealPlaybackTime()));
    m_timer->setInterval(1000);

    m_progress = new ProgressDialog(this);
    connect(m_progress, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_RETRIEVE, this);

    onLanguageChanged();
}

PageCommonBackup::~PageCommonBackup()
{
    delete ui;
}

void PageCommonBackup::initializeData()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget_mode->setCurrentIndex(0);

    m_displayMode = ModeList;
    ui->toolButton_list->setIcon(QIcon(":/retrieve/retrieve/list_checked.png"));
    ui->toolButton_chart->setIcon(QIcon(":/retrieve/retrieve/chart.png"));

    ui->checkBoxGroup_channel->clearCheck();
    ui->comboBox_streamType->setCurrentIndex(0);
    ui->comboBox_recordType->setCurrentIndex(0);
    ui->comboBox_fileType->setCurrentIndex(0);
    ui->lineEdit_time->clear();

    ui->dateEdit_start->setDate(QDate::currentDate());
    ui->timeEdit_start->setTime(QTime(0, 0));
    ui->dateEdit_end->setDate(QDate::currentDate());
    ui->timeEdit_end->setTime(QTime(23, 59, 59));
}

void PageCommonBackup::closePage()
{
    on_pushButton_back_2_clicked();
}

bool PageCommonBackup::isCloseable()
{
    return true;
}

bool PageCommonBackup::canAutoLogout()
{
    bool ok = true;
    if (m_timer->isActive()) {
        ok = false;
    }
    return ok;
}

NetworkResult PageCommonBackup::dealNetworkCommond(const QString &commond)
{
    if (ui->stackedWidget->currentIndex() == 0) {
        return NetworkReject;
    }

    qDebug() << "CommonBackup::dealNetworkCommond," << commond;

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

void PageCommonBackup::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_REC_RANGE:
        ON_RESPONSE_FLAG_GET_REC_RANGE(message);
        break;
    case RESPONSE_FLAG_SEARCH_COM_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(message);
        break;
    case RESPONSE_FLAG_GET_SEARCH_COM_PAGE:
        ON_RESPONSE_FLAG_GET_SEARCH_COM_PAGE(message);
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
    case RESPONSE_FLAG_LOCK_COM_BACKUP:
        ON_RESPONSE_FLAG_LOCK_COM_BACKUP(message);
        break;
    }
}

void PageCommonBackup::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        break;
    }
}

void PageCommonBackup::ON_RESPONSE_FLAG_GET_REC_RANGE(MessageReceive *message)
{
    struct resp_bkp_record_range *record_range = (struct resp_bkp_record_range *)message->data;
    if (record_range->startTime == (0x7fffffff) && record_range->endTime == (0x0)) {
        return;
    }
    ui->lineEdit_time->setText(GET_TEXT("COMMONBACKUP/100009", "From %1 To %2").arg(record_range->pStartTime).arg(record_range->pEndTime));
}

void PageCommonBackup::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message)
{
    m_searchSid = 0;

    //
    struct resp_search_common_backup *common_backup_array = (struct resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (!common_backup_array) {
        m_progress->hideProgress();
        qDebug() << QString("CommonBackup::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP, no matching video files.");
        ShowMessageBox(GET_TEXT("DISK/92026", "No matching video files."));
        return;
    }

    m_backupList.clear();
    for (int i = 0; i < count; ++i) {
        m_backupList.append(common_backup_array[i]);
    }

    const resp_search_common_backup &firstBackup = common_backup_array[0];
    //page
    m_pageCount = qCeil(firstBackup.allCnt / 100.0);
    m_pageIndex = 0;
    //第一次尽可能的向中心要数据，给的数据会是整数页
    int backupArrayPage = qCeil(count / 100.0);
    if (m_pageCount > backupArrayPage) {
        for (int i = backupArrayPage; i < m_pageCount; ++i) {
            struct rep_get_search_backup pageinfo;
            memset(&pageinfo, 0, sizeof(struct rep_get_search_backup));
            pageinfo.sid = firstBackup.sid;
            pageinfo.npage = i + 1;
            qDebug() << QString("REQUEST_FLAG_GET_SEARCH_COM_PAGE, page: %1").arg(pageinfo.npage);
            sendMessage(REQUEST_FLAG_GET_SEARCH_COM_PAGE, &pageinfo, sizeof(struct rep_get_search_backup));
            m_eventLoop.exec();
        }
    }

    //
    ui->label_size->setText(QString("%1%2").arg(GET_TEXT("COMMONBACKUP/100050", "Total Size: ")).arg(MyFileModel::fileSize(firstBackup.allSize)));
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(firstBackup.allCnt).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setMaxPage(m_pageCount);
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    m_progress->hideProgress();
    ui->stackedWidget->setCurrentIndex(1);
    m_isAbaoutToQuit = false;

    switch (m_displayMode) {
    case ModeList:
        updateTableList();
        break;
    case ModeChart:
        updateTableChart();
        break;
    }

    if (firstBackup.allCnt >= MAX_SEARCH_BACKUP_COUNT) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100042", "The search results is over %1, and the first %1 will be displayed.").arg(MAX_SEARCH_BACKUP_COUNT));
    }
}

void PageCommonBackup::ON_RESPONSE_FLAG_GET_SEARCH_COM_PAGE(MessageReceive *message)
{
    struct resp_search_common_backup *common_backup_array = (struct resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (common_backup_array) {
        for (int i = 0; i < count; ++i) {
            m_backupList.append(common_backup_array[i]);
        }
    }
    m_eventLoop.exit();
}

void PageCommonBackup::ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message)
{
    switch (m_displayMode) {
    case ModeList:
    case ModeChart: {
        if (m_playingSid >= 0) {
            return;
        }
        QPixmap pixmap = QPixmap::fromImage(message->image1);
        //pixmap.loadFromData((uchar *)message->data, message->header.size);
        ui->commonVideo->showPixmap(pixmap);
        break;
    }
    default:
        break;
    }
}

void PageCommonBackup::ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "CommonBackup::ON_RESPONSE_FLAG_PLAY_COM_BACKUP, data is null.";
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

void PageCommonBackup::ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message);

    m_timer->start();
}

void PageCommonBackup::ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)

    if (!m_isAbaoutToQuit) {
        updateSelectedInfo();
    }
    gEventLoopExit(0);
}

void PageCommonBackup::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message)
{
    if (!m_progress->isVisible()) {
        return;
    }

    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        qWarning() << "CommonBackup::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, data is null.";
        return;
    }
    qDebug() << QString("ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, %1").arg(progressinfo->percent);

    m_progress->setProgressValue(progressinfo->percent);
    m_searchSid = progressinfo->searchid;
}

void PageCommonBackup::ON_RESPONSE_FLAG_LOCK_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)

    //closeWait();
}

void PageCommonBackup::resizeEvent(QResizeEvent *event)
{
    //column width
    int w = width() / 3 * 2 - 250;
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnChannel, 100);
    ui->tableView->setColumnWidth(ColumnDisk, 100);
    ui->tableView->setColumnWidth(ColumnSize, 150);
    ui->tableView->setColumnWidth(ColumnPlay, 100);
    ui->tableView->setColumnWidth(ColumnTime, w - 400);

    QWidget::resizeEvent(event);
}

void PageCommonBackup::hideEvent(QHideEvent *event)
{
    on_pushButton_back_2_clicked();

    QWidget::hideEvent(event);
}

void PageCommonBackup::updateTableList()
{
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_backupList.size()).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));
    //
    on_toolButton_stop_clicked();
    ui->timeSlider->setRange(0, 0);
    m_currentRow = -1;
    ui->tableView->clearContent();

    int beginIndex = m_pageIndex * 100;
    int endIndex = beginIndex + 100;
    if (endIndex > m_backupList.size()) {
        endIndex = m_backupList.size();
    }
    qDebug() << QString("CommonBackup::updateTableList, beginIndex: %1, endIndex: %2").arg(beginIndex).arg(endIndex);
    ui->tableView->setRowCount(endIndex - beginIndex);
    int row = 0;
    for (int i = beginIndex; i < endIndex; ++i) {
        const resp_search_common_backup &backup = m_backupList.at(i);
        ui->tableView->setItemData(row, ColumnCheck, false, ItemCheckedRole);
        ui->tableView->setItemData(row, ColumnCheck, i, BackupIndexRole);
        ui->tableView->setItemIntValue(row, ColumnChannel, backup.chnid + 1);
        ui->tableView->setItemIntValue(row, ColumnDisk, backup.port);
        const QDateTime beginDateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        const QDateTime endDateTime = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
        if (beginDateTime.date() == endDateTime.date()) {
            ui->tableView->setItemText(row, ColumnTime, QString("%1-%2").arg(beginDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("HH:mm:ss")));
        } else {
            ui->tableView->setItemText(row, ColumnTime, QString("%1-%2").arg(beginDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss")));
        }
        ui->tableView->setItemBytesValue(row, ColumnSize, backup.size);
        ui->tableView->setItemData(row, ColumnLock, backup.isLock, ItemCheckedRole);
        row++;
    }
    ui->tableView->reorder();
    ui->tableView->selectRow(0);
    ui->tableView->setStyleSheet("border-bottom-width:1px");
    onItemClicked(0, ColumnChannel);
    updateCheckedSize();
    updateSelectedInfo();
}

void PageCommonBackup::updateTableChart()
{
    ui->widget_thumb->setCommonBackupList(m_backupList);
}

void PageCommonBackup::updateCheckedSize()
{
    qint64 bytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        const bool checked = ui->tableView->itemData(row, ColumnCheck, ItemCheckedRole).toBool();
        if (checked) {
            const int &index = ui->tableView->itemData(row, ColumnCheck, BackupIndexRole).toInt();
            const resp_search_common_backup &backup = m_backupList.at(index);
            bytes += backup.size;
        }
    }

    if (bytes > 0) {
        ui->label_size->setText(GET_TEXT("COMMONBACKUP/100037", "Total Size: %1         Select Size: %2").arg(MyFileModel::fileSize(m_backupList.first().allSize)).arg(MyFileModel::fileSize(bytes)));
    } else {
        ui->label_size->setText(QString("%1 %2").arg(GET_TEXT("COMMONBACKUP/100050", "Total Size: ")).arg(MyFileModel::fileSize(m_backupList.first().allSize)));
    }
}

void PageCommonBackup::updateSelectedInfo()
{
    if (m_selectedBackup.chnid < 0) {
        return;
    }

    switch (m_displayMode) {
    case ModeList:
    case ModeChart: {
        struct rep_play_common_backup playinfo;
        memset(&playinfo, 0, sizeof(struct rep_play_common_backup));
        playinfo.chnid = m_selectedBackup.chnid;
        playinfo.sid = m_selectedBackup.sid;
        playinfo.flag = 1;
        playinfo.enType = StreamType;
        snprintf(playinfo.pStartTime, sizeof(playinfo.pStartTime), "%s", m_selectedBackup.pStartTime);
        snprintf(playinfo.pEndTime, sizeof(playinfo.pEndTime), "%s", m_selectedBackup.pEndTime);
        sendMessage(REQUEST_FLAG_PLAY_COM_PICTURE, (void *)&playinfo, sizeof(struct rep_play_common_backup));
        break;
    }
    default:
        break;
    }

    m_currentChannel = m_selectedBackup.chnid;
    ui->commonVideo->showCurrentChannel(m_selectedBackup.chnid);
    QString strStarttime = GET_TEXT("COMMONBACKUP/100024", "Start Time:%1").arg(m_selectedBackup.pStartTime);
    QString strEndtime = GET_TEXT("COMMONBACKUP/100025", "End Time:%1").arg(m_selectedBackup.pEndTime);
    QString strFileSize = GET_TEXT("COMMONBACKUP/100026", "File Size:%1").arg(MyFileModel::fileSize(m_selectedBackup.size));
    ui->label_info->setText(QString("%1\n\n%2\n\n%3").arg(strStarttime).arg(strEndtime).arg(strFileSize));

    m_currentStartTime = QDateTime::fromString(m_selectedBackup.pStartTime, "yyyy-MM-dd HH:mm:ss");
    m_currentEndTime = QDateTime::fromString(m_selectedBackup.pEndTime, "yyyy-MM-dd HH:mm:ss");
    m_currentTime = m_currentStartTime;
    ui->label_playTime->setText(m_selectedBackup.pStartTime);
    ui->timeSlider->setTimeRange(m_currentStartTime, m_currentEndTime);
}

void PageCommonBackup::closeBackupSearch()
{
    if (!m_backupList.isEmpty()) {
        sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP_CLOSE, &m_backupList.first().sid, sizeof(int));
        m_backupList.clear();
    }
}

void PageCommonBackup::openAudio(int channel)
{
    struct req_set_audiochn audio;
    audio.chn = channel;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(audio));
}

void PageCommonBackup::closeAudio(int channel)
{
    Q_UNUSED(channel)

    struct req_set_audiochn audio;
    audio.chn = -1;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(audio));
}

void PageCommonBackup::updateTimeRange()
{
    ui->lineEdit_time->clear();
    const QString &checkedMask = ui->checkBoxGroup_channel->checkedMask();

    struct req_bkp_record_range record_range;
    memset(&record_range, 0, sizeof(struct req_bkp_record_range));
    snprintf(record_range.chnMaskl, sizeof(record_range.chnMaskl), "%s", checkedMask.toStdString().c_str());
    record_range.type = ui->comboBox_streamType->currentData().toInt();
    sendMessage(REQUEST_FLAG_GET_REC_RANGE, &record_range, sizeof(struct req_bkp_record_range));
}

void PageCommonBackup::onLanguageChanged()
{
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channels"));
    ui->label_time->setText(GET_TEXT("LOG/64005", "Time"));
    ui->label_streamType->setText(GET_TEXT("EVENTBACKUP/101001", "Stream Type"));
    ui->label_recordType->setText(GET_TEXT("RECORDMODE/90024", "Record Type"));
    ui->label_fileType->setText(GET_TEXT("COMMONBACKUP/100001", "File Type"));
    ui->label_startTime->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("LOG/64002", "End Time"));

    ui->checkBoxGroup_channel->onLanguageChanged();

    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->toolButton_list->setToolTip(GET_TEXT("RETRIEVE/96001", "List"));
    ui->toolButton_chart->setToolTip(GET_TEXT("RETRIEVE/96002", "Chart"));

    ui->toolButton_stop->setToolTip(GET_TEXT("COMMONBACKUP/100044", "Stop"));
    ui->toolButton_play->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
    ui->toolButton_previous->setToolTip(GET_TEXT("COMMONBACKUP/100047", "Previous"));
    ui->toolButton_next->setToolTip(GET_TEXT("COMMONBACKUP/100048", "Next"));

    ui->toolButton_firstPage->setToolTip(GET_TEXT("PLAYBACK/80061", "First Page"));
    ui->toolButton_previousPage->setToolTip(GET_TEXT("PLAYBACK/80062", "Previous Page"));
    ui->toolButton_nextPage->setToolTip(GET_TEXT("PLAYBACK/80063", "Next Page"));
    ui->toolButton_lastPage->setToolTip(GET_TEXT("PLAYBACK/80064", "Last Page"));

    ui->pushButton_go->setText(GET_TEXT("PLAYBACK/80065", "Go"));
    ui->pushButton_backupAll->setText(GET_TEXT("COMMONBACKUP/100002", "Backup All"));
    ui->pushButton_backup->setText(GET_TEXT("PLAYBACK/80045", "Backup"));
    ui->pushButton_back_2->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->toolButton_firstPage->setToolTip(GET_TEXT("PLAYBACK/80061", "First Page"));
    ui->toolButton_previousPage->setToolTip(GET_TEXT("PLAYBACK/80062", "Previous Page"));
    ui->toolButton_nextPage->setToolTip(GET_TEXT("PLAYBACK/80063", "Next Page"));
    ui->toolButton_lastPage->setToolTip(GET_TEXT("PLAYBACK/80064", "Last Page"));
}

void PageCommonBackup::onChannelCheckBoxClicked()
{
    updateTimeRange();
}

void PageCommonBackup::on_comboBox_streamType_activated(int index)
{
    Q_UNUSED(index)

    updateTimeRange();
}

void PageCommonBackup::onItemClicked(int row, int column)
{
    on_toolButton_stop_clicked();
    //
    m_currentRow = row;
    const int &index = ui->tableView->itemData(row, ColumnCheck, BackupIndexRole).toInt();
    resp_search_common_backup &backup = m_backupList[index];
    if (memcmp(&m_selectedBackup, &backup, sizeof(resp_search_common_backup)) != 0) {
        memcpy(&m_selectedBackup, &backup, sizeof(resp_search_common_backup));
        updateSelectedInfo();
    }
    //
    switch (column) {
    case ColumnCheck: {
        updateCheckedSize();
        break;
    }
    case ColumnPlay: {
        if (m_playingSid >= 0) {
            on_toolButton_stop_clicked();
        }
        on_toolButton_play_clicked();
        break;
    }
    case ColumnLock: {
        bool isLock = ui->tableView->itemData(row, column, ItemCheckedRole).toBool();
        if (isLock) {
            if (MessageBox::question(this, GET_TEXT("COMMONBACKUP/100034", "Record file may be overwritten after unlocking, continue?")) == MessageBox::Cancel) {
                return;
            }
        }

        struct rep_lock_common_backup lockinfo;
        memset(&lockinfo, 0, sizeof(struct rep_lock_common_backup));
        lockinfo.chnid = backup.chnid;
        lockinfo.sid = backup.sid;
        lockinfo.size = backup.size;
        lockinfo.isLock = ((backup.isLock) ? 0 : 1);
        snprintf(lockinfo.pStartTime, sizeof(lockinfo.pStartTime), "%s", backup.pStartTime);
        snprintf(lockinfo.pEndTime, sizeof(lockinfo.pEndTime), "%s", backup.pEndTime);
        sendMessage(REQUEST_FLAG_LOCK_COM_BACKUP, (void *)&lockinfo, sizeof(struct rep_lock_common_backup));

        execWait();

        backup.isLock = lockinfo.isLock;
        ui->tableView->setItemData(row, column, !isLock, ItemCheckedRole);
        break;
    }
    default:
        break;
    }
}

void PageCommonBackup::onTableHeaderChecked(bool checked)
{
    Q_UNUSED(checked)

    updateCheckedSize();
}

void PageCommonBackup::onSearchCanceled()
{
    if (m_searchSid >= 0) {
        sendMessageOnly(REQUEST_FLAG_SEARCH_COM_BACKUP_CANCEL, &m_searchSid, sizeof(int));
        m_searchSid = -1;
    }
}

void PageCommonBackup::onRealPlaybackTime()
{
    m_currentTime = m_currentTime.addSecs(1);
    if (m_currentTime >= m_currentEndTime) {
        m_currentTime = m_currentEndTime;
        on_toolButton_play_clicked();
    }
    ui->label_playTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setCurrentDateTime(m_currentTime);
}

void PageCommonBackup::onSliderValueChanged(int value)
{
    if (m_playingSid < 0) {
        return;
    }

    m_currentTime = QDateTime::fromTime_t(value);
    ui->label_playTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));

    seekLivePlayback(m_currentTime);
}

void PageCommonBackup::on_pushButton_search_clicked()
{
    return;
    m_pageIndex = 0;
    m_pageCount = 0;
    ui->stackedWidget_mode->setCurrentIndex(0);

    const QList<int> &checkedList = ui->checkBoxGroup_channel->checkedList();
    if (checkedList.isEmpty()) {
        ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel"));
        return;
    }
    const QString &checkedMask = ui->checkBoxGroup_channel->checkedMask();

    struct req_search_common_backup common_backup;
    memset(&common_backup, 0, sizeof(req_search_common_backup));
    snprintf(common_backup.chnMaskl, sizeof(common_backup.chnMaskl), "%s", checkedMask.toStdString().c_str());
    common_backup.chnNum = checkedList.count();
    common_backup.enType = ui->comboBox_streamType->currentData().toInt();
    common_backup.enEvent = ui->comboBox_recordType->currentData().toInt();
    common_backup.enState = ui->comboBox_fileType->currentData().toInt();
    const QDateTime startDateTime(ui->dateEdit_start->date(), ui->timeEdit_start->time());
    const QDateTime endDateTime(ui->dateEdit_end->date(), ui->timeEdit_end->time());
    if (startDateTime > endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }
    snprintf(common_backup.pStartTime, sizeof(common_backup.pStartTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(common_backup.pEndTime, sizeof(common_backup.pEndTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    common_backup.all = MF_YES;

    sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP, &common_backup, sizeof(struct req_search_common_backup));

    m_progress->setProgressValue(0);
    m_progress->showProgress();

    m_selectedBackup.chnid = -1;

    StreamType = common_backup.enType;
    ui->widget_thumb->StreamType = common_backup.enType;
}

void PageCommonBackup::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageCommonBackup::onButtonGroupDisplayModeClicked(int id)
{
    switch (id) {
    case 0:
        m_displayMode = ModeList;
        ui->toolButton_list->setIcon(QIcon(":/retrieve/retrieve/list_checked.png"));
        ui->toolButton_chart->setIcon(QIcon(":/retrieve/retrieve/chart.png"));
        ui->stackedWidget_mode->setCurrentIndex(0);
        updateTableList();
        break;
    case 1:
        m_displayMode = ModeChart;
        ui->toolButton_list->setIcon(QIcon(":/retrieve/retrieve/list.png"));
        ui->toolButton_chart->setIcon(QIcon(":/retrieve/retrieve/chart_checked.png"));
        ui->stackedWidget_mode->setCurrentIndex(1);
        updateTableChart();
        break;
    default:
        break;
    }
}

void PageCommonBackup::onTableChartClicked()
{
    on_toolButton_stop_clicked();
    //
    resp_search_common_backup backup = ui->widget_thumb->selectedCommonBakcup();
    if (memcmp(&m_selectedBackup, &backup, sizeof(resp_search_common_backup)) != 0) {
        memcpy(&m_selectedBackup, &backup, sizeof(resp_search_common_backup));
        updateSelectedInfo();
    }
}

void PageCommonBackup::onToolButtonFirstPageClicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;

    on_toolButton_stop_clicked();

    updateTableList();
}

void PageCommonBackup::onToolButtonPreviousPageClicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;

    on_toolButton_stop_clicked();

    updateTableList();
}

void PageCommonBackup::onToolButtonNextPageClicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;

    on_toolButton_stop_clicked();

    updateTableList();
}

void PageCommonBackup::onToolButtonLastPageClicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;

    on_toolButton_stop_clicked();

    updateTableList();
}

void PageCommonBackup::onPushButtonGoClicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;

    on_toolButton_stop_clicked();

    updateTableList();
}

void PageCommonBackup::on_toolButton_stop_clicked()
{
    if (m_playingSid < 0) {
        return;
    }

    MsWaittingContainer wait(ui->commonVideo);
    m_timer->stop();

    const QString &strState = ui->toolButton_play->property("state").toString();
    if (strState == "pause") {
        ui->toolButton_play->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
        ui->toolButton_play->setProperty("state", "play");
        ui->toolButton_play->style()->unpolish(ui->toolButton_play);
        ui->toolButton_play->style()->polish(ui->toolButton_play);
        ui->toolButton_play->update();
    }

    sendMessage(REQUEST_FLAG_PLAYSTOP_COM_BACKUP, &m_playingSid, sizeof(int));
    gEventLoopExec();
    m_playingSid = -1;
}

void PageCommonBackup::on_toolButton_play_clicked()
{
    const QString &strState = ui->toolButton_play->property("state").toString();
    if (strState == "play") {
        ui->toolButton_play->setToolTip(GET_TEXT("COMMONBACKUP/100046", "Pause"));
        ui->toolButton_play->setProperty("state", "pause");
        ui->toolButton_play->style()->unpolish(ui->toolButton_play);
        ui->toolButton_play->style()->polish(ui->toolButton_play);
        ui->toolButton_play->update();

        if (m_playingSid < 0) {
            m_playingSid = m_selectedBackup.sid;
            ui->commonVideo->hidePixmap();
            ui->commonVideo->playbackVideo(m_selectedBackup.chnid);
            const QDateTime &startDateTime = QDateTime::fromString(m_selectedBackup.pStartTime, "yyyy-MM-dd HH:mm:ss");
            m_currentTime = startDateTime;

            struct rep_play_common_backup playinfo;
            memset(&playinfo, 0, sizeof(struct rep_play_common_backup));
            playinfo.chnid = m_selectedBackup.chnid;
            playinfo.sid = m_selectedBackup.sid;
            playinfo.enType = StreamType;
            snprintf(playinfo.pStartTime, sizeof(playinfo.pStartTime), "%s", m_selectedBackup.pStartTime);
            snprintf(playinfo.pEndTime, sizeof(playinfo.pEndTime), "%s", m_selectedBackup.pEndTime);
            sendMessage(REQUEST_FLAG_PLAY_COM_BACKUP, &playinfo, sizeof(struct rep_play_common_backup));
        } else {
            if (m_currentTime >= m_currentEndTime) {
                //play from start time
                m_currentTime = m_currentStartTime;
                immediatelyUpdatePlayTime();
                seekLivePlayback(m_currentTime);
            }
            sendMessage(REQUEST_FLAG_PLAYRESTART_COM_BACKUP, &m_playingSid, sizeof(int));
        }
    } else if (strState == "pause") {
        m_timer->stop();

        ui->toolButton_play->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
        ui->toolButton_play->setProperty("state", "play");
        ui->toolButton_play->style()->unpolish(ui->toolButton_play);
        ui->toolButton_play->style()->polish(ui->toolButton_play);
        ui->toolButton_play->update();

        sendMessage(REQUEST_FLAG_PLAYPAUSE_COM_BACKUP, &m_playingSid, sizeof(int));
    }
}

void PageCommonBackup::on_toolButton_previous_clicked()
{
    switch (m_displayMode) {
    case ModeList:
        if (m_currentRow > 0) {
            m_currentRow--;
            ui->tableView->selectRow(m_currentRow);
            onItemClicked(m_currentRow, ColumnChannel);
        }
        break;
    case ModeChart:
        ui->widget_thumb->selectPrevious();
        break;
    default:
        break;
    }
}

void PageCommonBackup::on_toolButton_next_clicked()
{
    switch (m_displayMode) {
    case ModeList:
        if (m_currentRow < ui->tableView->rowCount() - 1) {
            m_currentRow++;
            ui->tableView->selectRow(m_currentRow);
            onItemClicked(m_currentRow, ColumnChannel);
        }
        break;
    case ModeChart:
        ui->widget_thumb->selectNext();
        break;
    default:
        break;
    }
}

void PageCommonBackup::on_pushButton_backupAll_clicked()
{
    //
    if (m_backupList.isEmpty()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        return;
    }

    //
    const QString &directory = MyFileSystemDialog::instance()->exportVideoWithAnimate();
    if (directory.isEmpty()) {
        return;
    }
    const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int &fileType = MyFileSystemDialog::instance()->fileType();

    //
    qint64 totalBytes = 0;
    QList<rep_export_common_backup> backupList;
    for (int i = 0; i < m_backupList.size(); ++i) {
        const resp_search_common_backup &searchBackup = m_backupList.at(i);
        totalBytes += searchBackup.size;

        rep_export_common_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_common_backup));
        exportBackup.chnid = searchBackup.chnid;
        exportBackup.sid = searchBackup.sid;
        exportBackup.filesize = searchBackup.size;
        exportBackup.fileno = searchBackup.fileno;
        exportBackup.filetype = fileType;
        snprintf(exportBackup.pStartTime, sizeof(exportBackup.pStartTime), "%s", searchBackup.pStartTime);
        snprintf(exportBackup.pEndTime, sizeof(exportBackup.pEndTime), "%s", searchBackup.pEndTime);
        snprintf(exportBackup.dev_name, sizeof(exportBackup.dev_name), "%s", deviceName.toStdString().c_str());
        snprintf(exportBackup.dev_path, sizeof(exportBackup.dev_path), "%s", directory.toStdString().c_str());

        backupList.append(exportBackup);
    }

    if (totalBytes > deviceFreeSize) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

#if 1
    for (int i = 0; i < backupList.size(); ++i) {
        const rep_export_common_backup &exportBackup = backupList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid = gDownload->availableId();
        auto_backup.chnid = exportBackup.chnid;
        auto_backup.enType = ui->comboBox_streamType->currentData().toInt();
        auto_backup.filetype = exportBackup.filetype;
        auto_backup.filesize = exportBackup.filesize;
        auto_backup.fileno = exportBackup.fileno;
        snprintf(auto_backup.pStartTime, sizeof(auto_backup.pStartTime), "%s", exportBackup.pStartTime);
        snprintf(auto_backup.pEndTime, sizeof(auto_backup.pEndTime), "%s", exportBackup.pEndTime);
        snprintf(auto_backup.dev_name, sizeof(auto_backup.dev_name), "%s", exportBackup.dev_name);
        snprintf(auto_backup.dev_path, sizeof(auto_backup.dev_path), "%s", exportBackup.dev_path);

        char fileName[128];
        ////get_auto_backup_vedio_filename(&auto_backup, fileName, sizeof(fileName));
        snprintf(auto_backup.filename, sizeof(auto_backup.filename), "%s", fileName);

        gDownload->appendItem(auto_backup);
    }
#else
    m_export->setExportCommonBackup(backupList);
    m_export->show();
#endif
}

void PageCommonBackup::on_pushButton_backup_clicked()
{
    //
    QList<resp_search_common_backup> selectedBackupList;
    switch (m_displayMode) {
    case ModeList:
        for (int i = 0; i < ui->tableView->rowCount(); ++i) {
            const bool &checked = ui->tableView->itemData(i, ColumnCheck, ItemCheckedRole).toBool();
            if (checked) {
                const int &index = ui->tableView->itemData(i, ColumnCheck, BackupIndexRole).toInt();
                const resp_search_common_backup &searchBackup = m_backupList.at(index);
                selectedBackupList.append(searchBackup);
            }
        }
        break;
    case ModeChart:
        selectedBackupList = ui->widget_thumb->checkedCommonBackupList();
        break;
    default:
        break;
    }
    if (selectedBackupList.isEmpty()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        return;
    }

    //
    const QString &directory = MyFileSystemDialog::instance()->exportVideoWithAnimate();
    if (directory.isEmpty()) {
        return;
    }
    const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int &fileType = MyFileSystemDialog::instance()->fileType();

    //
    qint64 totalBytes = 0;
    QList<rep_export_common_backup> backupList;
    for (int i = 0; i < selectedBackupList.size(); ++i) {
        const resp_search_common_backup &searchBackup = selectedBackupList.at(i);
        totalBytes += searchBackup.size;

        rep_export_common_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_common_backup));
        exportBackup.chnid = searchBackup.chnid;
        exportBackup.sid = searchBackup.sid;
        exportBackup.filesize = searchBackup.size;
        exportBackup.fileno = searchBackup.fileno;
        exportBackup.filetype = fileType;
        snprintf(exportBackup.pStartTime, sizeof(exportBackup.pStartTime), "%s", searchBackup.pStartTime);
        snprintf(exportBackup.pEndTime, sizeof(exportBackup.pEndTime), "%s", searchBackup.pEndTime);
        snprintf(exportBackup.dev_name, sizeof(exportBackup.dev_name), "%s", deviceName.toStdString().c_str());
        snprintf(exportBackup.dev_path, sizeof(exportBackup.dev_path), "%s", directory.toStdString().c_str());

        backupList.append(exportBackup);
    }

    if (totalBytes > deviceFreeSize) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

#if 1
    for (int i = 0; i < backupList.size(); ++i) {
        const rep_export_common_backup &exportBackup = backupList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid = gDownload->availableId();
        auto_backup.chnid = exportBackup.chnid;
        auto_backup.enType = ui->comboBox_streamType->currentData().toInt();
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
#else
    m_export->setExportCommonBackup(backupList);
    m_export->show();
#endif
}

void PageCommonBackup::on_pushButton_back_2_clicked()
{
    ui->widget_thumb->setPageIndex(0);
    if (ui->stackedWidget_mode->currentIndex() == 1) {
        m_displayMode = ModeList;
        ui->toolButton_list->setIcon(QIcon(":/retrieve/retrieve/list_checked.png"));
        ui->toolButton_chart->setIcon(QIcon(":/retrieve/retrieve/chart.png"));
    }
    m_isAbaoutToQuit = true;

    on_toolButton_stop_clicked();
    closeBackupSearch();

    ui->stackedWidget->setCurrentIndex(0);
    ui->commonVideo->hideVideo();
    emit updateVideoGeometry();
}

void PageCommonBackup::seekLivePlayback(const QDateTime &dateTime)
{
    struct req_seek_time seekinfo;
    memset(&seekinfo, 0, sizeof(struct req_seek_time));
    seekinfo.chnid = m_currentChannel;
    seekinfo.sid = m_playingSid;
    snprintf(seekinfo.pseektime, sizeof(seekinfo.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_PLAYSEEK_BACKUP, (void *)&seekinfo, sizeof(struct req_seek_time));
}

void PageCommonBackup::immediatelyUpdatePlayTime()
{
    ui->label_playTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setCurrentDateTime(m_currentTime);
}
