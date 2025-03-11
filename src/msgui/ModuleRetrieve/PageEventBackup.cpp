#include "PageEventBackup.h"
#include "ui_PageEventBackup.h"
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

}

const int BackupIndexRole = Qt::UserRole + 100;
const int AlarmKeyRole    = Qt::UserRole + 101;

PageEventBackup::PageEventBackup(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::EventBackup)
{
    ui->setupUi(this);

    //
    ui->comboBox_mainType->clear();
    ui->comboBox_mainType->addItem(GET_TEXT("MOTION/51000", "Motion Detection"), INFO_MAJOR_MOTION);
    ui->comboBox_mainType->addItem(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"), INFO_MAJOR_AUDIO_ALARM);
    ui->comboBox_mainType->addItem(GET_TEXT("ALARMIN/52001", "Alarm Input"), INFO_MAJOR_ALARMIN);
    ui->comboBox_mainType->addItem(GET_TEXT("SMARTEVENT/55000", "VCA"), INFO_MAJOR_VCA);
    ui->comboBox_mainType->addItem(GET_TEXT("ANPR/103054", "Smart Analysis"), INFO_MAJOR_SMART);

    //
    ui->comboBoxDetectionObject->clear();
    ui->comboBoxDetectionObject->beginEdit();
    ui->comboBoxDetectionObject->addItem(GET_TEXT("COMMON/1006", "All"), DETEC_ALL);
    ui->comboBoxDetectionObject->addItem(GET_TEXT("TARGETMODE/103201", "Human"), DETEC_HUMAN);
    ui->comboBoxDetectionObject->addItem(GET_TEXT("TARGETMODE/103202", "Vehicle"), DETEC_VEHICLE);
    ui->comboBoxDetectionObject->endEdit();

    //
    ui->checkBoxGroup_channel->setCountFromChannelName(qMsNvr->maxChannel());
    connect(ui->checkBoxGroup_channel, SIGNAL(checkBoxClicked()), this, SLOT(onChannelCheckBoxClicked()));

    //
    ui->comboBox_streamType->clear();
    ui->comboBox_streamType->addItem(GET_TEXT("SYSTEMNETWORK/71208", "Primary Stream"), FILE_TYPE_MAIN);
    ui->comboBox_streamType->addItem(GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"), FILE_TYPE_SUB);

    //
    ui->comboBox_prePlayback->clear();
    ui->comboBox_prePlayback->addItem(QString("0s"), 0);
    ui->comboBox_prePlayback->addItem(QString("5s"), 5);
    ui->comboBox_prePlayback->addItem(QString("10s"), 10);
    ui->comboBox_prePlayback->addItem(QString("30s"), 30);
    ui->comboBox_prePlayback->addItem(QString("60s"), 60);
    ui->comboBox_prePlayback->addItem(QString("120s"), 120);
    ui->comboBox_prePlayback->addItem(QString("300s"), 300);
    ui->comboBox_prePlayback->addItem(QString("600s"), 600);

    //
    ui->comboBox_postPlayback->clear();
    ui->comboBox_postPlayback->addItem(QString("5s"), 5);
    ui->comboBox_postPlayback->addItem(QString("10s"), 10);
    ui->comboBox_postPlayback->addItem(QString("30s"), 30);
    ui->comboBox_postPlayback->addItem(QString("60s"), 60);
    ui->comboBox_postPlayback->addItem(QString("120s"), 120);
    ui->comboBox_postPlayback->addItem(QString("300s"), 300);
    ui->comboBox_postPlayback->addItem(QString("600s"), 600);

    //
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->toolButton_list, 0);
    buttonGroup->addButton(ui->toolButton_chart, 1);
    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupDisplayModeClicked(int)));

    //table list
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("EVENTBACKUP/101006", "Source");
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("PLAYBACK/80105", "Disk");
    headerList << GET_TEXT("EVENTBACKUP/101009", "Event Duration Time");
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
    ui->tableView->setSortableForColumn(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortableForColumn(ColumnDisk, SortFilterProxyModel::SortInt);

    //alarmin
    auto allAlarminNames = qMsNvr->allAlarminNames();

    QStringList alarmHeaders;
    alarmHeaders << "";
    alarmHeaders << GET_TEXT("ALARMIN/52002", "Alarm Input NO.");
    alarmHeaders << GET_TEXT("EVENTBACKUP/101004", "Alarm Input Name");
    ui->tableView_alarm->setHorizontalHeaderLabels(alarmHeaders);
    ui->tableView_alarm->setColumnCount(alarmHeaders.size());
    ui->tableView_alarm->setSortingEnabled(false);
    ui->tableView_alarm->setRowCount(allAlarminNames.size());
    int row = 0;
    for (auto iter = allAlarminNames.constBegin(); iter != allAlarminNames.constEnd(); ++iter) {
        const AlarmKey &key = iter.key();

        ui->tableView_alarm->setItemText(row, 1, key.numberName());
        ui->tableView_alarm->setItemData(row, 1, QVariant::fromValue(key), AlarmKeyRole);
        ui->tableView_alarm->setItemText(row, 2, key.name());
        row++;
    }

    //table chart
    connect(ui->widget_thumb, SIGNAL(itemClicked()), this, SLOT(onTableChartClicked()));
    connect(ui->widget_thumb, SIGNAL(stopPlay()), this, SLOT(on_toolButtonStop_clicked()));

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

    //
    m_progress = new ProgressDialog(this);
    connect(m_progress, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));

    m_control = new CommonBackupControl(this);
    ui->widget_thumb->setControl(m_control);

    ui->toolButtonPause->hide();

    //
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_RETRIEVE, this);

    onLanguageChanged();
}

