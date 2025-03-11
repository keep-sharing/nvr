#include "PagePictureBackup.h"
#include "ui_PagePictureBackup.h"
#include "DownloadAnimate.h"
#include "DownloadPanel.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "MyFileModel.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"
#include <qmath.h>

extern "C" {

}

const int BackupIndexRole = Qt::UserRole + 100;
//const int AlarmKeyRole = Qt::UserRole + 101;

PagePictureBackup::PagePictureBackup(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PictureBackup)
{
    ui->setupUi(this);

    ui->checkBoxGroup_channel->setCountFromChannelName(qMsNvr->maxChannel());

    ui->comboBox_pictureType->clear();
    ui->comboBox_pictureType->addItem(GET_TEXT("COMMON/1006", "All"), INFO_MAJOR_PB_PIC | INFO_MAJOR_LIVE_PIC | INFO_MAJOR_TIME_PIC | INOF_MAJOR_EVENT_PIC);
    ui->comboBox_pictureType->addItem(GET_TEXT("PICTUREBACKUP/102000", "Live View Snapshot"), INFO_MAJOR_LIVE_PIC);
    ui->comboBox_pictureType->addItem(GET_TEXT("PICTUREBACKUP/102001", "Playback Snapshot"), INFO_MAJOR_PB_PIC);
    if (qMsNvr->isSupportContinuousSnapshot()) {
        ui->comboBox_pictureType->addItem(GET_TEXT("PICTUREBACKUP/102006", "Continuous Snapshot"), INFO_MAJOR_TIME_PIC);
    }
    if (qMsNvr->isSupportEventSnapshot()) {
        ui->comboBox_pictureType->addItem(GET_TEXT("PICTUREBACKUP/102007", "Event Snapshot"), INOF_MAJOR_EVENT_PIC);
    }

    ui->comboBox_subType->addItem(GET_TEXT("COMMON/1006", "All"), INFO_MINOR_PIC_EVENT);
    ui->comboBox_subType->addItem(GET_TEXT("MOTION/51000", "Motion Detection"), INFO_MINOR_PIC_MOTION);
    ui->comboBox_subType->addItem(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"), INFO_MINOR_PIC_AUDIO_ALARM);
    ui->comboBox_subType->addItem(GET_TEXT("ALARMIN/52001", "Alarm Input"), INFO_MINOR_PIC_ALARM);
    ui->comboBox_subType->addItem(GET_TEXT("SMARTEVENT/55000", "VCA"), INFO_MINOR_PIC_VCA);
    if (!qMsNvr->is3536c()) {
        ui->comboBox_subType->addItem(GET_TEXT("ANPR/103054", "Smart Analysis"), INFO_MINOR_PIC_LPR);
    }

    ui->label_subType->hide();
    ui->comboBox_subType->hide();
    ui->label_tertiaryType->hide();
    ui->comboBox_tertiaryType->hide();
#if 0
    //alarmin
    auto allAlarminNames = qMsNvr->allAlarminNames();

    QStringList alarmHeaders;
    alarmHeaders << "";
    alarmHeaders << GET_TEXT("ALARMIN/52002","Alarm Input NO.");
    alarmHeaders << GET_TEXT("EVENTBACKUP/101004","Alarm Input Name");
    ui->tableView_alarm->setHorizontalHeaderLabels(alarmHeaders);
    ui->tableView_alarm->setColumnCount(alarmHeaders.size());
    ui->tableView_alarm->setSortingEnabled(false);
    ui->tableView_alarm->setRowCount(allAlarminNames.size());
    int row = 0;
    for (auto iter = allAlarminNames.constBegin(); iter != allAlarminNames.constEnd(); ++iter)
    {
        const AlarmKey &key = iter.key();

        ui->tableView_alarm->setItemText(row, 1, key.numberName());
        ui->tableView_alarm->setItemData(row, 1, QVariant::fromValue(key), AlarmKeyRole);
        ui->tableView_alarm->setItemText(row, 2, key.name());
        row++;
    }
    ui->tableView_alarm->setColumnWidth(0, 50);
    ui->tableView_alarm->setColumnWidth(1, 300);
#endif
    ui->tableView_alarm->hide();

    //initialize table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("PLAYBACK/80105", "Disk");
    headerList << GET_TEXT("IMAGE/37300", "Time");
    headerList << GET_TEXT("COMMON/1053", "Size");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderChecked(bool)));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortableForColumn(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortableForColumn(ColumnDisk, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnSize, SortFilterProxyModel::SortInt);

    //progress
    m_progress = new ProgressDialog(this);
    connect(m_progress, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));


    //
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_RETRIEVE, this);

    onLanguageChanged();
}

