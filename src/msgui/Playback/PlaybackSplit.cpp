#include "PlaybackSplit.h"
#include "ui_PlaybackSplit.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackData.h"
#include "centralmessage.h"
#include "globalwaitting.h"
#include "msuser.h"
#include "PlaybackWindow.h"
#include "PlaybackBar.h"
#include "PlaybackLayoutSplit.h"
#include "PlaybackTimeLine.h"
#include <qmath.h>

const int ChannelRole = Qt::UserRole + 102;

PlaybackSplit *PlaybackSplit::s_self = nullptr;

PlaybackSplit::PlaybackSplit(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackSplit)
{
    ui->setupUi(this);

    s_self = this;

    ui->dateEdit_start->setProperty("style", "black");
    ui->dateEdit_end->setProperty("style", "black");

    //channel tree
    QStringList headerList;
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("COMMON/1051", "Name");
    ui->treeView_channel->setHorizontalHeaderLabels(headerList);
    ui->treeView_channel->setColumnWidth(ColumnChannel, 50);
    ItemChannelDelegate *itemDelegate = new ItemChannelDelegate(this);
    itemDelegate->setCheckedPixmap(":/common/common/radio_checked.png");
    itemDelegate->setUncheckedPixmap(":/common/common/radio_unchecked.png");
    ui->treeView_channel->setItemDelegate(itemDelegate);
    //
    int channelCount = qMsNvr->maxChannel();
    ui->treeView_channel->setRowCount(channelCount);
    //
    connect(ui->treeView_channel, SIGNAL(itemClicked(int, int)), this, SLOT(onChannelClicked(int, int)));
    connect(ui->treeView_channel, SIGNAL(enterPressed()), this, SLOT(onTreeViewEnterPressed()));

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PlaybackSplit::~PlaybackSplit()
{
    s_self = nullptr;
    delete ui;
}

PlaybackSplit *PlaybackSplit::instance()
{
    return s_self;
}

void PlaybackSplit::initializeData()
{
    const QDate &currentDate = QDate::currentDate();
    ui->dateEdit_start->setDate(currentDate);
    ui->dateEdit_end->setDate(currentDate);
    ui->timeEdit_start->setTime(QTime(0, 0, 0));
    ui->timeEdit_end->setTime(QTime(23, 59, 59));

    //
    struct osd osd_array[MAX_CAMERA];
    int osdCount = 0;
    read_osds(SQLITE_FILE_NAME, osd_array, &osdCount);
    //
    ui->treeView_channel->clearContent();
    QList<int> channelList = gMsUser.accessiblePlaybackChannelList();
    ui->treeView_channel->setRowCount(channelList.size());
    int row = 0;
    for (int i = 0; i < channelList.size(); ++i) {
        int channel = channelList.at(i);
        struct osd &osd_info = osd_array[channel];
        ui->treeView_channel->setItemData(row, ColumnChannel, channel, ChannelRole);
        ui->treeView_channel->setItemColorText(row, ColumnChannel, QString::number(channel + 1));
        ui->treeView_channel->setItemColorText(row, ColumnName, osd_info.name);
        ui->treeView_channel->setItemToolTip(row, ColumnName, osd_info.name);
        row++;
    }
    m_channel = 0;
    m_layoutMode = SplitLayout_4;
    m_selectedSid = 0;
}

void PlaybackSplit::closePlayback()
{
    qDebug() << "PlaybackSplit::closePlayback, begin";
    //GlobalWaitting::showWait();

    closeAudio();
    stopSplitPlayback();
    clearSplitCommonBackup();
    PlaybackLayoutSplit::instance()->setSplitLayout(SplitLayout_0, 0);
    s_timelineBeginDateTime = QDateTime(playbackDate(), QTime(0, 0));
    s_playbackBar->setPlaybackButtonState(PlaybackState_None);

    //GlobalWaitting:://closeWait();
    qDebug() << "PlaybackSplit::closePlayback, end";
}

void PlaybackSplit::setFocus()
{
    ui->treeView_channel->setFocus();
    ui->treeView_channel->setCurrentRow(0);
    ui->treeView_channel->clearCheck();
    ui->treeView_channel->setItemChecked(0, true);
}

bool PlaybackSplit::hasFocus()
{
    bool result = false;
    if (ui->treeView_channel->hasFocus()) {
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

bool PlaybackSplit::focusPrevious()
{
    return false;
}

bool PlaybackSplit::focusNext()
{
    bool result = true;
    if (ui->pushButton_search->hasFocus()) {
        result = false;
    } else {
        focusNextChild();
    }
    return result;
}

NetworkResult PlaybackSplit::dealNetworkCommond(const QString &commond)
{
    NetworkResult result = NetworkReject;
    if (commond.startsWith("Enter")) {
        if (ui->treeView_channel->hasFocus()) {
            int row = ui->treeView_channel->currentIndex().row();
            if (row < 0) {
                result = NetworkReject;
            } else {
                onChannelClicked(row, 0);
                result = NetworkAccept;
            }
        }
    }
    return result;
}

MsSplitLayoutMode PlaybackSplit::layoutMode()
{
    return m_layoutMode;
}

void PlaybackSplit::setLayoutMode(const MsSplitLayoutMode &mode)
{
    if (isPlaying() && mode != m_layoutMode) {
        //
        closeAudio();
        stopSplitPlayback();
        clearSplitCommonBackup();
        //
        ui->treeView_channel->clearCheck();
        ui->treeView_channel->setCurrentRow(m_channel);
        ui->treeView_channel->setItemChecked(m_channel, true);
        //
        m_layoutMode = mode;
        s_playbackBar->updateSplitChannel();
        searchSplitPlayback(m_channel);
    } else {
        m_layoutMode = mode;
    }
}

int PlaybackSplit::channel()
{
    return m_channel;
}

bool PlaybackSplit::isPlaying()
{
    return m_isPlaying;
}

bool PlaybackSplit::hasCommonBackup()
{
    bool hasBackup = false;
    for (int i = 0; i < m_layoutMode; ++i) {
        if (hasCommonBackup(i)) {
            hasBackup = true;
            break;
        }
    }
    return hasBackup;
}

bool PlaybackSplit::hasCommonBackup(int sid)
{
    const PlaybackSplitInfo &info = m_splitInfoMap.value(sid);
    return info.hasBackup();
}

int PlaybackSplit::playingChannel()
{
    if (m_isPlaying) {
        return m_channel;
    } else {
        return -1;
    }
}

void PlaybackSplit::setSelectedSid(int sid)
{
    m_selectedSid = sid;
}

int PlaybackSplit::selectedSid()
{
    return m_selectedSid;
}

void PlaybackSplit::makeSelectedSidMask(char *mask, int size)
{
    memset(mask, 0, size);
    for (int i = 0; i < size; ++i) {
        if (i == selectedSid()) {
            mask[i] = '1';
        } else {
            mask[i] = '0';
        }
    }
}

void PlaybackSplit::makeSidMask(char *mask, int size)
{
    memset(mask, 0, size);
    for (int i = 0; i < size; ++i) {
        mask[i] = '1';
    }
}

resp_search_common_backup PlaybackSplit::findCommonBackup(int sid, const QDateTime &dateTime)
{
    resp_search_common_backup common_backup;
    memset(&common_backup, 0, sizeof(resp_search_common_backup));
    common_backup.chnid = -1;

    const QList<resp_search_common_backup> &common_backup_list = splitCommonBackupList(sid);
    for (int i = 0; i < common_backup_list.size(); ++i) {
        const resp_search_common_backup &backup = common_backup_list.at(i);
        const QDateTime &beginDateTime = QDateTime::fromString(QString(backup.pStartTime), "yyyy-MM-dd HH:mm:ss");
        const QDateTime &endDateTime = QDateTime::fromString(QString(backup.pEndTime), "yyyy-MM-dd HH:mm:ss");
        if (beginDateTime > dateTime || endDateTime < dateTime) {
            continue;
        }
        memcpy(&common_backup, &backup, sizeof(resp_search_common_backup));
        break;
    }
    return common_backup;
}

QList<resp_search_common_backup> PlaybackSplit::splitCommonBackupList(int sid)
{
    return m_splitInfoMap.value(sid).commonBackupList();
}

QList<resp_search_common_backup> PlaybackSplit::splitCommonBackupList()
{
    return splitCommonBackupList(selectedSid());
}

void PlaybackSplit::clearSplitCommonBackup()
{
    for (auto iter = m_splitInfoMap.constBegin(); iter != m_splitInfoMap.constEnd(); ++iter) {
        const PlaybackSplitInfo &info = iter.value();
        if (info.hasBackup()) {
            int sid = info.sid();
            MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_SPLIT_PLAYBACK_CLOSE, &sid, sizeof(sid));
        }
    }

    m_splitInfoMap.clear();
}

void PlaybackSplit::openAudio()
{
    if (m_audioSid == selectedSid()) {
        return;
    }

    m_audioSid = selectedSid();
    req_set_audiochn audio;
    audio.chn = m_audioSid;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, sid: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(req_set_audiochn));
}

void PlaybackSplit::closeAudio()
{
    if (m_audioSid == -1) {
        return;
    }

    m_audioSid = -1;
    req_set_audiochn audio;
    audio.chn = m_audioSid;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, sid: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(req_set_audiochn));
}

