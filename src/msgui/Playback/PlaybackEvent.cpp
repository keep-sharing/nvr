#include "PlaybackEvent.h"
#include "ui_PlaybackEvent.h"
#include "AlarmKey.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackEventList.h"
#include "PlaybackWindow.h"
#include "centralmessage.h"
#include "msuser.h"
#include <QElapsedTimer>
#include <QKeyEvent>
#include <qmath.h>

const int AlarmKeyRole = Qt::UserRole + 101;
const int ChannelRole = Qt::UserRole + 102;

PlaybackEvent::PlaybackEvent(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackEvent)
{
    ui->setupUi(this);

    //
    connect(ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(ui->treeView, SIGNAL(enterPressed()), this, SLOT(onTreeViewEnterPressed()));

    ui->dateEdit_start->setProperty("style", "black");
    ui->dateEdit_end->setProperty("style", "black");

    //
    ui->comboBoxDetectionObject->clear();
    ui->comboBoxDetectionObject->beginEdit();
    ui->comboBoxDetectionObject->addTranslatableItem("COMMON/1006", "All", DETEC_ALL);
    ui->comboBoxDetectionObject->addTranslatableItem("TARGETMODE/103201", "Human", DETEC_HUMAN);
    ui->comboBoxDetectionObject->addTranslatableItem("TARGETMODE/103202", "Vehicle", DETEC_VEHICLE);
    ui->comboBoxDetectionObject->endEdit();
    ui->comboBoxDetectionObject->reSetIndex();

    //
    m_progress = new ProgressDialog(this);
    connect(m_progress, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_RETRIEVE, this);

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PlaybackEvent::~PlaybackEvent()
{
    delete ui;
}

void PlaybackEvent::initializeData()
{
    m_alarmInputMask.clear();
    m_channelMask.clear();
    ui->treeView->clear();
    ui->lineEditPosContent->clear();

    if (!m_playList) {
        m_playList = new PlaybackEventList(PlaybackWindow::instance());
    }
    m_playList->initializeData();

    m_treeType = NoneTree;
    ui->comboBox_mainType->setCurrentIndex(0);
    m_mainType = INFO_MAJOR_MOTION;
    on_comboBox_mainType_activated(0);

    const QDate &date = QDate::currentDate();
    ui->dateEdit_start->setDate(date);
    ui->dateEdit_end->setDate(date);
    ui->timeEdit_start->setTime(QTime(0, 0, 0));
    ui->timeEdit_end->setTime(QTime(23, 59, 59));
}

void PlaybackEvent::closePlayback()
{
    if (m_playList) {
        m_playList->closePlayback();
    }
}

void PlaybackEvent::setFocus()
{
    ui->comboBox_mainType->setFocus();
}

bool PlaybackEvent::hasFocus()
{
    bool result = false;
    if (ui->comboBox_mainType->hasFocus()) {
        result = true;
    } else if (ui->comboBoxSubType->hasFocus()) {
        result = true;
    } else if (ui->treeView->hasFocus()) {
        result = true;
    } else if (ui->dateEdit_start->hasFocus()) {
        result = true;
    } else if (ui->timeEdit_start->hasFocus()) {
        result = true;
    } else if (ui->dateEdit_end->hasFocus()) {
        result = true;
    } else if (ui->timeEdit_end->hasFocus()) {
        result = true;
    } else if (ui->pushButton_search->hasFocus()) {
        result = true;
    }
    return result;
}

bool PlaybackEvent::focusPrevious()
{
    return false;
}

bool PlaybackEvent::focusNext()
{
    bool result = true;
    if (ui->pushButton_search->hasFocus()) {
        result = false;
    } else if (ui->comboBox_mainType->hasFocus() && !ui->comboBoxSubType->isVisible()) {
        focusNextChild();
        ui->treeView->setCurrentRow(0);
    } else if (ui->comboBoxSubType->isVisible() && ui->comboBoxSubType->hasFocus()) {
        focusNextChild();
        ui->treeView->setCurrentRow(0);
    } else {
        focusNextChild();
    }
    return result;
}

NetworkResult PlaybackEvent::dealNetworkCommond(const QString &commond)
{
    NetworkResult result = NetworkReject;
    if (commond.startsWith("Enter")) {
        if (ui->treeView->hasFocus()) {
            int row = ui->treeView->currentIndex().row();
            if (row < 0) {
                result = NetworkReject;
            } else {
                onItemClicked(row, 0);
                result = NetworkAccept;
            }
        }
    } else if (m_playList && m_playList->isVisible()) {
        result = m_playList->dealNetworkCommond(commond);
    }
    return result;
}

void PlaybackEvent::dealMessage(MessageReceive *message)
{
    if (m_playList) {
        m_playList->dealMessage(message);
    }
}

void PlaybackEvent::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_EVT_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(message);
        break;
    }
}