PageEventBackup::~PageEventBackup()
{
    delete ui;
}

void PageEventBackup::initializeData()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget_mode->setCurrentIndex(0);

    m_displayMode = ModeList;
    ui->toolButton_list->setIcon(QIcon(":/retrieve/retrieve/list_checked.png"));
    ui->toolButton_chart->setIcon(QIcon(":/retrieve/retrieve/chart.png"));

    ui->comboBox_mainType->setCurrentIndex(0);
    on_comboBox_mainType_activated(0);

    ui->checkBoxGroup_channel->clearCheck();
    ui->comboBox_streamType->setCurrentIndex(0);
    ui->comboBox_prePlayback->setCurrentIndexFromData(0);
    ui->comboBox_postPlayback->setCurrentIndexFromData(10);

    ui->dateEdit_start->setDate(QDate::currentDate());
    ui->timeEdit_start->setTime(QTime(0, 0));
    ui->dateEdit_end->setDate(QDate::currentDate());
    ui->timeEdit_end->setTime(QTime(23, 59, 59));

    ui->tableView_alarm->clearCheck();
    ui->tableView_alarm->clearSelection();
    ui->tableView_alarm->setHeaderChecked(false);
    ui->tableView_alarm->scrollToTop();
}

void PageEventBackup::closePage()
{
    on_pushButton_back_2_clicked();
}

bool PageEventBackup::isCloseable()
{
    return true;
}

bool PageEventBackup::canAutoLogout()
{
    bool ok = true;
    if (m_timer->isActive()) {
        ok = false;
    }
    return ok;
}

NetworkResult PageEventBackup::dealNetworkCommond(const QString &commond)
{
    if (ui->stackedWidget->currentIndex() == 0) {
        return NetworkReject;
    }

    qDebug() << "EventBackup::dealNetworkCommond," << commond;

    NetworkResult result = NetworkReject;
    if (commond.startsWith("Video_Play")) {
        QMetaObject::invokeMethod(this, "on_toolButtonPlay_clicked", Qt::QueuedConnection);
        result = NetworkAccept;
    } else if (commond.startsWith("Video_Pause")) {
        QMetaObject::invokeMethod(this, "on_toolButtonPause_clicked", Qt::QueuedConnection);
        result = NetworkAccept;
    } else if (commond.startsWith("Video_Stop")) {
        QMetaObject::invokeMethod(this, "on_toolButtonStop_clicked", Qt::QueuedConnection);
        result = NetworkAccept;
    } else if (commond.startsWith("Dial_Insid_Add")) {
        //选中下一个
        QMetaObject::invokeMethod(this, "on_toolButtonNext_clicked", Qt::QueuedConnection);
        result = NetworkAccept;
    } else if (commond.startsWith("Dial_Insid_Sub")) {
        //选中上一个
        QMetaObject::invokeMethod(this, "on_toolButtonPrevious_clicked", Qt::QueuedConnection);
        result = NetworkAccept;
    }

    return result;
}

void PageEventBackup::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_EVT_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(message);
        break;
    case RESPONSE_FLAG_GET_SEARCH_EVT_PAGE:
        ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(message);
        break;
    }
}

void PageEventBackup::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        break;
    }
}