/**
 * @brief PlaybackSplit::isSelectedAudioOpen
 * 当前选择的窗口是否开启音频
 * @return
 */
bool PlaybackSplit::isSelectedAudioOpen()
{
    qDebug() << "audio sid:" << m_audioSid << ", selected sid:" << m_selectedSid;
    return m_audioSid >= 0 && m_audioSid == m_selectedSid;
}

/**
 * @brief PlaybackSplit::isAudioOpen
 * 分时回放是否开启音频
 * @return
 */
bool PlaybackSplit::isAudioOpen()
{
    qDebug() << "audio sid:" << m_audioSid << ", selected sid:" << m_selectedSid;
    return m_audioSid >= 0;
}

QString PlaybackSplit::splitSidMask()
{
    QString text;
    for (int i = 0; i < m_layoutMode; ++i) {
        const PlaybackSplitInfo &info = m_splitInfoMap.value(i);
        if (info.hasBackup()) {
            text.append("1");
        } else {
            text.append("0");
        }
    }
    return text;
}

void PlaybackSplit::searchSplitCommonPlaybackPage(int sid)
{
    const PlaybackSplitInfo &backInfo = m_splitInfoMap[sid];
    int pageCount = backInfo.pageCount();
    int backupsid = backInfo.sid();
    if (backupsid < 0) {
        qWarning() << QString("BasePlayback::searchSplitCommonPlaybackPage, invaild sid = %1").arg(sid);
        return;
    }

    struct rep_get_search_backup search_backup;
    memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
    search_backup.sid = backupsid;
    search_backup.npage = backInfo.nextCommonBackupPage();

    qDebug() << QString("REQUEST_FLAG_SEARCH_SPLIT_PLAYBACK_PAGE, sid: %1, page: %2/%3").arg(sid).arg(pageCount).arg(search_backup.npage);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_SPLIT_PLAYBACK_PAGE, (void *)&search_backup, sizeof(struct rep_get_search_backup));
}