void PlaybackEvent::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        break;
    }
}

void PlaybackEvent::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message)
{
    m_searchSid = -1;
    m_progress->hideProgress();
    m_isSearching = false;

    //
    struct resp_search_event_backup *event_backup_array = (struct resp_search_event_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_event_backup);

    if (!event_backup_array || count == 0) {
        qDebug() << QString("PlaybackEvent::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP, no matching video files.");
        ShowMessageBox(GET_TEXT("DISK/92026", "No matching video files."));
        return;
    }

    qMsDebug() << "allCnt:" << event_backup_array->allCnt;

    if (event_backup_array->allCnt >= MAX_SEARCH_BACKUP_COUNT) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100042", "The search results is over %1, and the first %1 will be displayed.").arg(MAX_SEARCH_BACKUP_COUNT));
    }

    m_playList->show();
    m_playList->processMessage(message);
}

void PlaybackEvent::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message)
{
    if (!m_isSearching) {
        return;
    }

    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        qWarning() << "PlaybackEvent::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, data is null.";
        return;
    }

    if (!m_progress->isVisible()) {
        m_progress->showProgress();
    }
    m_progress->setProgressValue(progressinfo->percent);
    m_searchSid = progressinfo->searchid;
}

void PlaybackEvent::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "====PlaybackEvent::keyPressEvent====";
    qDebug() << "----this:" << this;
    qDebug() << "----event:" << event;
    switch (event->key()) {
    case Qt::Key_Escape:
        //escapePressed();
        event->accept();
        break;
    case Qt::Key_Return:
        returnPressed();
        event->accept();
        break;
    default:
        break;
    }
    BasePlayback::keyPressEvent(event);
}

void PlaybackEvent::returnPressed()
{
    if (ui->treeView->hasFocus()) {
        int row = ui->treeView->currentIndex().row();
        if (row < 0) {

        } else {
            onItemClicked(row, 0);
        }
    }
}

void PlaybackEvent::initializeChannelTree()
{
    if (m_treeType == ChannelTree) {
        return;
    }
    m_treeType = ChannelTree;

    ui->treeView->clearContent();
    //channel tree
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("CAMERASTATUS/62002", "Name");
    ui->treeView->setHorizontalHeaderLabels(headerList);
    ui->treeView->setColumnWidth(0, 50);
    ui->treeView->setColumnWidth(1, 50);
    ui->treeView->setHorizontalHeaderCheckable(true);
    ui->treeView->setColumnCount(headerList.count());
    if (!m_columnCheckDelegate) {
        m_columnCheckDelegate = new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this);
    }
    ui->treeView->setItemDelegateForColumn(0, m_columnCheckDelegate);
    //
    QList<int> channelList = gMsUser.accessiblePlaybackChannelList();
    ui->treeView->setRowCount(channelList.size());
    //
    struct osd osdList[MAX_CAMERA];
    int osdCount = 0;
    read_osds(SQLITE_FILE_NAME, osdList, &osdCount);
    //
    int row = 0;
    for (int i = 0; i < channelList.size(); ++i) {
        int channel = channelList.at(i);
        ui->treeView->setItemData(row, 0, m_channelMask.contains(channel) ? 1 : 0, ItemCheckedRole);
        ui->treeView->setItemData(row, 0, channel, ChannelRole);
        ui->treeView->setItemText(row, 1, QString::number(channel + 1));
        ui->treeView->setItemText(row, 2, osdList[channel].name);
        row++;
    }
}