PagePictureBackup::~PagePictureBackup()
{
    delete ui;
}

void PagePictureBackup::initializeData()
{
    ui->stackedWidget->setCurrentIndex(0);

    ui->comboBox_pictureType->setCurrentIndex(0);
    on_comboBox_pictureType_activated(0);

    ui->dateEdit_start->setDate(QDate::currentDate());
    ui->timeEdit_start->setTime(QTime(0, 0));
    ui->dateEdit_end->setDate(QDate::currentDate());
    ui->timeEdit_end->setTime(QTime(23, 59, 59));

    ui->checkBoxGroup_channel->clearCheck();
}

void PagePictureBackup::closePage()
{
    on_pushButton_back_2_clicked();
}

bool PagePictureBackup::isCloseable()
{
    return true;
}

bool PagePictureBackup::canAutoLogout()
{
    return true;
}

NetworkResult PagePictureBackup::dealNetworkCommond(const QString &commond)
{
    Q_UNUSED(commond)

    NetworkResult result = NetworkReject;

    if (commond.startsWith("Dial_Insid_Add")) {
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

void PagePictureBackup::processMessage(MessageReceive *message)
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
    }
}

void PagePictureBackup::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        break;
    }
}

void PagePictureBackup::ON_RESPONSE_FLAG_SEARCH_PIC_BACKUP(MessageReceive *message)
{
    m_searchSid = 0;

    //
    struct resp_search_picture_backup *picture_backup_array = (struct resp_search_picture_backup *)message->data;
    if (!picture_backup_array) {
        m_progress->hideProgress();
        ShowMessageBox(GET_TEXT("PICTUREBACKUP/102002", "No matching picture files."));
        return;
    }
    int count = message->header.size / sizeof(struct resp_search_picture_backup);
    qMsDebug() << "all count:" << picture_backup_array->allCnt;

    m_backupList.clear();
    for (int i = 0; i < count; ++i) {
        m_backupList.append(picture_backup_array[i]);
    }
    const resp_search_picture_backup &firstBackup = picture_backup_array[0];
    //page
    m_pageCount = qCeil(firstBackup.allCnt / 100.0);
    m_pageIndex = 0;
    if (m_pageCount > 1) {
        for (int i = 1; i < m_pageCount; ++i) {
            struct rep_get_search_backup pageinfo;
            memset(&pageinfo, 0, sizeof(struct rep_get_search_backup));
            pageinfo.sid = firstBackup.sid;
            pageinfo.npage = i + 1;
            qDebug() << QString("REQUEST_FLAG_GET_SEARCH_COM_PAGE, page: %1").arg(pageinfo.npage);
            sendMessage(REQUEST_FLAG_GET_SEARCH_PIC_PAGE, (void *)&pageinfo, sizeof(struct rep_get_search_backup));
            m_eventLoop.exec();
        }
    }
    //page
    ui->label_size->setText(QString("%1%2").arg(GET_TEXT("COMMONBACKUP/100050", "Total Size: ")).arg(MyFileModel::fileSize(firstBackup.allSize)));
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(firstBackup.allCnt).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setMaxPage(m_pageCount);
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    ui->stackedWidget->setCurrentIndex(1);
    m_progress->hideProgress();
    updateTable();

    if (firstBackup.allCnt >= MAX_SEARCH_BACKUP_COUNT) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100042", "The search results is over %1, and the first %1 will be displayed.").arg(MAX_SEARCH_BACKUP_COUNT));
    }
}

void PagePictureBackup::ON_RESPONSE_FLAG_GET_SEARCH_PIC_PAGE(MessageReceive *message)
{
    struct resp_search_picture_backup *picture_backup_array = (struct resp_search_picture_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_picture_backup);
    if (!picture_backup_array) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        m_backupList.append(picture_backup_array[i]);
    }
    m_eventLoop.exit();
}