void PageEventBackup::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message)
{
    m_searchSid = 0;

    //
    struct resp_search_event_backup *event_backup_array = (struct resp_search_event_backup *)message->data;
    int count                                           = message->header.size / sizeof(struct resp_search_event_backup);

    if (!event_backup_array) {
        m_progress->hideProgress();
        qDebug() << QString("EventBackup::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP, no matching video files.");
        ShowMessageBox(GET_TEXT("DISK/92026", "No matching video files."));
        return;
    }

    m_backupList.clear();
    for (int i = 0; i < count; ++i) {
        m_backupList.append(event_backup_array[i]);
    }

    const resp_search_event_backup &firstBackup = event_backup_array[0];
    m_pageCount = qCeil(firstBackup.allCnt / 100.0);
    m_pageIndex = 0;
    int backupArrayPage = qCeil(count / 100.0);
    if (m_pageCount > backupArrayPage) {
        for (int i = backupArrayPage; i < m_pageCount; ++i) {
            struct rep_get_search_backup pageinfo;
            memset(&pageinfo, 0, sizeof(struct rep_get_search_backup));
            pageinfo.sid   = firstBackup.sid;
            pageinfo.npage = i + 1;
            qDebug() << QString("REQUEST_FLAG_GET_SEARCH_EVT_PAGE, page: %1").arg(pageinfo.npage);
            sendMessage(REQUEST_FLAG_GET_SEARCH_EVT_PAGE, &pageinfo, sizeof(struct rep_get_search_backup));
            m_eventLoop.exec();
        }
    }
    //
    ui->label_size->setText("");
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

void PageEventBackup::ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(MessageReceive *message)
{
    struct resp_search_event_backup *event_backup_array = (struct resp_search_event_backup *)message->data;
    int count                                           = message->header.size / sizeof(struct resp_search_event_backup);

    if (event_backup_array) {
        for (int i = 0; i < count; ++i) {
            m_backupList.append(event_backup_array[i]);
        }
    }
    m_eventLoop.exit();
}

void PageEventBackup::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message)
{
    if (!m_progress->isVisible()) {
        return;
    }

    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        qWarning() << "EventBackup::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, data is null.";
        return;
    }
    qDebug() << QString("ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, %1").arg(progressinfo->percent);

    m_progress->setProgressValue(progressinfo->percent);
    m_searchSid = progressinfo->searchid;
}

void PageEventBackup::resizeEvent(QResizeEvent *event)
{
    //column width
    int w = width() / 3 * 2 - 150;
    ui->tableView->setColumnWidth(ColumnCheck, 80);
    ui->tableView->setColumnWidth(ColumnSource, 130);
    ui->tableView->setColumnWidth(ColumnChannel, 130);
    ui->tableView->setColumnWidth(ColumnDisk, 130);
    ui->tableView->setColumnWidth(ColumnTime, w - 470);

    ui->tableView_alarm->setColumnWidth(0, 50);
    ui->tableView_alarm->setColumnWidth(1, 300);

    QWidget::resizeEvent(event);
}

void PageEventBackup::hideEvent(QHideEvent *event)
{
    on_pushButton_back_2_clicked();

    QWidget::hideEvent(event);
}

void PageEventBackup::updateTableList()
{
    MsWaittingContainer wait(ui->commonVideo);

    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_backupList.size()).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));
    //
    stopPlay();
    ui->timeSlider->setRange(0, 0);
    m_currentRow = -1;
    ui->tableView->clearContent();

    int beginIndex = m_pageIndex * 100;
    int endIndex   = beginIndex + 100;
    if (endIndex > m_backupList.size()) {
        endIndex = m_backupList.size();
    }
    //qDebug() << QString("EventBackup::updateTableList, beginIndex: %1, endIndex: %2").arg(beginIndex).arg(endIndex);
    ui->tableView->setRowCount(endIndex - beginIndex);
    int row = 0;
    for (int i = beginIndex; i < endIndex; ++i) {
        const resp_search_event_backup &backup = m_backupList.at(i);
        ui->tableView->setItemData(row, ColumnCheck, false, ItemCheckedRole);
        ui->tableView->setItemData(row, ColumnCheck, i, BackupIndexRole);
        ui->tableView->setItemText(row, ColumnSource, backup.source);
        ui->tableView->setItemIntValue(row, ColumnChannel, backup.chnid + 1);
        ui->tableView->setItemIntValue(row, ColumnDisk, backup.port);
        const QDateTime beginDateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        const QDateTime endDateTime   = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
        if (beginDateTime.date() == endDateTime.date()) {
            ui->tableView->setItemText(row, ColumnTime, QString("%1-%2").arg(beginDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("HH:mm:ss")));
        } else {
            ui->tableView->setItemText(row, ColumnTime, QString("%1-%2").arg(beginDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss")));
        }
        row++;
    }
    ui->tableView->reorder();
    ui->tableView->selectRow(0);
    ui->tableView->setStyleSheet("border-bottom-width:1px");
    onItemClicked(0, ColumnChannel);
    updateCheckedSize();
    updateSelectedInfo();
}

void PageEventBackup::updateTableChart()
{
    ui->widget_thumb->setEventBackupList(m_backupList);
}

void PageEventBackup::updateCheckedSize()
{
    qint64 bytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        const bool checked = ui->tableView->itemData(row, ColumnCheck, ItemCheckedRole).toBool();
        if (checked) {
            const int &index                       = ui->tableView->itemData(row, ColumnCheck, BackupIndexRole).toInt();
            const resp_search_event_backup &backup = m_backupList.at(index);
            bytes += backup.size;
        }
    }

}