void PlaybackSplit::searchSplitPlayback(int channel)
{
    QDateTime startDateTime(ui->dateEdit_start->date(), ui->timeEdit_start->time());
    QDateTime endDateTime(ui->dateEdit_end->date(), ui->timeEdit_end->time());
    s_playbackTimeLine->setTimeLineBeginDateTime(startDateTime);

    //
    setCurrentChannel(channel);
    setSelectedSid(0);
    setPlaybackSpeed(PLAY_SPEED_1X);
    s_playbackBar->updatePlaybackSpeedString();
    s_playbackBar->updateSplitChannel();
    s_playbackBar->clearSplitDateTimeMap();
    //
    req_search_split_pb_range search_range;
    memset(&search_range, 0, sizeof(search_range));
    search_range.chnid = channel;
    search_range.enType = playbackStream();
    search_range.enEvent = REC_EVENT_ALL;
    search_range.enState = SEG_STATE_ALL;
    snprintf(search_range.pStartTime, sizeof(search_range.pStartTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(search_range.pEndTime, sizeof(search_range.pEndTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    qDebug() << "----REQUEST_FLAG_SEARCH_SPLIT_PLAYBACK_RANGE"
             << ", chnid:" << (int)search_range.chnid
             << ", enType:" << search_range.enType
             << ", pStartTime:" << search_range.pStartTime
             << ", pEndTime:" << search_range.pEndTime;
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_SPLIT_PLAYBACK_RANGE, &search_range, sizeof(search_range));
    if (m_searchStartDateTime == m_searchEndDateTime) {
        PlaybackLayoutSplit::instance()->setSplitLayout(SplitLayout_0, 0);
        s_playbackBar->setPlaybackButtonState(PlaybackState_None);
        //GlobalWaitting:://closeWait();
        ShowMessageBox(GET_TEXT("DISK/92026", "No matching video files."));
        return;
    }
    //
    PlaybackLayoutSplit::instance()->setSplitLayout(m_layoutMode, 0);
    //
    int splitLayoutCount = layoutMode();
    int seconds = 0;
    int secondsCount = m_searchStartDateTime.secsTo(m_searchEndDateTime);
    if (secondsCount < 60) {
        //搜索结果如果小于1min，则只在窗口1播放对应搜索出来的所有录像
        splitLayoutCount = 1;
        seconds = secondsCount;
    } else {
        seconds = qCeil(secondsCount / (qreal)splitLayoutCount);
    }
    for (m_tempSearchSid = 0; m_tempSearchSid < splitLayoutCount; ++m_tempSearchSid) {
        PlaybackSplitInfo &info = m_splitInfoMap[m_tempSearchSid];
        QDateTime startDateTime = m_searchStartDateTime.addSecs(seconds * m_tempSearchSid);
        QDateTime endDateTime = m_searchStartDateTime.addSecs(seconds * (m_tempSearchSid + 1));
        if (startDateTime.time() >= QTime(0, 0, 2)) {
            startDateTime = startDateTime.addSecs(-2);
        }
        if (endDateTime.time() <= QTime(23, 59, 58)) {
            endDateTime = endDateTime.addSecs(2);
        }
        info.setDateTime(startDateTime, endDateTime);
        req_search_split_pb_backup search_backup;
        memset(&search_backup, 0, sizeof(search_backup));
        search_backup.chnid = channel;
        search_backup.sid = m_tempSearchSid;
        search_backup.enType = playbackStream();
        search_backup.enEvent = REC_EVENT_ALL;
        search_backup.enState = SEG_STATE_ALL;
        snprintf(search_backup.pStartTime, sizeof(search_backup.pStartTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        snprintf(search_backup.pEndTime, sizeof(search_backup.pEndTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        search_backup.all = MF_YES;
        qDebug() << "----REQUEST_FLAG_SEARCH_SPLIT_PLAYBACK_OPEN"
                 << ", chnid:" << (int)search_backup.chnid
                 << ", sid:" << search_backup.sid
                 << ", enType:" << search_backup.enType
                 << ", pStartTime:" << search_backup.pStartTime
                 << ", pEndTime:" << search_backup.pEndTime;
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_SPLIT_PLAYBACK_OPEN, &search_backup, sizeof(search_backup));
    }
    //
    if (hasCommonBackup()) {
        startSplitPlayback();
    }
    //
    ui->treeView_channel->setRowColor(m_channel, "#0AA8E3");
    //
    //GlobalWaitting:://closeWait();
}

void PlaybackSplit::onLanguageChanged()
{
    QStringList headerList;
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("COMMON/1051", "Name");
    ui->treeView_channel->setHorizontalHeaderLabels(headerList);

    ui->label_startTime->setText(GET_TEXT("IMAGE/37320", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("IMAGE/37321", "End Time"));
    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
}

void PlaybackSplit::startSplitPlayback()
{
    req_start_split_play start_play;
    memset(&start_play, 0, sizeof(start_play));
    start_play.chnid = m_channel;
    start_play.actState = PB_PLAY;
    start_play.actSpeed = PLAY_SPEED_1X;
    start_play.actdir = PB_FORWARD;
    start_play.enType = playbackStream();
    snprintf(start_play.sidMaskl, sizeof(start_play.sidMaskl), "%s", splitSidMask().toStdString().c_str());
    bool hasRecord = false;
    QString text;
    text.append("----REQUEST_FLAG_START_SPLIT_PLAYBACK----");
    text.append(QString("\n----chnid: %1").arg(start_play.chnid));
    for (int i = 0; i < m_layoutMode; ++i) {
        const PlaybackSplitInfo &info = m_splitInfoMap.value(i);
        if (info.hasBackup()) {
            hasRecord = true;
            start_play.sidMaskl[i] = '1';
        } else {
            start_play.sidMaskl[i] = '0';
        }
        snprintf(start_play.pStartTime[i], sizeof(start_play.pStartTime[i]), "%s", info.startDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        snprintf(start_play.pEndTime[i], sizeof(start_play.pEndTime[i]), "%s", info.endDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        snprintf(start_play.pPlayTime[i], sizeof(start_play.pPlayTime[i]), "%s", info.startDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        text.append(QString("\n----[%1]start: %2, end: %3, hasRecord: %4").arg(i).arg(start_play.pStartTime[i]).arg(start_play.pEndTime[i]).arg(info.hasBackup()));
    }
    qDebug() << text;
    if (hasRecord) {
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_START_SPLIT_PLAYBACK, &start_play, sizeof(start_play));
        m_isPlaying = true;
    }
}

void PlaybackSplit::stopSplitPlayback()
{
    if (m_isPlaying) {
        req_split_play stop_play;
        memset(&stop_play, 0, sizeof(stop_play));
        stop_play.chnid = m_channel;
        snprintf(stop_play.sidMaskl, sizeof(stop_play.sidMaskl), "%s", splitSidMask().toStdString().c_str());
        qDebug() << "REQUEST_FLAG_STOP_SPLIT_PLAYBACK, begin"
                 << "chnid:" << stop_play.chnid << "sidMaskl:" << stop_play.sidMaskl;
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_STOP_SPLIT_PLAYBACK, &stop_play, sizeof(stop_play));
        m_isPlaying = false;
        //m_eventloop.exec();
        qDebug() << "REQUEST_FLAG_STOP_SPLIT_PLAYBACK, end";
    }
}

void PlaybackSplit::pauseSplitPlayback()
{
    if (m_isPlaying) {
        req_split_play req;
        memset(&req, 0, sizeof(req));
        req.chnid = m_channel;
        snprintf(req.sidMaskl, sizeof(req.sidMaskl), "%s", splitSidMask().toStdString().c_str());
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_PAUSE_SPLIT_PLAYBACK, &req, sizeof(req));
    }
}

void PlaybackSplit::resumeSplitPlayback()
{
    if (m_isPlaying) {
        req_split_play req;
        memset(&req, 0, sizeof(req));
        req.chnid = m_channel;
        snprintf(req.sidMaskl, sizeof(req.sidMaskl), "%s", splitSidMask().toStdString().c_str());
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_RESTART_SPLIT_PLAYBACK, &req, sizeof(req));
    }
}

void PlaybackSplit::speedSplitPlayback(int speed)
{
    if (m_isPlaying) {
        req_split_play_speed req;
        memset(&req, 0, sizeof(req));
        req.chnid = m_channel;
        req.speed = speed;
        req.playallframe = 0;
        snprintf(req.sidMaskl, sizeof(req.sidMaskl), "%s", splitSidMask().toStdString().c_str());
        qDebug() << "REQUEST_FLAG_SPEED_SPLIT_PLAYBACK"
                 << ", channel:" << req.chnid
                 << ", speed:" << req.speed
                 << ", sidMaskl:" << req.sidMaskl;
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SPEED_SPLIT_PLAYBACK, &req, sizeof(req));
        //
        gPlaybackData.setPlaybackSpeed(speed);
    }
}

void PlaybackSplit::backwardSplitPlayback()
{
    if (m_isPlaying) {
        req_split_play req;
        memset(&req, 0, sizeof(req));
        req.chnid = m_channel;
        snprintf(req.sidMaskl, sizeof(req.sidMaskl), "%s", splitSidMask().toStdString().c_str());
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_BACKWARD_SPLIT_PLAYBACK, &req, sizeof(req));
    }
}

void PlaybackSplit::forwardSplitPlayback()
{
    if (m_isPlaying) {
        req_split_play req;
        memset(&req, 0, sizeof(req));
        req.chnid = m_channel;
        snprintf(req.sidMaskl, sizeof(req.sidMaskl), "%s", splitSidMask().toStdString().c_str());
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_FORWARD_SPLIT_PLAYBACK, &req, sizeof(req));
    }
}

void PlaybackSplit::stepSplitPlayback()
{
    if (m_isPlaying) {
        req_split_play req;
        memset(&req, 0, sizeof(req));
        req.chnid = m_channel;
        snprintf(req.sidMaskl, sizeof(req.sidMaskl), "%s", splitSidMask().toStdString().c_str());
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_STEP_SPLIT_PLAYBACK, &req, sizeof(req));
    }
}

void PlaybackSplit::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_RANGE:
        ON_RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_RANGE(message);
        break;
    case RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_OPEN:
        ON_RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_OPEN(message);
        break;
    case RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_PAGE:
        ON_RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_PAGE(message);
        break;
    case RESPONSE_FLAG_START_SPLIT_PLAYBACK:
        ON_RESPONSE_FLAG_START_SPLIT_PLAYBACK(message);
        break;
    case RESPONSE_FLAG_STOP_SPLIT_PLAYBACK:
        ON_RESPONSE_FLAG_STOP_SPLIT_PLAYBACK(message);
        break;
    }
}

