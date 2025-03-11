#include "BasePlayback.h"
#include "FilterEventPanel.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "PlaybackBar.h"
#include "PlaybackData.h"
#include "PlaybackGeneral.h"
#include "PlaybackLayout.h"
#include "PlaybackSplit.h"
#include "PlaybackTagData.h"
#include "PlaybackTimeLine.h"
#include "SmartSearchControl.h"
#include "SmartSpeedPanel.h"
#include "centralmessage.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QElapsedTimer>
#include <QStyle>

PlaybackMode *BasePlayback::s_playbackMode = nullptr;
PlaybackGeneral *BasePlayback::s_playbackGeneral = nullptr;
PlaybackLayout *BasePlayback::s_playbackLayout = nullptr;
PlaybackBar *BasePlayback::s_playbackBar = nullptr;
PlaybackTimeLine *BasePlayback::s_playbackTimeLine = nullptr;

ThumbWidget *BasePlayback::s_thumbWidget = nullptr;
SmartSpeedPanel *BasePlayback::s_smartSpeed = nullptr;
SmartSearchControl *BasePlayback::s_smartSearch = nullptr;
FilterEventPanel *BasePlayback::s_filterEventPanel = nullptr;

#ifdef MS_FISHEYE_SOFT_DEWARP
FisheyeDewarpControl *BasePlayback::s_fisheyeControl = nullptr;
#endif

QEventLoop *BasePlayback::s_eventLoop = nullptr;
QMap<int, PlaybackChannelInfo> BasePlayback::s_channelInfoMap;
bool BasePlayback::s_isWaitting = false;
MsWaitting *BasePlayback::s_waitting = nullptr;
MsPlaybackType BasePlayback::s_playbackType = GeneralPlayback;
FILE_TYPE_EN BasePlayback::s_playbackStream = FILE_TYPE_MAIN;
PLAY_SPEED BasePlayback::s_playbackSpeed = PLAY_SPEED_1X;
PB_ACTION_EN BasePlayback::s_playbackDirection = PB_FORWARD;
QDate BasePlayback::s_playbackDate;
QTime BasePlayback::s_playbackTime;
QList<int> BasePlayback::s_checkedChannelList;
int BasePlayback::s_currentChannel = -1;
MsPlaybackState BasePlayback::s_playbackState = PlaybackState_None;
QMap<int, MsPlayState> BasePlayback::s_channelPlayStateMap;
int BasePlayback::s_audioChannel = -1;
QMap<int, QMap<QDateTime, int>> BasePlayback::s_drawTagMap;
int BasePlayback::s_fisheyeDewarpState = 0;
bool BasePlayback::s_isEventOnly = false;
int BasePlayback::s_filterEvent = INFO_MAJOR_NONE;
//
QDateTime BasePlayback::s_timelineBeginDateTime;
QDateTime BasePlayback::s_timelineEndDateTime;

BasePlayback::BasePlayback(QWidget *parent)
    : MsWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

QString BasePlayback::playbackStateString(const MsPlaybackState &state)
{
    QString text;
    switch (state) {
    case PlaybackState_None:
        text = "PlaybackState_None";
        break;
    case PlaybackState_Stop:
        text = "PlaybackState_Stop";
        break;
    case PlaybackState_Play:
        text = "PlaybackState_Play";
        break;
    case PlaybackState_Pause:
        text = "PlaybackState_Pause";
        break;
    case PlaybackState_Reverse:
        text = "PlaybackState_Reverse";
        break;
    case PlaybackState_ReverseStep:
        text = "PlaybackState_ReverseStep";
        break;
    case PlaybackState_Forward:
        text = "PlaybackState_Forward";
        break;
    case PlaybackState_ForwardStep:
        text = "PlaybackState_ForwardStep";
        break;
    default:
        text = QString("Unknow(%1)").arg(state);
        break;
    }
    return text;
}

QString BasePlayback::playbackSpeedString(PLAY_SPEED speed)
{
    QString text;
    switch (speed) {
    case PLAY_SPEED_0_03125X:
        text = "PLAY_SPEED_1/32X";
        break;
    case PLAY_SPEED_0_0625X:
        text = "PLAY_SPEED_1/16X";
        break;
    case PLAY_SPEED_0_125X:
        text = "PLAY_SPEED_1/8X";
        break;
    case PLAY_SPEED_0_25X:
        text = "PLAY_SPEED_1/4X";
        break;
    case PLAY_SPEED_0_5X:
        text = "PLAY_SPEED_1/2X";
        break;
    case PLAY_SPEED_1X:
        text = "PLAY_SPEED_1X";
        break;
    case PLAY_SPEED_2X:
        text = "PLAY_SPEED_2X";
        break;
    case PLAY_SPEED_4X:
        text = "PLAY_SPEED_4X";
        break;
    case PLAY_SPEED_8X:
        text = "PLAY_SPEED_8X";
        break;
    case PLAY_SPEED_16X:
        text = "PLAY_SPEED_16X";
        break;
    case PLAY_SPEED_32X:
        text = "PLAY_SPEED_32X";
        break;
    case PLAY_SPEED_64X:
        text = "PLAY_SPEED_64X";
        break;
    case PLAY_SPEED_128X:
        text = "PLAY_SPEED_128X";
        break;
    default:
        text = QString("Unknow(%1)").arg(speed);
        break;
    }
    return text;
}