void PlaybackEvent::initializeAlarminTree()
{
    if (m_treeType == AlarmTree) {
        return;
    }
    m_treeType = AlarmTree;

    ui->treeView->clearContent();
    //alarmin tree
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CAMERASEARCH/32002", "No.");
    headerList << GET_TEXT("CAMERASTATUS/62002", "Name");
    ui->treeView->setHorizontalHeaderLabels(headerList);
    ui->treeView->setColumnWidth(0, 50);
    ui->treeView->setColumnWidth(1, 100);
    ui->treeView->setHorizontalHeaderCheckable(true);
    ui->treeView->setColumnCount(headerList.count());
    if (!m_columnCheckDelegate) {
        m_columnCheckDelegate = new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this);
    }
    ui->treeView->setItemDelegateForColumn(0, m_columnCheckDelegate);
    //alarmin
    auto allAlarminNames = qMsNvr->allAlarminNames();
    for (auto iter = allAlarminNames.begin(); iter != allAlarminNames.end();) {
        const AlarmKey &key = iter.key();
        if (key.channel() < 0) {
            ++iter;
            continue;
        }
        if (!gMsUser.checkPlaybackChannelPermission(key.channel())) {
            iter = allAlarminNames.erase(iter);
        } else {
            ++iter;
            continue;
        }
    }
    int row = 0;
    ui->treeView->setRowCount(allAlarminNames.size());
    for (auto iter = allAlarminNames.constBegin(); iter != allAlarminNames.constEnd(); ++iter) {
        const AlarmKey &key = iter.key();

        ui->treeView->setItemData(row, 0, m_alarmInputMask.contains(key) ? 1 : 0, ItemCheckedRole);
        ui->treeView->setItemText(row, 1, key.numberName());
        ui->treeView->setItemData(row, 1, QVariant::fromValue(key), AlarmKeyRole);
        if (key.name().isEmpty()) {
            ui->treeView->setItemText(row, 2, "-");
        } else {
            ui->treeView->setItemText(row, 2, key.name());
        }
        row++;
    }
}