void PlaybackSplit::ON_RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_RANGE(MessageReceive *message)
{
    resp_search_split_pb_range *search_range = (resp_search_split_pb_range *)message->data;
    if (search_range) {
        m_searchStartDateTime = QDateTime::fromTime_t(search_range->startTime);
        m_searchEndDateTime = QDateTime::fromTime_t(search_range->endTime);
        qDebug() << "----RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_RANGE"
                 << ", chnid:" << (int)search_range->chnid
                 << ", startTime:" << search_range->startTime << m_searchStartDateTime.toString("yyyy-MM-dd HH:mm:ss")
                 << ", endTime:" << search_range->endTime << m_searchEndDateTime.toString("yyyy-MM-dd HH:mm:ss");
        //m_eventloop.exit(0);
    } else {
        //m_eventloop.exit(-1);
    }
}

void PlaybackSplit::ON_RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_OPEN(MessageReceive *message)
{
    resp_search_common_backup *backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (!backup_array) {
        qMsDebug() << "resp_search_common_backup is null.";
    } else {
        qDebug() << "----RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_OPEN"
                 << ", chnid:" << backup_array->chnid
                 << ", sid:" << backup_array->sid
                 << ", allCnt:" << backup_array->allCnt
                 << ", pStartTime:" << backup_array->pStartTime;
        if (backup_array->allCnt == 0) {

        } else {
            const int &sid = backup_array->sid;
            PlaybackSplitInfo &info = m_splitInfoMap[sid];
            info.appendCommonBackup(backup_array, count);
            if (info.commonBackupListSize() < backup_array->allCnt) {
                //继续搜索
                searchSplitCommonPlaybackPage(sid);
                return;
            }
        }
    }
    //m_eventloop.exit();

    updateTimeLine();
}