void BasePlayback::initializeBasePlayback(QWidget *parent)
{
    //
    if (!s_waitting) {
        s_waitting = new MsWaitting(parent);
        s_waitting->setWindowModality(Qt::WindowModal);
    }
    const QRect &screenRect = qApp->desktop()->screenGeometry();
    s_waitting->move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, s_waitting->size(), screenRect).topLeft());

    if (!s_eventLoop) {
        s_eventLoop = new QEventLoop(parent);
    }

    //
    s_isEventOnly = false;
    s_filterEvent = INFO_MAJOR_NONE;
    s_fisheyeDewarpState = 0;
}

void BasePlayback::clearupBasePlayback()
{
    s_channelInfoMap.clear();
    s_playbackType = GeneralPlayback;
    s_playbackStream = FILE_TYPE_MAIN;
    s_playbackSpeed = PLAY_SPEED_1X;
    s_playbackDirection = PB_FORWARD;
    s_checkedChannelList.clear();
    s_currentChannel = -1;
    s_playbackState = PlaybackState_None;
    s_audioChannel = -1;
}

void BasePlayback::setSelectedChannel(int channel)
{
    s_playbackGeneral->setCurrentSelectedChannel(channel);
}

void BasePlayback::updateLayout(int channel)
{
    s_playbackLayout->adjustLayout(channel);
}

void BasePlayback::setCurrentVideoInLayout(int channel)
{
    s_playbackLayout->setCurrentVideo(channel);
}

void BasePlayback::initializeTimeLine()
{
    s_playbackBar->initializeTimeLine();
}

void BasePlayback::setCurrentTimeLine(int channel)
{
    s_playbackBar->setCurrentTimeLine(channel);
}

void BasePlayback::setCurrentTimeLine()
{
    s_playbackBar->setCurrentTimeLine(s_currentChannel);
}

void BasePlayback::updateTimeLine()
{
    s_playbackBar->updateTimeLine();
}

void BasePlayback::setTimeLineStartDateTime(const QDateTime &dateTime)
{
    s_playbackTimeLine->setTimeLineBeginDateTime(QDateTime(dateTime.date(), QTime(0, 0)));
}

bool BasePlayback::isSmartSearchMode()
{
    if (s_smartSearch) {
        return s_smartSearch->isSmartSearchMode();
    }
    return false;
}

void BasePlayback::getMonthEvent()
{
    if (gDebug.checkCategorySet("pb_search_month", "off")) {
        return;
    }

    rep_get_month_event month_event;
    memset(&month_event, 0, sizeof(struct rep_get_month_event));
    month_event.enType = playbackStream();
    month_event.year = playbackDate().year();
    month_event.month = playbackDate().month();
    makeChannelMask(month_event.chnMaskl, sizeof(month_event.chnMaskl));

    qDebug() << "====REQUEST_FLAG_GET_MONTH_EVENT====";
    qDebug() << "----enType:" << month_event.enType;
    qDebug() << "----year:" << month_event.year;
    qDebug() << "----month:" << month_event.month;
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_MONTH_EVENT, (void *)&month_event, sizeof(rep_get_month_event));
}

void BasePlayback::searchCommonPlayback(int channel)
{
    s_channelInfoMap[channel].clearCommonBackup();
    //
    req_search_common_backup common_backup;
    memset(&common_backup, 0, sizeof(req_search_common_backup));

    makeChannelMask(channel, common_backup.chnMaskl, sizeof(common_backup.chnMaskl));
    common_backup.chnNum = 1;
    common_backup.enType = playbackStream();
    common_backup.enEvent = REC_EVENT_ALL;
    common_backup.enState = SEG_STATE_ALL;
    snprintf(common_backup.pStartTime, sizeof(common_backup.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    snprintf(common_backup.pEndTime, sizeof(common_backup.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());

    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_COM_PLAYBACK_OPEN, (void *)&common_backup, sizeof(req_search_common_backup));
}

void BasePlayback::searchCommonPlaybackPage(int channel)
{
    const PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
    int sid = channelInfo.sid();
    if (sid < 0) {
        qWarning() << QString("BasePlayback::searchCommonPlaybackPage, invaild sid = %1, channel = %2").arg(sid).arg(channel);
        return;
    }

    struct rep_get_search_backup search_backup;
    memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
    search_backup.sid = sid;
    search_backup.npage = channelInfo.nextCommonBackupPage();

    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_COM_PLAYBACK_PAGE, (void *)&search_backup, sizeof(struct rep_get_search_backup));
}

void BasePlayback::waitForSearchCommonPlayback(int channel)
{
    searchCommonPlayback(channel);
    execWait();
}

void BasePlayback::closeCommonPlayback(int channel)
{
    PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
    int sid = channelInfo.sid();
    if (sid >= 0) {
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_COM_PLAYBACK_CLOSE, (void *)&(sid), sizeof(int));
        channelInfo.clearCommonBackup();
    }
}

void BasePlayback::closeAllCommonPlayback()
{
    for (auto iter = s_channelInfoMap.begin(); iter != s_channelInfoMap.end(); ++iter) {
        PlaybackChannelInfo &channelInfo = iter.value();
        int sid = channelInfo.sid();
        if (sid >= 0) {
            MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_COM_PLAYBACK_CLOSE, (void *)&(sid), sizeof(int));
            channelInfo.clearCommonBackup();
        }
    }
}