void PagePictureBackup::ON_RESPONSE_FLAG_PLAY_PIC_BACKUP(MessageReceive *message)
{
    QPixmap pixmap = QPixmap::fromImage(message->image1);
    if (pixmap.isNull()) {
        qMsCritical() << "pixmap is null";
    } else {
        const int &index = ui->tableView->itemData(m_currentRow, ColumnCheck, BackupIndexRole).toInt();
        const resp_search_picture_backup &backup = m_backupList.at(index);
        QString strPort = GET_TEXT("EVENTBACKUP/101007", "Location:Disk Port %1").arg(backup.port);
        QString strTime = GET_TEXT("PICTUREBACKUP/102004", "Time:%1").arg(backup.pTime);
        QString strFileSize = GET_TEXT("COMMONBACKUP/100026", "File Size:%1").arg(MyFileModel::fileSize(backup.size));
        QString strResolution = GET_TEXT("PICTUREBACKUP/102005", "Resolution:%1*%2").arg(pixmap.width()).arg(pixmap.height());
        ui->label_info->setText(QString("%1\n\n%2\n\n%3\n\n%4").arg(strPort).arg(strTime).arg(strFileSize).arg(strResolution));
        //
        ui->commonVideo->showPixmap(pixmap);
    }

    m_eventLoop.exit();
}

void PagePictureBackup::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message)
{
    if (!m_progress->isVisible()) {
        return;
    }

    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        qWarning() << "PictureBackup::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, data is null.";
        return;
    }
    qDebug() << QString("ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, %1").arg(progressinfo->percent);

    m_progress->setProgressValue(progressinfo->percent);
    m_searchSid = progressinfo->searchid;
}

void PagePictureBackup::resizeEvent(QResizeEvent *event)
{
    //column width
    int w = width() / 3 * 2 - 100;
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnChannel, 100);
    ui->tableView->setColumnWidth(ColumnDisk, 100);
    ui->tableView->setColumnWidth(ColumnTime, w - 350);

    QWidget::resizeEvent(event);
}

void PagePictureBackup::hideEvent(QHideEvent *event)
{
    closeBackup();

    QWidget::hideEvent(event);
}

void PagePictureBackup::updateTable()
{
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_backupList.size()).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    m_currentRow = -1;
    ui->tableView->clearContent();

    int beginIndex = m_pageIndex * 100;
    int endIndex = beginIndex + 100;
    if (endIndex > m_backupList.size()) {
        endIndex = m_backupList.size();
    }
    int row = 0;
    ui->tableView->setRowCount(endIndex - beginIndex);
    for (int i = beginIndex; i < endIndex; ++i) {
        const resp_search_picture_backup &backup = m_backupList.at(i);
        ui->tableView->setItemData(row, ColumnCheck, false, ItemCheckedRole);
        ui->tableView->setItemData(row, ColumnCheck, i, BackupIndexRole);
        ui->tableView->setItemIntValue(row, ColumnChannel, backup.chnid + 1);
        ui->tableView->setItemIntValue(row, ColumnDisk, backup.port);
        const QDateTime dateTime = QDateTime::fromString(backup.pTime, "yyyy-MM-dd HH:mm:ss");
        ui->tableView->setItemText(row, ColumnTime, dateTime.toString("yyyy-MM-dd HH:mm:ss"));
        ui->tableView->setItemBytesValue(row, ColumnSize, backup.size);
        row++;
    }
    ui->tableView->reorder();
    ui->tableView->selectRow(0);
    m_currentRow = 0;
    updateSelectedSize();
    updateSelectedInfo();
}

void PagePictureBackup::updateSelectedSize()
{
    qint64 bytes = 0;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        const bool checked = ui->tableView->itemData(row, ColumnCheck, ItemCheckedRole).toBool();
        if (checked) {
            const int &index = ui->tableView->itemData(row, ColumnCheck, BackupIndexRole).toInt();
            const resp_search_picture_backup &backup = m_backupList.at(index);
            bytes += backup.size;
        }
    }
    if (bytes > 0) {
        ui->label_size->setText(GET_TEXT("COMMONBACKUP/100037", "Total Size: %1         Select Size: %2").arg(MyFileModel::fileSize(m_backupList.first().allSize)).arg(MyFileModel::fileSize(bytes)));
    } else {
        ui->label_size->setText(QString("%1 %2").arg(GET_TEXT("COMMONBACKUP/100050", "Total Size: ")).arg(MyFileModel::fileSize(m_backupList.first().allSize)));
    }
}