void PageEventBackup::updateSelectedInfo()
{
    if (m_selectedBackup.chnid < 0) {
        return;
    }

    switch (m_displayMode) {
    case ModeList:
    case ModeChart: {
        QDateTime begin = QDateTime::fromString(m_selectedBackup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        QDateTime end   = QDateTime::fromString(m_selectedBackup.pEndTime, "yyyy-MM-dd HH:mm:ss");
        m_control->waitForSearchCommonBackup(m_selectedBackup.chnid, begin, end, StreamType);
        m_control->waitForSearchCommonBackupPicture();
        ui->commonVideo->showPixmap(m_control->pixmap());
        break;
    }
    default:
        break;
    }

    m_currentChannel   = m_selectedBackup.chnid;
    m_currentStartTime = m_control->realStartDateTime();
    m_currentEndTime   = m_control->realEndDateTime();
    m_currentTime      = m_currentStartTime;

    QString strStarttime = GET_TEXT("COMMONBACKUP/100024", "Start Time:%1").arg(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
    QString strEndtime   = GET_TEXT("COMMONBACKUP/100025", "End Time:%1").arg(m_currentEndTime.toString("yyyy-MM-dd HH:mm:ss"));
    QString strFileSize  = GET_TEXT("COMMONBACKUP/100026", "File Size:%1").arg(MyFileModel::fileSize(m_control->fileSize()));
    ui->commonVideo->showCurrentChannel(m_selectedBackup.chnid);
    ui->label_info->setText(QString("%1\n\n%2\n\n%3").arg(strStarttime).arg(strEndtime).arg(strFileSize));
    ui->label_playTime->setText(m_currentStartTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setTimeRange(m_currentStartTime, m_currentEndTime);
}

void PageEventBackup::initializeVcaSubType()
{
    ui->comboBox_subType->clear();
    ui->comboBox_subType->beginEdit();
    ui->comboBox_subType->addItem(GET_TEXT("COMMON/1006", "All"), INFO_MINOR_ALL_0);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55001", "Region Entrance"), INFO_MINOR_REGIONIN);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55002", "Region Exiting"), INFO_MINOR_REGIONOUT);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55003", "Advanced Motion Detection"), INFO_MINOR_ADVANCED_MOTION);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55004", "Tamper Detection"), INFO_MINOR_TAMPER);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55005", "Line Crossing"), INFO_MINOR_LINECROSS);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55006", "Loitering"), INFO_MINOR_LOITERING);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55007", "Human Detection"), INFO_MINOR_HUMAN);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55055", "Object Left/Removed"), INFO_MINOR_OBJECT);
    ui->comboBox_subType->endEdit();
}

void PageEventBackup::initializeSmartSubType()
{
    ui->comboBox_subType->clear();
    ui->comboBox_subType->addItem(GET_TEXT("COMMON/1006", "All"), INFO_MINOR_ALL);
    if (qMsNvr->isSupportTargetMode()) {
        ui->comboBox_subType->addItem("ANPR", INFO_MINOR_LPR);
    }
    if (qMsNvr->isSupportFaceDetection()) {
        ui->comboBox_subType->addItem(GET_TEXT("FACE/141000", "Face Detection"), INFO_MINOR_FACE);
    }
    ui->comboBox_subType->addItem("POS", INFO_MINOR_POS);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55008", "People Counting"), INFO_MINOR_PCNT);
    ui->comboBox_subType->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/103313", "Regional People Counting"), INFO_MINOR_REGION);
}

void PageEventBackup::closeBackupSearch()
{
    if (!m_backupList.isEmpty()) {
        m_backupList.clear();
    }
}

void PageEventBackup::openAudio(int channel)
{
    struct req_set_audiochn audio;
    audio.chn        = channel;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(audio));
}

void PageEventBackup::closeAudio(int channel)
{
    Q_UNUSED(channel)

    struct req_set_audiochn audio;
    audio.chn        = -1;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(audio));
}