void BasePlayback::searchEventPlayback(int channel)
{
    qDebug() << "----channel(0-63):" << channel;
    if (!isChannelChecked(channel)) {
        qDebug() << "----channel checked:" << false;
        exitWait();
        return;
    }
    if (s_channelInfoMap.contains(channel)) {
        const PlaybackChannelInfo &channelInfo = s_channelInfoMap.value(channel);
        if (channelInfo.hasSearchEventBackup(playbackDate())) {
            qDebug() << "----channel has searched EventBackup:" << true;
            exitWait();
            return;
        }
    }

    qDebug() << "----begin search";
    //
    PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
    channelInfo.clearEventBackup();
    channelInfo.setEventSearchState(playbackDate(), EventSearching);

    //
    struct req_search_event_backup event_backup;
    memset(&event_backup, 0, sizeof(req_search_event_backup));

    struct ms_socket_packet ms_packet;
    memset(&ms_packet, 0, sizeof(struct ms_socket_packet));

    makeChannelMask(channel, event_backup.chnMaskl, sizeof(event_backup.chnMaskl));
    event_backup.chnNum = 1;

    ms_packet.nSize = sizeof(struct req_search_event_backup);
    ms_packet.nHeaderSize = ms_packet.nSize;
    ms_packet.nBodySize = 0;
    ms_packet.packet = (char *)ms_malloc(ms_packet.nSize);
    ms_packet.packetHeader = ms_packet.packet;
    ms_packet.packetBody = NULL;

    event_backup.enType = playbackStream();
    event_backup.enMajor = INFO_MAJOR_MOTION;
    event_backup.ahead = 0;
    event_backup.delay = 0;

    snprintf(event_backup.pStartTime, sizeof(event_backup.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    snprintf(event_backup.pEndTime, sizeof(event_backup.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());

    memcpy(ms_packet.packet, &event_backup, sizeof(struct req_search_event_backup));
    qMsDebug() << "\n----REQUEST_FLAG_SEARCH_EVT_BACKUP----"
               << "\n----chnMaskl:" << event_backup.chnMaskl
               << "\n----chnNum:" << event_backup.chnNum
               << "\n----enType:" << event_backup.enType
               << "\n----enMajor:" << event_backup.enMajor
               << "\n----ahead:" << event_backup.ahead
               << "\n----delay:" << event_backup.delay
               << "\n----pStartTime:" << event_backup.pStartTime
               << "\n----pEndTime:" << event_backup.pEndTime;
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_EVT_BACKUP, (void *)ms_packet.packet, ms_packet.nSize);

    if (ms_packet.packet) {
        ms_free(ms_packet.packet);
    }
}

void BasePlayback::searchEventPlaybackPage(int channel)
{
    const PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
    int sid = channelInfo.eventSid();
    if (sid < 0) {
        qWarning() << QString("BasePlayback::searchEventPlaybackPage, invaild sid = %1, channel = %2").arg(sid).arg(channel);
        return;
    }

    struct rep_get_search_backup search_backup;
    memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
    search_backup.sid = sid;
    search_backup.npage = channelInfo.nextEventBackupPage();
    qDebug() << QString("BasePlayback::searchEventPlaybackPage, channel: %1, sid: %2, page: %3/%4").arg(channel).arg(sid).arg(search_backup.npage).arg(channelInfo.eventBackupPageCount());
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_SEARCH_EVT_PAGE, (void *)&search_backup, sizeof(struct rep_get_search_backup));
}

void BasePlayback::waitForSearchEventPlayback(int channel)
{
    searchEventPlayback(channel);
    execWait();
}

void BasePlayback::searchEventPlayback_smartSearch(int channel, char *region)
{
    qDebug() << "----channel(0-63):" << channel;
    if (!isChannelChecked(channel)) {
        qDebug() << "----channel checked:" << false;
        exitWait();
        return;
    }

    qDebug() << "----begin search";
    //
    PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
    channelInfo.clearEventBackup();
    channelInfo.setEventSearchState(playbackDate(), EventSearching);

    //
    struct req_search_event_backup event_backup;
    memset(&event_backup, 0, sizeof(req_search_event_backup));

    struct ms_socket_packet ms_packet;
    memset(&ms_packet, 0, sizeof(struct ms_socket_packet));

    makeChannelMask(channel, event_backup.chnMaskl, sizeof(event_backup.chnMaskl));
    event_backup.chnNum = 1;

    ms_packet.nSize = sizeof(struct req_search_event_backup);
    ms_packet.nHeaderSize = ms_packet.nSize;
    ms_packet.nBodySize = 0;
    ms_packet.packet = (char *)ms_malloc(ms_packet.nSize);
    ms_packet.packetHeader = ms_packet.packet;
    ms_packet.packetBody = NULL;

    event_backup.enType = playbackStream();
    event_backup.enMajor = INFO_MAJOR_MOTION;
    event_backup.ahead = 0;
    event_backup.delay = 0;

    snprintf(event_backup.pStartTime, sizeof(event_backup.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    snprintf(event_backup.pEndTime, sizeof(event_backup.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());

    if (region) {
        memcpy(event_backup.pMotMap, region, sizeof(event_backup.pMotMap));
    }

    memcpy(ms_packet.packet, &event_backup, sizeof(struct req_search_event_backup));
    qMsDebug() << "\n----REQUEST_FLAG_SEARCH_EVT_BACKUP----"
               << "\n----chnMaskl:" << event_backup.chnMaskl
               << "\n----chnNum:" << event_backup.chnNum
               << "\n----enType:" << event_backup.enType
               << "\n----enMajor:" << event_backup.enMajor
               << "\n----ahead:" << event_backup.ahead
               << "\n----delay:" << event_backup.delay
               << "\n----pStartTime:" << event_backup.pStartTime
               << "\n----pEndTime:" << event_backup.pEndTime;
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_EVT_BACKUP, (void *)ms_packet.packet, ms_packet.nSize);

    if (ms_packet.packet) {
        ms_free(ms_packet.packet);
    }
}

void BasePlayback::searchEventPlaybackPage_smartSearch(int channel)
{
    const PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
    int sid = channelInfo.eventSid_smartSearch();
    if (sid < 0) {
        qWarning() << QString("BasePlayback::searchEventPlaybackPage_smartSearch, invaild sid = %1, channel = %2").arg(sid).arg(channel);
        return;
    }

    struct rep_get_search_backup search_backup;
    memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
    search_backup.sid = sid;
    search_backup.npage = channelInfo.nextEventBackupPage_smartSearch();
    qDebug() << QString("BasePlayback::searchEventPlaybackPage_smartSearch, channel: %1, sid: %2, page: %3/%4").arg(channel).arg(sid).arg(search_backup.npage).arg(channelInfo.eventBackupPageCount_smartSearch());
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_SEARCH_EVT_PAGE, (void *)&search_backup, sizeof(struct rep_get_search_backup));
}

void BasePlayback::waitForSearchEventPlayback_smartSearch(int channel, char *region)
{
    searchEventPlayback_smartSearch(channel, region);
    execWait();
}

void BasePlayback::searchTagPlayback(int channel)
{
    struct req_search_tags search;
    memset(&search, 0, sizeof(struct req_search_tags));
    search.chnNum = 1;
    makeChannelMask(channel, search.chnMaskl, sizeof(search.chnMaskl));
    search.enType = playbackStream();
    snprintf(search.pStartTime, sizeof(search.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    snprintf(search.pEndTime, sizeof(search.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    gPlaybackTagData.searchTag(search);
}

void BasePlayback::closeEventPlayback(int channel)
{
    PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
    int sid = channelInfo.eventSid();
    if (sid >= 0) {
        //MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_EVT_BACKUP_CLOSE, (void *)&(sid), sizeof(int));
        channelInfo.clearEventBackup();
    }
}

void BasePlayback::closeAllEventPlayback()
{
    for (auto iter = s_channelInfoMap.begin(); iter != s_channelInfoMap.end(); ++iter) {
        PlaybackChannelInfo &channelInfo = iter.value();
        int sid = channelInfo.eventSid();
        if (sid >= 0) {
            //qDebug() << "closeAllEventPlayback" << iter.key();
            //MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_EVT_BACKUP_CLOSE, (void *)&(sid), sizeof(int));
            channelInfo.clearEventBackup();
        }
    }
}

void BasePlayback::startPlayback(int channel)
{
    if (playbackState() == PlaybackState_Pause) {
        resumeAllPlayback();
    }

    rep_start_all_play start_all;
    memset(&start_all, 0, sizeof(rep_start_all_play));
    start_all.actState = PB_PLAY;
    start_all.actSpeed = playbackSpeed();
    start_all.actdir = playbackDirection();
    makeChannelMask(channel, start_all.chnMaskl, sizeof(start_all.chnMaskl));

    snprintf(start_all.pPlayTime, sizeof(start_all.pPlayTime), "%s", QDateTime(playbackDate(), playbackTime()).toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(start_all.pStartTime, sizeof(start_all.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    snprintf(start_all.pEndTime, sizeof(start_all.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());

    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_START_ALL_PLAYBACK, (void *)&start_all, sizeof(rep_start_all_play));
}

void BasePlayback::startAllPlayback()
{
    if (playbackState() == PlaybackState_Pause) {
        resumeAllPlayback();
    }

    rep_start_all_play start_all;
    memset(&start_all, 0, sizeof(rep_start_all_play));
    start_all.actState = PB_PLAY;
    start_all.actSpeed = playbackSpeed();
    start_all.actdir = playbackDirection();
    makeChannelMask(start_all.chnMaskl, sizeof(start_all.chnMaskl));

    snprintf(start_all.pPlayTime, sizeof(start_all.pPlayTime), "%s", QDateTime(playbackDate(), playbackTime()).toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(start_all.pStartTime, sizeof(start_all.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    snprintf(start_all.pEndTime, sizeof(start_all.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());

    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_START_ALL_PLAYBACK, (void *)&start_all, sizeof(rep_start_all_play));
}

void BasePlayback::waitForStartPlayback(int channel)
{
    startPlayback(channel);
    execWait();
}

void BasePlayback::waitForStartAllPlayback()
{
    startAllPlayback();
    execWait();
}

void BasePlayback::stopPlayback(int channel)
{
    char chnMaskl[MAX_LEN_64] = { 0 };
    makeChannelMask(channel, chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_STOP_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));
}

void BasePlayback::stopAllPlayback()
{
    char chnMaskl[MAX_LEN_64] = { 0 };
    makeChannelMask(chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_STOP_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));
}

void BasePlayback::waitForStopPlayback(int channel)
{
    stopPlayback(channel);
    execWait();
}

void BasePlayback::waitForStopAllPlayback()
{
    stopAllPlayback();
    execWait();
}

void BasePlayback::resumeAllPlayback()
{
    char chnMaskl[MAX_LEN_64] = { 0 };
    makeChannelMask(chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_FORWARD_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_RESTART_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));
}

void BasePlayback::waitForResumeAllPlayback()
{
    resumeAllPlayback();
    execWait();
}

void BasePlayback::restartAllPlayback()
{
    char chnMaskl[MAX_LEN_64] = { 0 };
    makeChannelMask(chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_RESTART_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));
}

void BasePlayback::waitForRestartAllPlayback()
{
    restartAllPlayback();
    execWait();
}

void BasePlayback::pauseAllPlayback()
{
    char chnMaskl[MAX_LEN_64] = { 0 };
    makeChannelMask(chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_PAUSE_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));
}

void BasePlayback::backwardAllPlayback()
{
    char chnMaskl[MAX_LEN_64] = { 0 };
    makeChannelMask(chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_BACKWARD_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_RESTART_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));

    setPlaybackDirection(PB_BACKWARD);
}

void BasePlayback::speedUpAllPlayback()
{
    PLAY_SPEED speed = playbackSpeed();
    if (speed < PLAY_SPEED_128X) {
        speed = static_cast<PLAY_SPEED>(speed + 1);
        if (playbackType() == SplitPlayback) {
            s_playbackSpeed = speed;

            PlaybackSplit *split = PlaybackSplit::instance();
            split->speedSplitPlayback(speed);
        } else {
            setPlaybackSpeed(speed);
        }
    }
}

void BasePlayback::speedDownAllPlayback()
{
    PLAY_SPEED speed = playbackSpeed();
    if (speed > PLAY_SPEED_0_125X) {
        speed = static_cast<PLAY_SPEED>(speed - 1);
        if (playbackType() == SplitPlayback) {
            s_playbackSpeed = speed;

            PlaybackSplit *split = PlaybackSplit::instance();
            split->speedSplitPlayback(speed);
        } else {
            setPlaybackSpeed(speed);
        }
    }
}

void BasePlayback::stepForwardAllPlayback()
{
    char chnMaskl[MAX_LEN_64] = { 0 };
    makeChannelMask(chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_FORWARD_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_STEP_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));

    if (playbackDirection() != PB_FORWARD) {
        setPlaybackDirection(PB_FORWARD);
    }

    //
    s_smartSearch->stepForward();
    s_smartSpeed->stepForward();
}

void BasePlayback::stepBackwardAllPlayback()
{
    char chnMaskl[MAX_LEN_64] = { 0 };
    makeChannelMask(chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_BACKWARD_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_STEP_ALL_PLAYBACK, (void *)&chnMaskl, sizeof(chnMaskl));

    if (playbackDirection() != PB_BACKWARD) {
        setPlaybackDirection(PB_BACKWARD);
    }

    //
    s_smartSearch->stepBackward();
    s_smartSpeed->stepBackward();
}

void BasePlayback::openPlaybackAudio(int channel)
{
    if (s_audioChannel == channel) {
        return;
    }
    s_audioChannel = channel;

    req_set_audiochn audio;
    audio.chn = s_audioChannel;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(req_set_audiochn));
}

void BasePlayback::closePlaybackAudio()
{
    if (s_audioChannel == -1) {
        return;
    }
    s_audioChannel = -1;

    req_set_audiochn audio;
    audio.chn = s_audioChannel;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(req_set_audiochn));
}

int BasePlayback::audioChannel()
{
    return s_audioChannel;
}

void BasePlayback::makeChannelMask(QList<int> list, char *mask, int size)
{
    memset(mask, 0, size);

    for (int i = 0; i < size; ++i) {
        if (list.contains(i)) {
            mask[i] = '1';
        } else {
            mask[i] = '0';
        }
    }
}

void BasePlayback::makeChannelMask(int channel, char *mask, int size)
{
    memset(mask, 0, size);

    for (int i = 0; i < size; ++i) {
        if (i == channel) {
            mask[i] = '1';
        } else {
            mask[i] = '0';
        }
    }
}

void BasePlayback::makeChannelMask(char *mask, int size)
{
    memset(mask, 0, size);

    for (int i = 0; i < size; ++i) {
        if (s_checkedChannelList.contains(i)) {
            mask[i] = '1';
        } else {
            mask[i] = '0';
        }
    }
}

bool BasePlayback::isWaitting()
{
    return s_isWaitting;
}

void BasePlayback::showWait()
{
    s_isWaitting = true;
    //s_waitting->//showWait();
}

void BasePlayback::closeWait()
{
    s_isWaitting = false;
    //s_waitting->//closeWait();
}

void BasePlayback::execWait()
{
    s_eventLoop->exec();
}

void BasePlayback::exitWait()
{
    s_eventLoop->exit();
}

MsPlaybackType BasePlayback::playbackType()
{
    return s_playbackType;
}

void BasePlayback::setPlaybackType(MsPlaybackType mode)
{
    s_playbackType = mode;
}

FILE_TYPE_EN BasePlayback::playbackStream()
{
    return s_playbackStream;
}

void BasePlayback::setPlaybackStream(FILE_TYPE_EN stream)
{
    s_playbackStream = stream;
}

PLAY_SPEED BasePlayback::playbackSpeed()
{
    return s_playbackSpeed;
}

void BasePlayback::setPlaybackSpeed(PLAY_SPEED speed)
{
    s_playbackSpeed = speed;

    int channelCount = channelCheckedCount();
    if (channelCount > 0) {
        req_set_all_play_speed speedinfo;
        memset(&speedinfo, 0, sizeof(req_set_all_play_speed));
        makeChannelMask(speedinfo.chnMaskl, sizeof(speedinfo.chnMaskl));
        speedinfo.speed = speed;
        if (channelCount == 1 && speed >= PLAY_SPEED_1X && speed <= PLAY_SPEED_4X) {
            speedinfo.playallframe = 1;
        } else {
            speedinfo.playallframe = 0;
        }
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SPEED_ALL_PLAYBACK, (void *)&speedinfo, sizeof(req_set_all_play_speed));
        //
        gPlaybackData.setPlaybackSpeed(speed);
    }
}

QString BasePlayback::playbackSpeedString()
{
    QString strSpeed;
    const PLAY_SPEED &speed = playbackSpeed();
    switch (speed) {
    case PLAY_SPEED_0_125X:
        strSpeed = "1/8X";
        break;
    case PLAY_SPEED_0_25X:
        strSpeed = "1/4X";
        break;
    case PLAY_SPEED_0_5X:
        strSpeed = "1/2X";
        break;
    case PLAY_SPEED_1X:
        strSpeed = "1X";
        break;
    case PLAY_SPEED_2X:
        strSpeed = "2X";
        break;
    case PLAY_SPEED_4X:
        strSpeed = "4X";
        break;
    case PLAY_SPEED_8X:
        strSpeed = "8X";
        break;
    case PLAY_SPEED_16X:
        strSpeed = "16X";
        break;
    case PLAY_SPEED_32X:
        strSpeed = "32X";
        break;
    case PLAY_SPEED_64X:
        strSpeed = "64X";
        break;
    case PLAY_SPEED_128X:
        strSpeed = "128X";
        break;
    default:
        break;
    }
    return strSpeed;
}

void BasePlayback::seekPlayback(const QDateTime &dateTime)
{
    req_seek_all_play_time seek_all_play_time;
    memset(&seek_all_play_time, 0, sizeof(req_seek_all_play_time));
    makeChannelMask(seek_all_play_time.chnMaskl, sizeof(seek_all_play_time.chnMaskl));
    snprintf(seek_all_play_time.pseektime, sizeof(seek_all_play_time.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEEK_ALL_PLAYBACK, (void *)&seek_all_play_time, sizeof(req_seek_all_play_time));
}

PB_ACTION_EN BasePlayback::playbackDirection()
{
    return s_playbackDirection;
}

void BasePlayback::setPlaybackDirection(PB_ACTION_EN direction)
{
    s_playbackDirection = direction;
    //
    s_smartSearch->changePlayDirection();
    s_smartSpeed->changePlayDirection();
}

QDate BasePlayback::playbackDate()
{
    return s_playbackDate;
}

void BasePlayback::setPlaybackDate(const QDate &date)
{
    s_playbackDate = date;
}

QTime BasePlayback::playbackTime()
{
    return s_playbackTime;
}

void BasePlayback::setPlaybackTime(const QTime &time)
{
    s_playbackTime = time;
}

QDateTime BasePlayback::playbackDateTime()
{
    return QDateTime(playbackDate(), playbackTime());
}

QList<resp_search_common_backup> BasePlayback::commonBackupList(int channel)
{
    return s_channelInfoMap.value(channel).commonBackupList();
}

QDateTime BasePlayback::commonBackupEndDateTime(int channel)
{
    QDateTime dateTime;
    const QList<resp_search_common_backup> &backupList = commonBackupList(channel);
    if (!backupList.isEmpty()) {
        dateTime = QDateTime::fromString(backupList.last().pEndTime, "yyyy-MM-dd HH:mm:ss");
    }
    return dateTime;
}

QList<resp_search_event_backup> BasePlayback::eventBackupList(int channel)
{
    return s_channelInfoMap.value(channel).eventBackupList();
}

QList<resp_search_event_backup> BasePlayback::eventBackupList_smartSearch(int channel)
{
    return s_channelInfoMap.value(channel).eventBackupList_smartSearch();
}

bool BasePlayback::isChannelHasCommonBackup(int channel)
{
    if (!s_channelInfoMap.contains(channel)) {
        return false;
    }
    return s_channelInfoMap.value(channel).hasCommonBackup();
}

resp_search_common_backup BasePlayback::findCommonBackup(int channel, const QDateTime &dateTime)
{
    resp_search_common_backup common_backup;
    memset(&common_backup, 0, sizeof(resp_search_common_backup));
    common_backup.chnid = -1;

    const QList<resp_search_common_backup> &common_backup_list = commonBackupList(channel);
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

void BasePlayback::setCommonBackupLock(const resp_search_common_backup &backup, int isLock)
{
    if (s_channelInfoMap.contains(backup.chnid)) {
        PlaybackChannelInfo &channelInfo = s_channelInfoMap[backup.chnid];
        channelInfo.setCommonBackupLock(backup, isLock);

        s_playbackBar->updateTimeLine();
    }
}

int BasePlayback::currentSid()
{
    int sid = -1;
    int channel = currentChannel();
    QList<resp_search_common_backup> common_list = commonBackupList(channel);
    if (!common_list.isEmpty()) {
        sid = common_list.first().sid;
    }
    return sid;
}

void BasePlayback::addDrawTag(int channel, const QDateTime &dateTime)
{
    //TODO: LiuHuanyu 2021-08-05, 去掉毫秒影响，暂时先这样，后续统一用UTC秒
    uint time = dateTime.toTime_t();
    s_drawTagMap[channel].insert(QDateTime::fromTime_t(time), 0);
    s_playbackBar->updateTimeLine();
}

void BasePlayback::removeDrawTag(int channel, const QDateTime &dateTime)
{
    uint time = dateTime.toTime_t();
    s_drawTagMap[channel].remove(QDateTime::fromTime_t(time));
    s_playbackBar->updateTimeLine();
}

void BasePlayback::clearDrawTag()
{
    s_drawTagMap.clear();
}

QMap<QDateTime, int> BasePlayback::drawTagMap(int channel)
{
    QMap<QDateTime, int> map;
    if (s_drawTagMap.contains(channel)) {
        map = s_drawTagMap.value(channel);
    }
    return map;
}

void BasePlayback::setCurrentChannel(int channel)
{
    s_currentChannel = channel;
}

int BasePlayback::currentChannel()
{
    return s_currentChannel;
}

QList<int> BasePlayback::playingChannels()
{
    QList<int> list;
    for (auto iter = s_channelPlayStateMap.constBegin(); iter != s_channelPlayStateMap.constEnd(); ++iter) {
        auto channel = iter.key();
        auto state = iter.value();
        if (state == MsPlayingState) {
            list.append(channel);
        }
    }
    return list;
}

QList<int> BasePlayback::channelCheckedList()
{
    return s_checkedChannelList;
}

void BasePlayback::setChannelChecked(int channel, bool checked)
{
    s_currentChannel = channel;

    //
    if (checked) {
        s_checkedChannelList.append(channel);
    } else {
        s_checkedChannelList.removeAll(channel);
    }
}

void BasePlayback::clearChannelChecked()
{
    s_currentChannel = -1;
    s_checkedChannelList.clear();
}

bool BasePlayback::isChannelChecked(int channel)
{
    return s_checkedChannelList.contains(channel);
}

int BasePlayback::channelCheckedCount()
{
    return s_checkedChannelList.count();
}

MsPlaybackState BasePlayback::playbackState()
{
    return s_playbackState;
}

void BasePlayback::setPlaybackState(MsPlaybackState state)
{
    s_playbackState = state;
}

MsPlayState BasePlayback::channelPlayState(int channel)
{
    return s_channelPlayStateMap.value(channel, MsStopedState);
}

void BasePlayback::setChannelPlayState(int channel, MsPlayState state)
{
    s_channelPlayStateMap.insert(channel, state);
}

bool BasePlayback::isFisheyeDewarpEnable()
{
    return (s_fisheyeDewarpState == 1);
}

void BasePlayback::setFisheyeDewarpState(int state)
{
    s_fisheyeDewarpState = state;
}

void BasePlayback::setFilterEvent(int filetEvent)
{
    s_filterEvent = filetEvent;
}

void BasePlayback::setIsEventOnly(bool isEventOnly)
{
    s_isEventOnly = (playbackType() == GeneralPlayback) && isEventOnly;
}

int BasePlayback::getFilterEvent()
{
    return s_filterEvent;
}

bool BasePlayback::getIsEventOnly()
{
    return s_isEventOnly;
}

void BasePlayback::waitForSearchGeneralEventPlayBack()
{
    s_playbackBar->waitForSearchGeneralEventPlayBack();
}

int BasePlayback::getRECFilterEvent()
{
    switch (s_filterEvent) {
    case INFO_MAJOR_MOTION:
        return REC_EVENT_MOTION;
    case INFO_MAJOR_AUDIO_ALARM:
        return REC_EVENT_AUDIO_ALARM;
    case INFO_MAJOR_ALARMIN:
        return REC_EVENT_ALARMIN;
    case INFO_MAJOR_VCA:
        return REC_EVENT_VCA;
    case INFO_MAJOR_SMART:
        return REC_EVENT_SMART;
    default:
        return REC_EVENT_NONE;
    }
}

void BasePlayback::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN:
        ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(message);
        break;
    case RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE:
        ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE(message);
        break;
    case RESPONSE_FLAG_SEARCH_EVT_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(message);
        break;
    case RESPONSE_FLAG_GET_SEARCH_EVT_PAGE:
        ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(message);
        break;
    case RESPONSE_FLAG_START_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_START_ALL_PLAYBACK(message);
        break;
    case RESPONSE_FLAG_STOP_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(message);
        break;
    case RESPONSE_FLAG_RESTART_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_RESTART_ALL_PLAYBACK(message);
        break;
    }
}

void BasePlayback::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(MessageReceive *message)
{
    resp_search_common_backup *common_backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (!common_backup_array) {
        qWarning() << QString("BasePlayback::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN, resp_search_common_backup is null.");
    } else {
        if (common_backup_array->allCnt == 0) {
            qDebug() << QString("BasePlayback::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN, resp_search_common_backup is empty.");
        } else {
            const int &channel = common_backup_array->chnid;

            PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
            channelInfo.appendCommonBackup(common_backup_array, count);

            //
            //            for (int i = 0; i < count; ++i)
            //            {
            //                const resp_search_common_backup &common_backup = common_backup_array[i];
            //                if (common_backup.isLock)
            //                {
            //                    if (PlaybackFileManagement::instance())
            //                    {
            //                        PlaybackFileManagement::instance()->addLockFile(common_backup);
            //                    }
            //                }
            //            }

            //
            if (channelInfo.commonBackupSize() < common_backup_array->allCnt) {
                //继续搜索
                searchCommonPlaybackPage(channel);
                return;
            }
        }
    }
    exitWait();
}

void BasePlayback::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE(MessageReceive *message)
{
    resp_search_common_backup *common_backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (!common_backup_array) {
        qWarning() << QString("BasePlayback::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE, resp_search_common_backup is null.");
    } else {
        if (common_backup_array->allCnt == 0) {
            qDebug() << QString("BasePlayback::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE, resp_search_common_backup is empty.");
        } else {
            const int &channel = common_backup_array->chnid;

            PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
            channelInfo.appendCommonBackup(common_backup_array, count);

            //
            //            for (int i = 0; i < count; ++i)
            //            {
            //                const resp_search_common_backup &common_backup = common_backup_array[i];
            //                if (common_backup.isLock)
            //                {
            //                    if (PlaybackFileManagement::instance())
            //                    {
            //                        PlaybackFileManagement::instance()->addLockFile(common_backup);
            //                    }
            //                }
            //            }

            //
            if (channelInfo.commonBackupSize() < common_backup_array->allCnt) {
                //继续搜索
                searchCommonPlaybackPage(channel);
                return;
            }
        }
    }
    exitWait();
}

void BasePlayback::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message)
{
    struct resp_search_event_backup *event_backup_array = (struct resp_search_event_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_event_backup);

    if (!event_backup_array) {
        qWarning() << QString("BasePlayback::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP, resp_search_event_backup is null.");
    } else {
        const QDateTime &eventDataTime = QDateTime::fromString(event_backup_array->pStartTime, "yyyy-MM-dd HH:mm:ss");
        if (eventDataTime.date() != playbackDate()) {
            qDebug() << QString("BasePlayback::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP, close event backup, channel: %1, sid: %2")
                            .arg(event_backup_array->chnid)
                            .arg(event_backup_array->sid);
            MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_EVT_BACKUP_CLOSE, (void *)&(event_backup_array->sid), sizeof(int));
            return;
        }
        //
        if (event_backup_array->allCnt == 0) {
            qDebug() << QString("BasePlayback::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP, resp_search_event_backup is empty.");
        } else {
            const int &channel = event_backup_array->chnid;

            PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
            if (isSmartSearchMode()) {
                channelInfo.appendEventBackup_smartSearch(event_backup_array, count);
                if (channelInfo.eventBackupSize_smartSearch() < event_backup_array->allCnt) {
                    //继续搜索
                    searchEventPlaybackPage_smartSearch(channel);
                    return;
                }
            } else {
                channelInfo.appendEventBackup(event_backup_array, count);
                if (channelInfo.eventBackupSize() < event_backup_array->allCnt) {
                    //继续搜索
                    searchEventPlaybackPage(channel);
                    return;
                }
            }
        }
        //搜索完直接关闭，释放sid资源
        qDebug() << QString("BasePlayback::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP, close event backup, channel: %1, sid: %2")
                        .arg(event_backup_array->chnid)
                        .arg(event_backup_array->sid);
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_EVT_BACKUP_CLOSE, (void *)&(event_backup_array->sid), sizeof(int));
    }

    //搜索完毕，显示事件录像
    s_playbackBar->updateTimeLine();
    exitWait();
}