void PagePictureBackup::updateSelectedInfo()
{
    MsWaittingContainer wait(ui->commonVideo);

    const int &index = ui->tableView->itemData(m_currentRow, ColumnCheck, BackupIndexRole).toInt();
    const resp_search_picture_backup &backup = m_backupList.at(index);
    m_currentChannel = backup.chnid;
    ui->commonVideo->showCurrentChannel(backup.chnid);
    //    QString strPort = GET_TEXT("EVENTBACKUP/101007", "Location:Disk Port %1").arg(backup.port);
    //    QString strTime = GET_TEXT("PICTUREBACKUP/102004", "Time:%1").arg(backup.pTime);
    //    QString strFileSize = GET_TEXT("COMMONBACKUP/100026", "File Size:%1").arg(MyFileModel::fileSize(backup.size));
    //    QString strResolution = GET_TEXT("PICTUREBACKUP/102005", "Resolution:%1*%2").arg(backup.width).arg(backup.height);
    //    ui->label_info->setText(QString("%1\n\n%2\n\n%3\n\n%4").arg(strPort).arg(strTime).arg(strFileSize).arg(strResolution));

    struct rep_play_picture_backup playinfo;
    memset(&playinfo, 0, sizeof(struct rep_play_picture_backup));
    playinfo.chnid = backup.chnid;
    playinfo.sid = backup.sid;
    playinfo.port = backup.port;
    playinfo.pts = backup.pts;
    snprintf(playinfo.pTime, sizeof(playinfo.pTime), "%s", backup.pTime);
    qMsDebug() << "\nREQUEST_FLAG_PLAY_PIC_BACKUP"
               << ", chnid:" << playinfo.chnid
               << ", sid:" << playinfo.sid
               << ", port:" << playinfo.port
               << ", pts:" << playinfo.pts
               << ", pTime:" << playinfo.pTime;
    sendMessage(REQUEST_FLAG_PLAY_PIC_BACKUP, (void *)&playinfo, sizeof(struct rep_play_picture_backup));
    //m_eventLoop.exec();
}

void PagePictureBackup::searchPicturePage(int page)
{
    //m_waitting->//showWait();

    struct rep_get_search_backup pageinfo;
    memset(&pageinfo, 0, sizeof(struct rep_get_search_backup));
    pageinfo.sid = m_backupList.first().sid;
    pageinfo.npage = page;
    qDebug() << QString("PictureBackup::searchPicturePage, sid: %1, page: %2").arg(pageinfo.sid).arg(pageinfo.npage);
    sendMessage(REQUEST_FLAG_GET_SEARCH_PIC_PAGE, (void *)&pageinfo, sizeof(struct rep_get_search_backup));
}

void PagePictureBackup::closeBackup()
{
    if (!m_backupList.isEmpty()) {
        sendMessage(REQUEST_FLAG_SEARCH_PIC_BACKUP_CLOSE, &m_backupList.first().sid, sizeof(int));
        m_backupList.clear();
    }
}

void PagePictureBackup::openAudio(int channel)
{
    struct req_set_audiochn audio;
    audio.chn = channel;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(audio));
}

void PagePictureBackup::closeAudio(int channel)
{
    Q_UNUSED(channel)
    struct req_set_audiochn audio;
    audio.chn = -1;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(audio));
}

void PagePictureBackup::onLanguageChanged()
{
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channels"));
    ui->label_pictureType->setText(GET_TEXT("EVENTBACKUP/101005", "Picture Type"));
    ui->label_subType->setText(GET_TEXT("LOG/64004", "Sub Type"));
    ui->label_startTime->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("LOG/64002", "End Time"));

    ui->checkBoxGroup_channel->onLanguageChanged();

    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

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
}

void PagePictureBackup::onItemClicked(int row, int column)
{
    //
    if (m_currentRow != row) {
        m_currentRow = row;
        updateSelectedInfo();
    }
    //
    switch (column) {
    case ColumnCheck: {
        updateSelectedSize();
        break;
    }
    default:
        break;
    }
}

void PagePictureBackup::onTableHeaderChecked(bool checked)
{
    Q_UNUSED(checked)

    updateSelectedSize();
}