void PageEventBackup::onLanguageChanged()
{
    ui->label_mainType->setText(GET_TEXT("LOG/64003", "Main Type"));
    ui->label_subType->setText(GET_TEXT("LOG/64004", "Sub Type"));
    ui->labelDetectionObject->setText(GET_TEXT("TARGETMODE/103200", "Detection Object"));
    ui->label_streamType->setText(GET_TEXT("EVENTBACKUP/101001", "Stream Type"));
    ui->label_startTime->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("LOG/64002", "End Time"));
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channels"));
    ui->label_prePlayback->setText(GET_TEXT("EVENTBACKUP/101002", "Pre Playback"));
    ui->label_postPlayback->setText(GET_TEXT("EVENTBACKUP/101003", "Post Playback"));

    ui->checkBoxGroup_channel->onLanguageChanged();

    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->toolButton_list->setToolTip(GET_TEXT("RETRIEVE/96001", "List"));
    ui->toolButton_chart->setToolTip(GET_TEXT("RETRIEVE/96002", "Chart"));

    ui->toolButtonStop->setToolTip(GET_TEXT("COMMONBACKUP/100044", "Stop"));
    ui->toolButtonPlay->setToolTip(GET_TEXT("COMMONBACKUP/100045", "Play"));
    ui->toolButtonPrevious->setToolTip(GET_TEXT("COMMONBACKUP/100047", "Previous"));
    ui->toolButtonNext->setToolTip(GET_TEXT("COMMONBACKUP/100048", "Next"));

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

void PageEventBackup::onChannelCheckBoxClicked()
{
}