void PlaybackEvent::updateSubType()
{
    const INFO_MAJOR_EN &type = static_cast<INFO_MAJOR_EN>(ui->comboBox_mainType->currentData().toInt());
    switch (type) {
    case INFO_MAJOR_VCA:
        ui->comboBoxSubType->beginEdit();
        ui->comboBoxSubType->clear();
        ui->comboBoxSubType->addItem(GET_TEXT("COMMON/1006", "All"), INFO_MINOR_ALL_0);
        ui->comboBoxSubType->setItemData(0, GET_TEXT("COMMON/1006", "All"), Qt::ToolTipRole);
        ui->comboBoxSubType->addItem(GET_TEXT("SMARTEVENT/55001", "Region Entrance"), INFO_MINOR_REGIONIN);
        ui->comboBoxSubType->setItemData(1, GET_TEXT("SMARTEVENT/55001", "Region Entrance"), Qt::ToolTipRole);
        ui->comboBoxSubType->addItem(GET_TEXT("SMARTEVENT/55002", "Region Exiting"), INFO_MINOR_REGIONOUT);
        ui->comboBoxSubType->setItemData(2, GET_TEXT("SMARTEVENT/55002", "Region Exiting"), Qt::ToolTipRole);
        ui->comboBoxSubType->addItem(GET_TEXT("SMARTEVENT/55003", "Advanced Motion Detection"), INFO_MINOR_ADVANCED_MOTION);
        ui->comboBoxSubType->setItemData(3, GET_TEXT("SMARTEVENT/55003", "Advanced Motion Detection"), Qt::ToolTipRole);
        ui->comboBoxSubType->addItem(GET_TEXT("SMARTEVENT/55004", "Tamper Detection"), INFO_MINOR_TAMPER);
        ui->comboBoxSubType->setItemData(4, GET_TEXT("SMARTEVENT/55004", "Tamper Detection"), Qt::ToolTipRole);
        ui->comboBoxSubType->addItem(GET_TEXT("SMARTEVENT/55005", "Line Crossing"), INFO_MINOR_LINECROSS);
        ui->comboBoxSubType->setItemData(5, GET_TEXT("SMARTEVENT/55005", "Line Crossing"), Qt::ToolTipRole);
        ui->comboBoxSubType->addItem(GET_TEXT("SMARTEVENT/55006", "Loitering"), INFO_MINOR_LOITERING);
        ui->comboBoxSubType->setItemData(6, GET_TEXT("SMARTEVENT/55006", "Loitering"), Qt::ToolTipRole);
        ui->comboBoxSubType->addItem(GET_TEXT("SMARTEVENT/55007", "Human Detection"), INFO_MINOR_HUMAN);
        ui->comboBoxSubType->setItemData(7, GET_TEXT("SMARTEVENT/55007", "Human Detection"), Qt::ToolTipRole);
        ui->comboBoxSubType->addItem(GET_TEXT("SMARTEVENT/55055", "Object Left/Removed"), INFO_MINOR_OBJECT);
        ui->comboBoxSubType->setItemData(8, GET_TEXT("SMARTEVENT/55055", "Object Left/Removed"), Qt::ToolTipRole);
        ui->comboBoxSubType->endEdit();
        ui->comboBoxSubType->reSetIndex();
        break;
    case INFO_MAJOR_SMART: {
        ui->comboBoxSubType->beginEdit();
        ui->comboBoxSubType->clear();
        int index = 0;
        ui->comboBoxSubType->addItem(GET_TEXT("COMMON/1006", "All"), INFO_MINOR_ALL);
        ui->comboBoxSubType->setItemData(index++, GET_TEXT("COMMON/1006", "All"), Qt::ToolTipRole);
        if (qMsNvr->isSupportTargetMode()) {
            ui->comboBoxSubType->addItem("ANPR", INFO_MINOR_LPR);
            ui->comboBoxSubType->setItemData(index++, "ANPR", Qt::ToolTipRole);
        }
        if (qMsNvr->isSupportFaceDetection()) {
            ui->comboBoxSubType->addItem(GET_TEXT("FACE/141000", "Face Detection"), INFO_MINOR_FACE);
            ui->comboBoxSubType->setItemData(index++, GET_TEXT("FACE/141000", "Face Detection"), Qt::ToolTipRole);
        }
        ui->comboBoxSubType->addItem("POS", INFO_MINOR_POS);
        ui->comboBoxSubType->setItemData(index++, "POS", Qt::ToolTipRole);

        ui->comboBoxSubType->addItem(GET_TEXT("SMARTEVENT/55008", "People Counting"), INFO_MINOR_PCNT);
        ui->comboBoxSubType->setItemData(index++, GET_TEXT("SMARTEVENT/55008", "People Counting"), Qt::ToolTipRole);

        ui->comboBoxSubType->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/103313", "Regional People Counting"), INFO_MINOR_REGION);
        ui->comboBoxSubType->setItemData(index++, GET_TEXT("REGIONAL_PEOPLECOUNTING/103313", "Regional People Counting"), Qt::ToolTipRole);
        ui->comboBoxSubType->endEdit();
        ui->comboBoxSubType->reSetIndex();
        break;
    }
    default:
        break;
    }
}

void PlaybackEvent::onLanguageChanged()
{
    ui->comboBox_mainType->clear();
    ui->comboBox_mainType->addItem(GET_TEXT("MOTION/51000", "Motion Detection"), INFO_MAJOR_MOTION);
    ui->comboBox_mainType->addItem(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"), INFO_MAJOR_AUDIO_ALARM);
    ui->comboBox_mainType->addItem(GET_TEXT("ALARMIN/52001", "Alarm Input"), INFO_MAJOR_ALARMIN);
    ui->comboBox_mainType->addItem(GET_TEXT("SMARTEVENT/55000", "VCA"), INFO_MAJOR_VCA);
    ui->comboBox_mainType->addItem(GET_TEXT("ANPR/103054", "Smart Analysis"), INFO_MAJOR_SMART);

    ui->labelPosContent->setText(GET_TEXT("POS/130034", "POS Content"));

    updateSubType();
    //
    ui->label_mainType->setText(GET_TEXT("LOG/64003", "Main Type"));
    ui->label_subType->setText(GET_TEXT("LOG/64004", "Sub Type"));
    ui->label_startTime->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("LOG/64002", "End Time"));
    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    //
    ui->labelDetectionObject->setText(GET_TEXT("TARGETMODE/103200", "Detection Object"));
    ui->comboBoxDetectionObject->retranslate();
}

void PlaybackEvent::onItemClicked(const QModelIndex &index)
{
    onItemClicked(index.row(), index.column());
}