void PagePictureBackup::onSearchCanceled()
{
    if (m_searchSid >= 0) {
        sendMessageOnly(REQUEST_FLAG_SEARCH_PIC_BACKUP_CANCEL, &m_searchSid, sizeof(int));
        m_searchSid = -1;
    }
}
#if 0
void PictureBackup::tertiaryTypeInit(int value)
{
    ui->comboBox_tertiaryType->clear();
    ui->label_tertiaryType->show();
    ui->comboBox_tertiaryType->show();
    ui->comboBox_tertiaryType->addItem(GET_TEXT("COMMON/1006","All"), INFO_MINOR_ALL_0);
    if(value == INFO_MAJOR_VCA)
    {
        qDebug()<<"@@@@@@@@@@@ PictureBackup::tertiaryTypeInit(int index):"<<value;
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55001","Region Entrance"), INFO_MINOR_REGIONIN);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55002","Region Exiting"), INFO_MINOR_REGIONOUT);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55003","Advanced Motion Detection"), INFO_MINOR_ADVANCED_MOTION);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55004","Tamper Detection"), INFO_MINOR_TAMPER);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55005","Line Crossing"), INFO_MINOR_LINECROSS);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55006","Loitering"), INFO_MINOR_LOITERING);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55007","Human Detection"), INFO_MINOR_HUMAN);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55055","Object Left/Removed"), INFO_MINOR_OBJECT);
    }
    else if(value == INFO_MAJOR_SMART)
    {
        qDebug()<<"@@@@@@@@@@@ PictureBackup::tertiaryTypeInit(int index):"<<value;
        ui->comboBox_tertiaryType->addItem(GET_TEXT("ANPR/103005","ANPR"), 0);
    }
}

void PictureBackup::on_comboBox_subType_activated(int index)
{
    int value = ui->comboBox_subType->currentData().toInt();
    qDebug()<<"PictureBackup::on_comboBox_subType_activated(int index):"<<index<<";  get:"<<value;
    ui->label_tertiaryType->hide();
    ui->comboBox_tertiaryType->hide();
    ui->tableView_alarm->hide();
    ui->checkBoxGroup_channel->show();
    ui->widget->show();
    switch (value){
    case INFO_MAJOR_ALARMIN:
        ui->checkBoxGroup_channel->hide();
        ui->widget->hide();
        ui->tableView_alarm->show();
        break;
    case INFO_MAJOR_VCA:
        tertiaryTypeInit(value);
        break;
    case INFO_MAJOR_SMART:
        tertiaryTypeInit(value);
        break;
    default:
        break;
    }
}
#endif
void PagePictureBackup::on_comboBox_pictureType_activated(int index)
{
    int value = ui->comboBox_pictureType->currentData().toInt();
    qDebug() << "PictureBackup::on_comboBox_pictureType_activated(int index):" << index << ";  get:" << value;
    //    ui->tableView_alarm->hide();
    //    ui->checkBoxGroup_channel->show();
    //    ui->widget->show();
    if (value == INOF_MAJOR_EVENT_PIC) {
        qDebug() << "########## PictureBackup::on_comboBox_pictureType_activated(int index):" << index;
        ui->label_subType->show();
        ui->comboBox_subType->show();
        ui->comboBox_subType->setCurrentIndex(0);
    } else {
        ui->label_subType->hide();
        ui->comboBox_subType->hide();
        //        ui->label_tertiaryType->hide();
        //        ui->comboBox_tertiaryType->hide();
    }
    //    if(ui->comboBox_subType->isVisible())
    //        on_comboBox_subType_activated(0);
}