void PlaybackSplit::ON_RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_PAGE(MessageReceive *message)
{
    resp_search_common_backup *backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (!backup_array) {
        qMsDebug() << "resp_search_common_backup is null.";
    } else {
        qDebug() << "----RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_OPEN"
                 << ", chnid:" << backup_array->chnid
                 << ", sid:" << backup_array->sid
                 << ", allCnt:" << backup_array->allCnt
                 << ", pStartTime:" << backup_array->pStartTime;
        if (backup_array->allCnt == 0) {

        } else {
            const int &sid = backup_array->sid;
            PlaybackSplitInfo &info = m_splitInfoMap[sid];
            info.appendCommonBackup(backup_array, count);
            if (info.commonBackupListSize() < backup_array->allCnt) {
                //继续搜索
                searchSplitCommonPlaybackPage(sid);
                return;
            }
        }
    }
    //m_eventloop.exit();

    updateTimeLine();
}

void PlaybackSplit::ON_RESPONSE_FLAG_START_SPLIT_PLAYBACK(MessageReceive *message)
{
    resp_split_pb_state *pb_state = (resp_split_pb_state *)message->data;
    if (pb_state) {
        qDebug() << "----RESPONSE_FLAG_START_SPLIT_PLAYBACK"
                 << ", chnid:" << pb_state->chnid
                 << ", sidMask:" << pb_state->sidMask
                 << ", stateMask:" << pb_state->stateMask;
    }
    //m_eventloop.exit();

    s_playbackBar->setPlaybackButtonState(PlaybackState_Play);
}