void PlaybackEvent::onItemClicked(int row, int column)
{
    Q_UNUSED(column)

    bool checked = ui->treeView->isItemChecked(row);
    checked = !checked;
    ui->treeView->setItemChecked(row, checked);
}

void PlaybackEvent::onTreeViewEnterPressed()
{
    int row = ui->treeView->currentIndex().row();
    if (row < 0) {

    } else {
        onItemClicked(row, 0);
    }
}

void PlaybackEvent::onSearchCanceled()
{
    if (m_searchSid >= 0) {
        sendMessageOnly(REQUEST_FLAG_SEARCH_EVT_BACKUP_CANCEL, &m_searchSid, sizeof(int));
        m_searchSid = -1;
    }
}

void PlaybackEvent::on_comboBox_mainType_activated(int index)
{
    const INFO_MAJOR_EN &type = static_cast<INFO_MAJOR_EN>(ui->comboBox_mainType->itemData(index).toInt());
    updateCheckedChannel();

    ui->labelDetectionObject->hide();
    ui->comboBoxDetectionObject->hide();

    ui->comboBoxSubType->hide();
    ui->labelPosContent->hide();

    switch (type) {
    case INFO_MAJOR_MOTION:
    case INFO_MAJOR_AUDIO_ALARM:
        ui->label_subType->hide();
        ui->lineEditPosContent->hide();
        initializeChannelTree();
        break;
    case INFO_MAJOR_ALARMIN:
        ui->label_subType->hide();
        ui->comboBoxSubType->hide();
        initializeAlarminTree();
        break;
    case INFO_MAJOR_VCA:
        ui->label_subType->show();
        ui->comboBoxSubType->show();
        ui->comboBoxDetectionObject->setCurrentIndex(0);
        initializeChannelTree();
        updateSubType();
        break;
    case INFO_MAJOR_SMART:
        ui->label_subType->show();
        ui->comboBoxSubType->show();
        initializeChannelTree();
        updateSubType();
        break;
    default:
        break;
    }

    m_mainType = type;
}

void PlaybackEvent::on_comboBox_mainType_indexSet(int index)
{
    Q_UNUSED(index)
    ui->comboBox_mainType->setToolTip(ui->comboBox_mainType->currentText());
}

void PlaybackEvent::on_comboBoxSubType_indexSet(int index)
{
    ui->labelDetectionObject->hide();
    ui->comboBoxDetectionObject->hide();

    ui->labelPosContent->hide();
    ui->lineEditPosContent->hide();

    int mainType = ui->comboBox_mainType->currentData().toInt();
    int enMinor = ui->comboBoxSubType->itemData(index).toInt();
    if (mainType == INFO_MAJOR_VCA) {
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
            break;
        }
    } else if (mainType == INFO_MAJOR_SMART) {
        switch (enMinor) {
        case INFO_MINOR_POS:
            ui->labelPosContent->show();
            ui->lineEditPosContent->show();
            break;
        default:
            break;
        }
    }
    //
    ui->comboBoxSubType->setToolTip(ui->comboBoxSubType->currentText());
}

void PlaybackEvent::on_comboBoxDetectionObject_indexSet(int index)
{
    Q_UNUSED(index)
    ui->comboBoxDetectionObject->setToolTip(ui->comboBoxDetectionObject->currentText());
}