void PagePictureBackup::on_pushButton_search_clicked()
{
    return;
    m_pageIndex = 0;
    m_pageCount = 0;

    const QList<int> &checkedList = ui->checkBoxGroup_channel->checkedList();
    if (checkedList.isEmpty()) {
        ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel"));
        return;
    }
    const QString &checkedMask = ui->checkBoxGroup_channel->checkedMask();

    struct req_search_picture_backup picture_backup;
    memset(&picture_backup, 0, sizeof(req_search_picture_backup));
    snprintf(picture_backup.chnMaskl, sizeof(picture_backup.chnMaskl), "%s", checkedMask.toStdString().c_str());
    picture_backup.chnNum = checkedList.count();
    picture_backup.enMajor = ui->comboBox_pictureType->currentData().toInt();
    picture_backup.enMinor = ui->comboBox_subType->currentData().toInt();
    if (picture_backup.enMajor != INOF_MAJOR_EVENT_PIC)
        picture_backup.enMinor = INFO_MINOR_NONE;
    qDebug() << QString("PictureBackup enMajor:[%1], enMinor:[%2]").arg(picture_backup.enMajor).arg(picture_backup.enMinor);
    const QDateTime startDateTime(ui->dateEdit_start->date(), ui->timeEdit_start->time());
    const QDateTime endDateTime(ui->dateEdit_end->date(), ui->timeEdit_end->time());
    if (startDateTime > endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }
    snprintf(picture_backup.pStartTime, sizeof(picture_backup.pStartTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(picture_backup.pEndTime, sizeof(picture_backup.pEndTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
#if 0
    const INFO_MAJOR_EN &type = static_cast<INFO_MAJOR_EN>(ui->comboBox_subType->currentData().toInt());
    switch (type) {
    case INFO_MAJOR_MOTION:
    case INFO_MAJOR_VCA:
    case INFO_MAJOR_SMART:
    {
        const QList<int> &checkedList = ui->checkBoxGroup_channel->checkedList();
        if (checkedList.isEmpty())
        {
            ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel"));
            return;
        }
        const QString &checkedMask = ui->checkBoxGroup_channel->checkedMask();
//        snprintf(event_backup.chnMaskl, sizeof(event_backup.chnMaskl), "%s", checkedMask.toStdString().c_str());
//        event_backup.chnNum = checkedList.count();
        snprintf(picture_backup.chnMaskl, sizeof(picture_backup.chnMaskl), "%s", checkedMask.toStdString().c_str());
        picture_backup.chnNum = checkedList.count();

        if (type == INFO_MAJOR_VCA || type == INFO_MAJOR_SMART)
        {
            ui->comboBox_tertiaryType->currentData().toInt();
        }

//        ms_packet.nSize = sizeof(struct req_search_event_backup);
//        ms_packet.nHeaderSize = ms_packet.nSize;
//        ms_packet.nBodySize = 0;

//        ms_packet.packet = (char *)ms_malloc(ms_packet.nSize);
//        ms_packet.packetHeader = ms_packet.packet;
//        ms_packet.packetBody = NULL;
        break;
    }
    case INFO_MAJOR_ALARMIN:
    {
        QList<AlarmKey> checkedList;
        for (int i = 0; i < ui->tableView_alarm->rowCount(); ++i)
        {
            const bool &checked = ui->tableView_alarm->itemData(i, 0, ItemCheckedRole).toInt();
            if (checked)
            {
                const AlarmKey &key = ui->tableView_alarm->itemData(i, 1, AlarmKeyRole).value<AlarmKey>();
                checkedList.append(key);
            }
        }
        if (checkedList.isEmpty())
        {
            ShowMessageBox(GET_TEXT("EVENTBACKUP/101008","Please select at least one Alarm Input No."));
            return;
        }
//        event_backup.pEventNameLen = MAX_LEN_64;
//        event_backup.enEventCnt = checkedList.count();

//        ms_packet.nHeaderSize = sizeof(struct req_search_event_backup);
//        ms_packet.nBodySize = event_backup.pEventNameLen * event_backup.enEventCnt;
//        ms_packet.nSize = ms_packet.nHeaderSize + ms_packet.nBodySize;

//        ms_packet.packet = (char *)ms_malloc(ms_packet.nSize);
//        ms_packet.packetHeader = ms_packet.packet;
//        ms_packet.packetBody = ms_packet.packetHeader + ms_packet.nHeaderSize;

        for (int i = 0; i < checkedList.count(); ++i)
        {
//            const AlarmKey &alarm = checkedList.at(i);
//            snprintf(ms_packet.packetBody, event_backup.pEventNameLen, "%s", alarm.nameForSearch().toStdString().c_str());
//            ms_packet.packetBody += event_backup.pEventNameLen;
        }
        break;
    }
    default:
        break;
    }
#endif
    sendMessage(REQUEST_FLAG_SEARCH_PIC_BACKUP, (void *)&picture_backup, sizeof(struct req_search_picture_backup));

    m_progress->setProgressValue(0);
    m_progress->showProgress();
}

void PagePictureBackup::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PagePictureBackup::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));
    updateTable();
}

void PagePictureBackup::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));
    updateTable();
}