void PageEventBackup::onItemClicked(int row, int column)
{
    MsWaittingContainer wait(ui->commonVideo);

    stopPlay();
    //
    m_currentRow       = row;
    const int &index   = ui->tableView->itemData(row, ColumnCheck, BackupIndexRole).toInt();
    const auto &backup = m_backupList.at(index);
    if (memcmp(&m_selectedBackup, &backup, sizeof(resp_search_event_backup)) != 0) {
        memcpy(&m_selectedBackup, &backup, sizeof(resp_search_event_backup));
        updateSelectedInfo();
    }
    //
    switch (column) {
    case ColumnCheck: {
        updateCheckedSize();
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

void PageEventBackup::onTableHeaderChecked(bool checked)
{
    Q_UNUSED(checked)

    updateCheckedSize();
}

void PageEventBackup::onSearchCanceled()
{
    if (m_searchSid >= 0) {
        sendMessageOnly(REQUEST_FLAG_SEARCH_EVT_BACKUP_CANCEL, &m_searchSid, sizeof(int));
        m_searchSid = -1;
    }
}

void PageEventBackup::onRealPlaybackTime()
{
    m_currentTime = m_currentTime.addSecs(1);

    QDateTime dateTime = m_control->getPlaybackRealTime();
    if (dateTime.isValid() && dateTime > m_currentTime) {
        m_currentTime = dateTime;
    }

    if (m_currentTime >= m_currentEndTime) {
        m_currentTime = m_currentEndTime;
        on_toolButtonPause_clicked();
    }
    ui->label_playTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setCurrentDateTime(m_currentTime);
}

void PageEventBackup::onSliderValueChanged(int value)
{
    QDateTime time = QDateTime::fromTime_t(value);

    m_currentTime = time;
    ui->label_playTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));

    m_control->waitForSeekCommonBackup(m_currentTime);
}

void PageEventBackup::on_comboBox_mainType_activated(int index)
{
    const INFO_MAJOR_EN &type = static_cast<INFO_MAJOR_EN>(ui->comboBox_mainType->itemData(index).toInt());
    switch (type) {
    case INFO_MAJOR_MOTION:
    case INFO_MAJOR_AUDIO_ALARM:
        ui->label_subType->hide();
        ui->comboBox_subType->hide();
        ui->labelDetectionObject->hide();
        ui->comboBoxDetectionObject->hide();
        ui->widget_channel->show();
        ui->tableView_alarm->hide();
        break;
    case INFO_MAJOR_ALARMIN:
        ui->label_subType->hide();
        ui->comboBox_subType->hide();
        ui->labelDetectionObject->hide();
        ui->comboBoxDetectionObject->hide();
        ui->widget_channel->hide();
        ui->tableView_alarm->show();
        break;
    case INFO_MAJOR_VCA:
        ui->label_subType->show();
        ui->comboBox_subType->show();
        ui->widget_channel->show();
        ui->tableView_alarm->hide();
        ui->comboBoxDetectionObject->setCurrentIndex(0);
        initializeVcaSubType();
        ui->comboBox_subType->reSetIndex();
        break;
    case INFO_MAJOR_SMART:
        ui->label_subType->show();
        ui->comboBox_subType->show();
        ui->labelDetectionObject->hide();
        ui->comboBoxDetectionObject->hide();
        ui->widget_channel->show();
        ui->tableView_alarm->hide();
        initializeSmartSubType();
        break;
    default:
        break;
    }
}

void PageEventBackup::on_comboBox_subType_indexSet(int index)
{
    int mainType = ui->comboBox_mainType->currentData().toInt();
    if (mainType == INFO_MAJOR_VCA) {
        int enMinor = ui->comboBox_subType->itemData(index).toInt();
        switch (enMinor) {
        case INFO_MINOR_ALL_0:
        case INFO_MINOR_REGIONIN:
        case INFO_MINOR_REGIONOUT:
        case INFO_MINOR_ADVANCED_MOTION:
        case INFO_MINOR_LINECROSS:
        case INFO_MINOR_LOITERING:
            ui->labelDetectionObject->show();
            ui->comboBoxDetectionObject->show();
            break;
        default:
            ui->labelDetectionObject->hide();
            ui->comboBoxDetectionObject->hide();
            break;
        }
    } else {
        ui->labelDetectionObject->hide();
        ui->comboBoxDetectionObject->hide();
    }
}

void PageEventBackup::on_pushButton_search_clicked()
{
    return;
    m_pageIndex = 0;
    m_pageCount = 0;
    ui->stackedWidget_mode->setCurrentIndex(0);

    const QDateTime startDateTime(ui->dateEdit_start->date(), ui->timeEdit_start->time());
    const QDateTime endDateTime(ui->dateEdit_end->date(), ui->timeEdit_end->time());
    if (startDateTime > endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }

    //
    struct req_search_event_backup event_backup;
    memset(&event_backup, 0, sizeof(req_search_event_backup));

    struct ms_socket_packet ms_packet;
    memset(&ms_packet, 0, sizeof(struct ms_socket_packet));

    const INFO_MAJOR_EN &type = static_cast<INFO_MAJOR_EN>(ui->comboBox_mainType->currentData().toInt());
    switch (type) {
    case INFO_MAJOR_MOTION:
    case INFO_MAJOR_AUDIO_ALARM:
    case INFO_MAJOR_VCA:
    case INFO_MAJOR_SMART: {
        const QList<int> &checkedList = ui->checkBoxGroup_channel->checkedList();
        if (checkedList.isEmpty()) {
            ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel"));
            return;
        }
        const QString &checkedMask = ui->checkBoxGroup_channel->checkedMask();
        snprintf(event_backup.chnMaskl, sizeof(event_backup.chnMaskl), "%s", checkedMask.toStdString().c_str());
        event_backup.chnNum = checkedList.count();

        if (type == INFO_MAJOR_VCA || type == INFO_MAJOR_SMART) {
            event_backup.enMinor = ui->comboBox_subType->currentData().toInt();
        }
        if (type == INFO_MAJOR_VCA) {
            event_backup.objtype = static_cast<DETEC_OBJ_EN>(ui->comboBoxDetectionObject->currentData().toInt());
        }

        ms_packet.nSize       = sizeof(struct req_search_event_backup);
        ms_packet.nHeaderSize = ms_packet.nSize;
        ms_packet.nBodySize   = 0;

        ms_packet.packet       = (char *)ms_malloc(ms_packet.nSize);
        ms_packet.packetHeader = ms_packet.packet;
        ms_packet.packetBody   = NULL;
        break;
    }
    case INFO_MAJOR_ALARMIN: {
        QList<AlarmKey> checkedList;
        for (int i = 0; i < ui->tableView_alarm->rowCount(); ++i) {
            const bool &checked = ui->tableView_alarm->itemData(i, 0, ItemCheckedRole).toInt();
            if (checked) {
                const AlarmKey &key = ui->tableView_alarm->itemData(i, 1, AlarmKeyRole).value<AlarmKey>();
                checkedList.append(key);
            }
        }
        if (checkedList.isEmpty()) {
            ShowMessageBox(GET_TEXT("EVENTBACKUP/101008", "Please select at least one Alarm Input No."));
            return;
        }
        event_backup.pEventNameLen = MAX_LEN_64;
        event_backup.enEventCnt    = checkedList.count();

        ms_packet.nHeaderSize = sizeof(struct req_search_event_backup);
        ms_packet.nBodySize   = event_backup.pEventNameLen * event_backup.enEventCnt;
        ms_packet.nSize       = ms_packet.nHeaderSize + ms_packet.nBodySize;

        ms_packet.packet       = (char *)ms_malloc(ms_packet.nSize);
        ms_packet.packetHeader = ms_packet.packet;
        ms_packet.packetBody   = ms_packet.packetHeader + ms_packet.nHeaderSize;

        for (int i = 0; i < checkedList.count(); ++i) {
            const AlarmKey &alarm = checkedList.at(i);
            snprintf(ms_packet.packetBody, event_backup.pEventNameLen, "%s", alarm.nameForSearch().toStdString().c_str());
            ms_packet.packetBody += event_backup.pEventNameLen;
        }
        break;
    }
    default:
        break;
    }
    event_backup.enType  = ui->comboBox_streamType->currentData().toInt();
    event_backup.enMajor = type;
    event_backup.ahead   = 0;
    event_backup.delay   = 0;

    snprintf(event_backup.pStartTime, sizeof(event_backup.pStartTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(event_backup.pEndTime, sizeof(event_backup.pEndTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    event_backup.all = MF_YES;
    event_backup.close = MF_YES;

    memcpy(ms_packet.packet, &event_backup, sizeof(struct req_search_event_backup));
    qMsDebug() << "\n----REQUEST_FLAG_SEARCH_EVT_BACKUP----"
               << "\n----chnMaskl:" << event_backup.chnMaskl
               << "\n----chnNum:" << event_backup.chnNum
               << "\n----enType:" << event_backup.enType
               << "\n----enMajor:" << event_backup.enMajor
               << "\n----object:" << event_backup.objtype
               << "\n----ahead:" << event_backup.ahead
               << "\n----delay:" << event_backup.delay
               << "\n----pStartTime:" << event_backup.pStartTime
               << "\n----pEndTime:" << event_backup.pEndTime;
    sendMessage(REQUEST_FLAG_SEARCH_EVT_BACKUP, (void *)ms_packet.packet, ms_packet.nSize);

    if (ms_packet.packet) {
        ms_free(ms_packet.packet);
    }

    m_progress->setProgressValue(0);
    m_progress->showProgress();

    m_selectedBackup.chnid = -1;

    StreamType                   = event_backup.enType;
    ui->widget_thumb->StreamType = event_backup.enType;

    m_control->setPreSec(ui->comboBox_prePlayback->currentData().toInt());
    m_control->setPostSec(ui->comboBox_postPlayback->currentData().toInt());
}

void PageEventBackup::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageEventBackup::onButtonGroupDisplayModeClicked(int id)
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

void PageEventBackup::onTableChartClicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    stopPlay();
    //
    resp_search_event_backup backup = ui->widget_thumb->selectedEventBakcup();
    if (memcmp(&m_selectedBackup, &backup, sizeof(resp_search_event_backup)) != 0) {
        memcpy(&m_selectedBackup, &backup, sizeof(resp_search_event_backup));
        updateSelectedInfo();
    }
}

void PageEventBackup::onToolButtonFirstPageClicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;

    stopPlay();

    updateTableList();
}

void PageEventBackup::onToolButtonPreviousPageClicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;

    stopPlay();

    updateTableList();
}

void PageEventBackup::onToolButtonNextPageClicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;

    stopPlay();

    updateTableList();
}

void PageEventBackup::onToolButtonLastPageClicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;

    stopPlay();

    updateTableList();
}