void PlaybackSplit::ON_RESPONSE_FLAG_STOP_SPLIT_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)

    //m_eventloop.exit();
}

void PlaybackSplit::selectNextChannel()
{
    int channel = ui->treeView_channel->currentIndex().row();
    if (channel < 0) {
        channel = 0;
    } else if (channel < ui->treeView_channel->rowCount() - 1) {
        channel++;
    }
    ui->treeView_channel->setCurrentRow(channel);
    ui->treeView_channel->clearCheck();
    ui->treeView_channel->setItemChecked(channel, true);
}

void PlaybackSplit::onChannelClicked(int row, int column)
{
    if (column == ColumnChannel) {
        ui->treeView_channel->clearCheck();
        ui->treeView_channel->setItemChecked(row, true);
    }
}

void PlaybackSplit::onTreeViewEnterPressed()
{
    int row = ui->treeView_channel->currentIndex().row();
    if (row < 0) {

    } else {
        onChannelClicked(row, 0);
    }
}

void PlaybackSplit::on_pushButton_search_clicked()
{
    ui->pushButton_search->clearUnderMouse();
    ui->treeView_channel->clearItemColor();

    PlaybackWindow::instance()->closeFisheyeDewarp();
    if (PlaybackZoom::instance()->isZooming()) {
        PlaybackZoom::instance()->closeZoom();
    }

    QDateTime startDateTime(ui->dateEdit_start->date(), ui->timeEdit_start->time());
    QDateTime endDateTime(ui->dateEdit_end->date(), ui->timeEdit_end->time());

    if (startDateTime >= endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }

    int secs = startDateTime.secsTo(endDateTime);
    if (secs > 86400) {
        ShowMessageBox(GET_TEXT("PLAYBACK/80124", "Time period should be within 24 hours."));
        return;
    }

    int row = ui->treeView_channel->firstCheckedRow();
    if (row < 0) {
        ShowMessageBox(GET_TEXT("PLAYBACK/80129", "Please select one channel."));
        return;
    }

    //
    closeAudio();
    stopSplitPlayback();
    clearSplitCommonBackup();
    //
    m_channel = ui->treeView_channel->itemData(row, ColumnChannel, ChannelRole).toInt();
    searchSplitPlayback(m_channel);
    //
    s_playbackBar->updateSplitChannel();
}