void PlaybackEvent::on_pushButton_search_clicked()
{
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

    const INFO_MAJOR_EN &mainType = static_cast<INFO_MAJOR_EN>(ui->comboBox_mainType->currentData().toInt());
    const INFO_MINOR_EN &subType = static_cast<INFO_MINOR_EN>(ui->comboBoxSubType->currentData().toInt());
    switch (mainType) {
    case INFO_MAJOR_MOTION:
    case INFO_MAJOR_AUDIO_ALARM:
    case INFO_MAJOR_VCA:
    case INFO_MAJOR_SMART: {
        //
        QList<int> checkedList;
        for (int i = 0; i < ui->treeView->rowCount(); ++i) {
            const bool &checked = ui->treeView->itemData(i, 0, ItemCheckedRole).toInt();
            if (checked) {
                int channel = ui->treeView->itemData(i, 0, ChannelRole).toInt();
                checkedList.append(channel);
            }
        }
        if (checkedList.isEmpty()) {
            ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel"));
            return;
        }
        makeChannelMask(checkedList, event_backup.chnMaskl, sizeof(event_backup.chnMaskl));
        event_backup.chnNum = checkedList.count();

        if (mainType == INFO_MAJOR_VCA || mainType == INFO_MAJOR_SMART) {
            event_backup.enMinor = subType;

            if (subType == INFO_MINOR_POS) {
                snprintf(event_backup.content, sizeof(event_backup.content), "%s", ui->lineEditPosContent->text().toStdString().c_str());
            }
        }
        if (mainType == INFO_MAJOR_VCA) {
            event_backup.objtype = static_cast<DETEC_OBJ_EN>(ui->comboBoxDetectionObject->currentData().toInt());
        }

        ms_packet.nSize = sizeof(struct req_search_event_backup);
        ms_packet.nHeaderSize = ms_packet.nSize;
        ms_packet.nBodySize = 0;

        ms_packet.packet = (char *)ms_malloc(ms_packet.nSize);
        ms_packet.packetHeader = ms_packet.packet;
        ms_packet.packetBody = NULL;
        break;
    }
    case INFO_MAJOR_ALARMIN: {
        //
        QList<AlarmKey> checkedList;
        for (int i = 0; i < ui->treeView->rowCount(); ++i) {
            const bool &checked = ui->treeView->itemData(i, 0, ItemCheckedRole).toInt();
            if (checked) {
                const AlarmKey &key = ui->treeView->itemData(i, 1, AlarmKeyRole).value<AlarmKey>();
                checkedList.append(key);
            }
        }
        if (checkedList.isEmpty()) {
            ShowMessageBox(GET_TEXT("EVENTBACKUP/101008", "Please select at least one Alarm Input No."));
            return;
        }
        event_backup.pEventNameLen = MAX_LEN_64;
        event_backup.enEventCnt = checkedList.count();

        ms_packet.nHeaderSize = sizeof(struct req_search_event_backup);
        ms_packet.nBodySize = event_backup.pEventNameLen * event_backup.enEventCnt;
        ms_packet.nSize = ms_packet.nHeaderSize + ms_packet.nBodySize;

        ms_packet.packet = (char *)ms_malloc(ms_packet.nSize);
        ms_packet.packetHeader = ms_packet.packet;
        ms_packet.packetBody = ms_packet.packetHeader + ms_packet.nHeaderSize;

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
    event_backup.enType = playbackStream();
    event_backup.enMajor = mainType;
    event_backup.ahead = 0;
    event_backup.delay = 0;
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
               << "\n----enMinor:" << event_backup.enMinor
               << "\n----object:" << event_backup.objtype
               << "\n----ahead:" << event_backup.ahead
               << "\n----delay:" << event_backup.delay
               << "\n----pStartTime:" << event_backup.pStartTime
               << "\n----pEndTime:" << event_backup.pEndTime;
    sendMessage(REQUEST_FLAG_SEARCH_EVT_BACKUP, ms_packet.packet, ms_packet.nSize);

    m_isSearching = true;

    if (ms_packet.packet) {
        ms_free(ms_packet.packet);
    }
}

void PlaybackEvent::updateCheckedChannel()
{
    switch (m_mainType) {
    case INFO_MAJOR_ALARMIN:
        m_alarmInputMask.clear();
        break;
    default:
        m_channelMask.clear();
        break;
    }

    for (int i = 0; i < ui->treeView->rowCount(); ++i) {
        const bool &checked = ui->treeView->itemData(i, 0, ItemCheckedRole).toInt();
        if (checked) {
            switch (m_mainType) {
            case INFO_MAJOR_ALARMIN: {
                const AlarmKey &key = ui->treeView->itemData(i, 1, AlarmKeyRole).value<AlarmKey>();
                m_alarmInputMask.append(key);
                break;
            }
            default: {
                int channel = ui->treeView->itemData(i, 0, ChannelRole).toInt();
                m_channelMask.append(channel);
                break;
            }
            }
        }
    }
}

void PlaybackEvent::selectNextChannel()
{
    int channel = ui->treeView->currentIndex().row();
    if (channel < 0) {
        channel = 0;
    } else if (channel < ui->treeView->rowCount() - 1) {
        channel++;
    }
    ui->treeView->setCurrentRow(channel);
}