void PageEventBackup::onPushButtonGoClicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;

    stopPlay();

    updateTableList();
}

void PageEventBackup::on_toolButtonStop_clicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    stopPlay();
}

void PageEventBackup::on_toolButtonPlay_clicked()
{
    if (!m_control->hasBackup()) {
        return;
    }

    MsWaittingContainer wait(ui->commonVideo);

    if (m_control->isPlaying()) {
        if (m_currentTime >= m_currentEndTime) {
            m_currentTime = m_currentStartTime;
            m_control->waitForSeekCommonBackup(m_currentTime);
        }
        m_control->waitForResumeCommonBackup();
    } else {
        ui->commonVideo->hidePixmap();
        ui->commonVideo->playbackVideo(m_currentChannel);
        if (m_control->waitForPlayCommonBackup() == -2) {
            //搜索后磁盘被移除了等导致录像不存在。
            MessageBox::information(this, "Stream is invalid.");
            return;
        }
        m_currentTime = m_currentStartTime;
        immediatelyUpdatePlayTime();
    }

    m_timer->start();
    ui->commonVideo->setPosPaused(false);

    ui->toolButtonPlay->hide();
    ui->toolButtonPause->show();
}

void PageEventBackup::on_toolButtonPause_clicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    if (m_control->waitForPauseCommonBackup() < 0) {
        return;
    }

    m_timer->stop();
    ui->toolButtonPlay->show();
    ui->toolButtonPause->hide();
}

void PageEventBackup::on_toolButtonPrevious_clicked()
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

void PageEventBackup::on_toolButtonNext_clicked()
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

