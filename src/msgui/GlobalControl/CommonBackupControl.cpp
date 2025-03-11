#include "CommonBackupControl.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include "myqt.h"

CommonBackupControl::CommonBackupControl(QObject *parent)
    : MsObject(parent)
{
}

int CommonBackupControl::preSec() const
{
    return m_preSec;
}

void CommonBackupControl::setPreSec(int newPreSec)
{
    m_preSec = newPreSec;
}

int CommonBackupControl::postSec() const
{
    return m_postSec;
}

void CommonBackupControl::setPostSec(int newPostSec)
{
    m_postSec = newPostSec;
}

int CommonBackupControl::streamType() const
{
    return m_streamType;
}

int CommonBackupControl::fileSize() const
{
    int size = 0;
    if (!m_playBackupList.isEmpty()) {
        size = m_playBackupList.first().allSize;
    }
    return size;
}

QPixmap CommonBackupControl::pixmap() const
{
    return m_pixmap;
}

QImage CommonBackupControl::image() const
{
    return m_pixmap.toImage();
}

bool CommonBackupControl::hasBackup() const
{
    return !m_playBackupList.isEmpty();
}

bool CommonBackupControl::isPlaying() const
{
    return m_isPlaying;
}

const QDateTime &CommonBackupControl::realEndDateTime() const
{
    return m_realEndDateTime;
}

const QDateTime &CommonBackupControl::realStartDateTime() const
{
    return m_realStartDateTime;
}

int CommonBackupControl::waitForSearchCommonBackup(int channel, const QDateTime &begin, const QDateTime &end, int streamType)
{
    waitForCloseCommonBackup();

    m_playBackupList.clear();

    m_channel = channel;
    QDateTime realBegin = begin.addSecs(-m_preSec);
    QDateTime realEnd = end.addSecs(m_postSec);

    req_search_common_backup commonBackup;
    memset(&commonBackup, 0, sizeof(req_search_common_backup));
    MyQt::makeChannelMask(channel, commonBackup.chnMaskl, sizeof(commonBackup.chnMaskl));
    commonBackup.chnNum  = 1;
    if (streamType == -1) {
        commonBackup.enType  = FILE_TYPE_MAIN;
    } else {
        commonBackup.enType  = streamType;
    }
    commonBackup.enEvent = REC_EVENT_ALL;
    commonBackup.enState = SEG_STATE_ALL;
    snprintf(commonBackup.pStartTime, sizeof(commonBackup.pStartTime), "%s", realBegin.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(commonBackup.pEndTime, sizeof(commonBackup.pEndTime), "%s", realEnd.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP, &commonBackup, sizeof(commonBackup));
    //int result = m_eventLoop.exec();

#if 1
    if (streamType == -1 && m_playBackupList.isEmpty()) {
        commonBackup.enType = FILE_TYPE_SUB;
        sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP, &commonBackup, sizeof(commonBackup));
        //result = m_eventLoop.exec();
    }
#endif

    m_streamType = commonBackup.enType;

    return 1;
}