void BasePlayback::ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(MessageReceive *message)
{
    struct resp_search_event_backup *event_backup_array = (struct resp_search_event_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_event_backup);

    if (!event_backup_array) {
        qWarning() << QString("BasePlayback::ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE, resp_search_event_backup is null.");
    } else {
        const QDateTime &eventDataTime = QDateTime::fromString(event_backup_array->pStartTime, "yyyy-MM-dd HH:mm:ss");
        if (eventDataTime.date() != playbackDate()) {
            closeEventPlayback(event_backup_array->chnid);
            return;
        }
        //
        if (event_backup_array->allCnt == 0) {
            qDebug() << QString("BasePlayback::ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE, resp_search_event_backup is empty.");
        } else {
            const int &channel = event_backup_array->chnid;

            PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
            if (isSmartSearchMode()) {
                channelInfo.appendEventBackup_smartSearch(event_backup_array, count);
                if (channelInfo.eventBackupSize_smartSearch() < event_backup_array->allCnt) {
                    //继续搜索
                    searchEventPlaybackPage_smartSearch(channel);
                    return;
                }
            } else {
                channelInfo.appendEventBackup(event_backup_array, count);
                if (channelInfo.eventBackupSize() < event_backup_array->allCnt) {
                    //继续搜索
                    searchEventPlaybackPage(channel);
                    return;
                }
            }
        }
        //搜索完直接关闭，释放sid资源
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_EVT_BACKUP_CLOSE, (void *)&(event_backup_array->sid), sizeof(int));
    }

    //搜索完毕，显示事件录像
    s_playbackBar->updateTimeLine();
    exitWait();
}