void PageEventBackup::on_pushButton_backupAll_clicked()
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
    const QString &deviceName    = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int &fileType          = MyFileSystemDialog::instance()->fileType();

    //
    qint64 totalBytes = 0;
    QList<rep_export_event_backup> backupList;
    for (int i = 0; i < m_backupList.size(); ++i) {
        const resp_search_event_backup &searchBackup = m_backupList.at(i);
        totalBytes += searchBackup.size;

        rep_export_event_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_event_backup));
        exportBackup.chnid    = searchBackup.chnid;
        exportBackup.sid      = searchBackup.sid;
        exportBackup.filesize = searchBackup.size;
        exportBackup.filetype = fileType;
        QDateTime begin = QDateTime::fromString(searchBackup.pStartTime, "yyyy-MM-dd HH:mm:ss").addSecs(-m_control->preSec());
        QDateTime end   = QDateTime::fromString(searchBackup.pEndTime, "yyyy-MM-dd HH:mm:ss").addSecs(m_control->postSec());
        snprintf(exportBackup.pStartTime, sizeof(exportBackup.pStartTime), "%s", begin.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        snprintf(exportBackup.pEndTime, sizeof(exportBackup.pEndTime), "%s", end.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
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
        const rep_export_event_backup &exportBackup = backupList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid      = gDownload->availableId();
        auto_backup.chnid    = exportBackup.chnid;
        auto_backup.enType   = ui->comboBox_streamType->currentData().toInt();
        auto_backup.filetype = exportBackup.filetype;
        auto_backup.filesize = exportBackup.filesize;
        auto_backup.fileno   = exportBackup.fileno;
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
    m_export->setExportEventBackup(backupList);
    m_export->show();
#endif
}

void PageEventBackup::on_pushButton_backup_clicked()
{
    //
    QList<resp_search_event_backup> selectedBackupList;
    switch (m_displayMode) {
    case ModeList:
        for (int i = 0; i < ui->tableView->rowCount(); ++i) {
            const bool &checked = ui->tableView->itemData(i, ColumnCheck, ItemCheckedRole).toBool();
            if (checked) {
                const int &index                             = ui->tableView->itemData(i, ColumnCheck, BackupIndexRole).toInt();
                const resp_search_event_backup &searchBackup = m_backupList.at(index);
                selectedBackupList.append(searchBackup);
            }
        }
        break;
    case ModeChart:
        selectedBackupList = ui->widget_thumb->checkedEventBackupList();
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
    const QString &deviceName    = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int &fileType          = MyFileSystemDialog::instance()->fileType();

    //
    qint64 totalBytes = 0;
    QList<rep_export_event_backup> backupList;
    for (int i = 0; i < selectedBackupList.size(); ++i) {
        const resp_search_event_backup &searchBackup = selectedBackupList.at(i);
        totalBytes += searchBackup.size;

        rep_export_event_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_event_backup));
        exportBackup.chnid    = searchBackup.chnid;
        exportBackup.sid      = searchBackup.sid;
        exportBackup.filesize = searchBackup.size;
        exportBackup.filetype = fileType;
        QDateTime begin = QDateTime::fromString(searchBackup.pStartTime, "yyyy-MM-dd HH:mm:ss").addSecs(-m_control->preSec());
        QDateTime end   = QDateTime::fromString(searchBackup.pEndTime, "yyyy-MM-dd HH:mm:ss").addSecs(m_control->postSec());
        snprintf(exportBackup.pStartTime, sizeof(exportBackup.pStartTime), "%s", begin.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        snprintf(exportBackup.pEndTime, sizeof(exportBackup.pEndTime), "%s", end.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
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
        const rep_export_event_backup &exportBackup = backupList.at(i);

        //
        req_auto_backup auto_backup;
        memset(&auto_backup, 0, sizeof(req_auto_backup));
        auto_backup.sid      = gDownload->availableId();
        auto_backup.chnid    = exportBackup.chnid;
        auto_backup.enType   = ui->comboBox_streamType->currentData().toInt();
        auto_backup.filetype = exportBackup.filetype;
        auto_backup.filesize = exportBackup.filesize;
        auto_backup.fileno   = exportBackup.fileno;
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
    m_export->setExportEventBackup(backupList);
    m_export->show();
#endif
}

void PageEventBackup::on_pushButton_back_2_clicked()
{
    MsWaittingContainer wait(ui->commonVideo);

    m_isAbaoutToQuit = true;

    ui->widget_thumb->setPageIndex(0);
    if (ui->stackedWidget_mode->currentIndex() == 1) {
        m_displayMode = ModeList;
        ui->toolButton_list->setIcon(QIcon(":/retrieve/retrieve/list_checked.png"));
        ui->toolButton_chart->setIcon(QIcon(":/retrieve/retrieve/chart.png"));
    }

    stopPlay();
    m_control->waitForCloseCommonBackup();
    closeBackupSearch();

    ui->stackedWidget->setCurrentIndex(0);
    ui->commonVideo->hideVideo();
    emit updateVideoGeometry();
}

void PageEventBackup::immediatelyUpdatePlayTime()
{
    ui->label_playTime->setText(m_currentTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->timeSlider->setCurrentDateTime(m_currentTime);
}

void PageEventBackup::stopPlay()
{
    //先把定时器停止，避免stop消息和pause消息同时发送
    m_timer->stop();

    if (m_control->waitForStopCommonBackup() < 0) {
        return;
    }

    ui->commonVideo->showPixmap();

    ui->toolButtonPlay->show();
    ui->toolButtonPause->hide();

    m_currentTime = m_currentStartTime;
    immediatelyUpdatePlayTime();
}