int CommonBackupControl::waitForPlayCommonBackup()
{
    struct rep_play_common_backup playinfo;
    memset(&playinfo, 0, sizeof(struct rep_play_common_backup));
    playinfo.chnid = m_channel;
    playinfo.sid   = m_sid;
    snprintf(playinfo.pStartTime, sizeof(playinfo.pStartTime), "%s", m_realStartDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(playinfo.pEndTime, sizeof(playinfo.pEndTime), "%s", m_realEndDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(playinfo.pPlayTime, sizeof(playinfo.pPlayTime), "%s", m_realStartDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    qMsDebug() << QString("REQUEST_FLAG_PLAY_COM_BACKUP, chnid: %1, sid: %2, pStartTime: %3, pEndTime: %4, pPlayTime: %5")
                      .arg(playinfo.chnid)
                      .arg(playinfo.sid)
                      .arg(playinfo.pStartTime)
                      .arg(playinfo.pEndTime)
                      .arg(playinfo.pPlayTime);
    sendMessage(REQUEST_FLAG_PLAY_COM_BACKUP, (void *)&playinfo, sizeof(struct rep_play_common_backup));
    //int result = m_eventLoop.exec();
    m_isPlaying = true;

    return 1;
}

int CommonBackupControl::waitForPauseCommonBackup()
{
    if (m_sid < 0) {
        return -1;
    }
    sendMessage(REQUEST_FLAG_PLAYPAUSE_COM_BACKUP, &m_sid, sizeof(int));
    //int result = m_eventLoop.exec();

    return 1;
}

int CommonBackupControl::waitForResumeCommonBackup()
{
    if (m_sid < 0) {
        return -1;
    }
    sendMessage(REQUEST_FLAG_PLAYRESTART_COM_BACKUP, &m_sid, sizeof(int));
    //int result = m_eventLoop.exec();

    return 1;
}

int CommonBackupControl::waitForStopCommonBackup()
{
    if (!m_isPlaying) {
        return -1;
    }

    sendMessage(REQUEST_FLAG_PLAYSTOP_COM_BACKUP, &m_sid, sizeof(int));
    //int result = m_eventLoop.exec();
    m_isPlaying = false;

    return 1;
}

int CommonBackupControl::waitForCloseCommonBackup()
{
    if (m_sid < 0) {
        return -1;
    }

    sendMessage(REQUEST_FLAG_SEARCH_COM_BACKUP_CLOSE, &m_sid, sizeof(int));
    //int result = m_eventLoop.exec();
    m_sid = -1;

    return 1;
}

int CommonBackupControl::waitForSeekCommonBackup(const QDateTime &dateTime)
{
    struct req_seek_time seekinfo;
    memset(&seekinfo, 0, sizeof(struct req_seek_time));
    seekinfo.chnid = m_channel;
    seekinfo.sid = m_sid;
    snprintf(seekinfo.pseektime, sizeof(seekinfo.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_PLAYSEEK_BACKUP, &seekinfo, sizeof(seekinfo));
    //int result = m_eventLoop.exec();

    return 1;
}

int CommonBackupControl::waitForSearchCommonBackupPicture()
{
    if (m_playBackupList.isEmpty()) {
        return -1;
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
    //int result = m_eventLoop.exec();

    return 1;
}

QDateTime CommonBackupControl::getPlaybackRealTime()
{
    return QDateTime(QDate(1970, 1, 1), QTime(0, 0));
}

void CommonBackupControl::processMessage(MessageReceive *message)
{
    switch (message->type()) {
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
    }
}

void CommonBackupControl::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message)
{
    resp_search_common_backup *backupArray = (resp_search_common_backup *)message->data;

    int result = 0;
    do {
        if (!backupArray) {
            result = -1;
            break;
        }
        if (backupArray->allCnt < 1) {
            result = -2;
            break;
        }

        int count = message->size() / sizeof(struct resp_search_common_backup);
        m_sid     = backupArray->sid;
        for (int i = 0; i < count; ++i) {
            const resp_search_common_backup &backup = backupArray[i];
            const QDateTime &startDateTime          = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
            const QDateTime &endDateTime            = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
            if (i == 0) {
                m_realStartDateTime = startDateTime;
            }
            if (i == count - 1) {
                m_realEndDateTime = endDateTime;
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
    } while (0);

    m_eventLoop.exit(result);
}

void CommonBackupControl::ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message)
{
    int result = (*((int *)message->data));
    m_eventLoop.exit(result);
}

void CommonBackupControl::ON_RESPONSE_FLAG_PLAYPAUSE_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit(0);
}

void CommonBackupControl::ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit(0);
}

void CommonBackupControl::ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit(0);
}

void CommonBackupControl::ON_RESPONSE_FLAG_SEARCH_COM_BACKUP_CLOSE(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit(0);
}

void CommonBackupControl::ON_RESPONSE_FLAG_PLAYSEEK_BACKUP(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit(0);
}

void CommonBackupControl::ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message)
{
    int result = 0;

    do {
        if (m_isPlaying) {
            result = -1;
            break;
        }

        m_pixmap = QPixmap::fromImage(message->image1);
    } while (0);

    m_eventLoop.exit(result);
}