void BasePlayback::ON_RESPONSE_FLAG_START_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message);

    exitWait();
}

void BasePlayback::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)
    qDebug() << "BasePlayback::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK";

    exitWait();
}

void BasePlayback::ON_RESPONSE_FLAG_RESTART_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)

    exitWait();
}

void BasePlayback::seekPlayback()
{
    req_seek_all_play_time seek_all_play_time;
    memset(&seek_all_play_time, 0, sizeof(req_seek_all_play_time));
    makeChannelMask(seek_all_play_time.chnMaskl, sizeof(seek_all_play_time.chnMaskl));
    snprintf(seek_all_play_time.pseektime, sizeof(seek_all_play_time.pseektime), "%s", QDateTime(playbackDate(), playbackTime()).toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    qMsDebug() << QString("REQUEST_FLAG_SEEK_ALL_PLAYBACK, time = %1").arg(seek_all_play_time.pseektime);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEEK_ALL_PLAYBACK, (void *)&seek_all_play_time, sizeof(req_seek_all_play_time));
}

void BasePlayback::seekPlayback(qint64 secs)
{
    QDateTime dateTime = QDateTime::fromTime_t(secs);

    req_seek_all_play_time seek_all_play_time;
    memset(&seek_all_play_time, 0, sizeof(req_seek_all_play_time));
    makeChannelMask(seek_all_play_time.chnMaskl, sizeof(seek_all_play_time.chnMaskl));
    snprintf(seek_all_play_time.pseektime, sizeof(seek_all_play_time.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    qMsDebug() << QString("REQUEST_FLAG_SEEK_ALL_PLAYBACK, time = %1").arg(seek_all_play_time.pseektime);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEEK_ALL_PLAYBACK, (void *)&seek_all_play_time, sizeof(req_seek_all_play_time));
}