void PagePictureBackup::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));
    updateTable();
}

void PagePictureBackup::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));
    updateTable();
}

void PagePictureBackup::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));
    updateTable();
}

void PagePictureBackup::on_toolButton_previous_clicked()
{
    if (m_currentRow > 0) {
        m_currentRow--;
        ui->tableView->selectRow(m_currentRow);
        updateSelectedInfo();
    }
}

void PagePictureBackup::on_toolButton_next_clicked()
{
    if (m_currentRow < ui->tableView->rowCount() - 1) {
        m_currentRow++;
        ui->tableView->selectRow(m_currentRow);
        updateSelectedInfo();
    }
}

void PagePictureBackup::on_pushButton_backupAll_clicked()
{
    //
    if (m_backupList.isEmpty()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        return;
    }

    //
    const QString &directory = MyFileSystemDialog::instance()->exportPictureWithAnimate();
    if (directory.isEmpty()) {
        return;
    }
    const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int pictureResolution = MyFileSystemDialog::instance()->pictureResolution();

    //
    qint64 totalBytes = 0;
    QList<rep_export_picture_backup> backupList;
    for (int i = 0; i < m_backupList.size(); ++i) {
        const resp_search_picture_backup &searchBackup = m_backupList.at(i);
        totalBytes += searchBackup.size;

        rep_export_picture_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_picture_backup));
        exportBackup.chnid = searchBackup.chnid;
        exportBackup.sid = searchBackup.sid;
        exportBackup.port = searchBackup.port;
        exportBackup.pts = searchBackup.pts;
        exportBackup.index = searchBackup.index;
        exportBackup.resolution = pictureResolution;
        snprintf(exportBackup.pTime, sizeof(exportBackup.pTime), "%s", searchBackup.pTime);
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
        const rep_export_picture_backup &exportBackup = backupList.at(i);

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
#else
    m_export->setExportPictureBackup(backupList);
    m_export->show();
#endif
}

void PagePictureBackup::on_pushButton_backup_clicked()
{
    //
    QList<int> checkedList;
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        const bool &checked = ui->tableView->itemData(i, ColumnCheck, ItemCheckedRole).toBool();
        if (checked) {
            checkedList.append(i);
        }
    }
    if (checkedList.isEmpty()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        return;
    }

    //
    const QString &directory = MyFileSystemDialog::instance()->exportPictureWithAnimate();
    if (directory.isEmpty()) {
        return;
    }
    const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeSize = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    const int pictureResolution = MyFileSystemDialog::instance()->pictureResolution();

    //
    qint64 totalBytes = 0;
    QList<rep_export_picture_backup> backupList;
    for (int i = 0; i < checkedList.size(); ++i) {
        const int &row = checkedList.at(i);
        const int &index = ui->tableView->itemData(row, ColumnCheck, BackupIndexRole).toInt();
        const resp_search_picture_backup &searchBackup = m_backupList.at(index);
        totalBytes += searchBackup.size;

        rep_export_picture_backup exportBackup;
        memset(&exportBackup, 0, sizeof(rep_export_picture_backup));
        exportBackup.chnid = searchBackup.chnid;
        exportBackup.sid = searchBackup.sid;
        exportBackup.port = searchBackup.port;
        exportBackup.pts = searchBackup.pts;
        exportBackup.index = searchBackup.index;
        exportBackup.resolution = pictureResolution;
        snprintf(exportBackup.pTime, sizeof(exportBackup.pTime), "%s", searchBackup.pTime);
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
        const rep_export_picture_backup &exportBackup = backupList.at(i);

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
#else
    m_export->setExportPictureBackup(backupList);
#endif

    //
    DownloadAnimate::instance()->startAnimate(MyFileSystemDialog::instance()->geometry(), QPixmap::grabWidget(MyFileSystemDialog::instance()));
}

void PagePictureBackup::on_pushButton_back_2_clicked()
{
    closeBackup();

    ui->stackedWidget->setCurrentIndex(0);
    ui->commonVideo->hideVideo();
    emit updateVideoGeometry();
}
