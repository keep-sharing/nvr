#include "SmartSearchControl.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackBar.h"
#include "PlaybackRealTimeThread.h"
#include "PlaybackWindow.h"
#include "SmartSearchDrawRegion.h"
#include "SmartSpeedPanel.h"
#include <QMouseEvent>
#include <QPainter>

SmartSearchControl::SmartSearchControl(QWidget *parent)
    : BasePlayback(parent)
{
    setCursor(QCursor(QPixmap(":/playback/playback/pen.png"), 0, 0));

    m_drawMotion = new SmartSearchDrawRegion(this);
    connect(m_drawMotion, SIGNAL(drawFinished()), this, SLOT(onDrawFinished()));
    m_drawMotion->setMode(DrawMotion::CoverMode);

    connect(PlaybackRealTimeThread::instance(), SIGNAL(playbackRealTime(QDateTime)), this, SLOT(onPlaybackRealTime(QDateTime)));

    if (!m_search) {
        m_search = new SearchEventBackup(this);
        connect(m_search, SIGNAL(finished(int)), this, SLOT(onSearchFinished(int)));
    }
}

SmartSearchControl *SmartSearchControl::instance()
{
    if (!s_smartSearch) {
        s_smartSearch = new SmartSearchControl(PlaybackWindow::instance());
        s_smartSearch->hide();
    }
    return s_smartSearch;
}

bool SmartSearchControl::isSmartSearchMode()
{
    if (!isVisible()) {
        return false;
    }
    return true;
}

bool SmartSearchControl::isSmartSearchEnable()
{
    if (!isSmartSearchMode()) {
        return false;
    }
    if (!m_isSearchFinished) {
        return false;
    }
    return true;
}

void SmartSearchControl::showSmartSearch()
{
    qMsDebug();
    m_drawMotion->clearAll();
    show();
    raise();
    //
    setMode(true);
}

void SmartSearchControl::closeSmartSearch()
{
    qMsDebug();

    m_drawMotion->clearAll();
    m_isSearchFinished = false;
    m_search->clearSearch();
    close();
    //
    if (s_smartSpeed->isTemporarilyDisableForSmartSearch()) {
        s_smartSpeed->resumeSmartSpeed();
    }
    //
    setMode(false);
}

void SmartSearchControl::manualSeek(const QDateTime &dateTime)
{
    qMsCDebug("qt_smart_search") << dateTime.toString("yyyy-MM-dd HH:mm:ss");
    m_state = StateNone;
    clearCurrentMotionTime();
    clearNextMotionTime();
    if (m_fakerPause) {
        if (playbackState() != PlaybackState_Pause) {
            restartAllPlayback();
        }
        m_fakerPause = false;
    }
    //
    m_isSeeked = true;
}

void SmartSearchControl::research()
{
    QMetaObject::invokeMethod(this, "onDrawFinished", Qt::QueuedConnection);
}

void SmartSearchControl::changePlayDirection()
{
    qMsCDebug("qt_smart_search") << playbackDirection();
    m_state = StateNone;
    clearCurrentMotionTime();
    clearNextMotionTime();
}

void SmartSearchControl::stepForward()
{
    if (m_fakerPause) {
        m_fakerPause = false;
    }
}

void SmartSearchControl::stepBackward()
{
    if (m_fakerPause) {
        m_fakerPause = false;
    }
}

bool SmartSearchControl::hasRecord()
{
    return m_search->hasBackup();
}

void SmartSearchControl::resizeEvent(QResizeEvent *event)
{
    m_drawMotion->setGeometry(rect());
    BasePlayback::resizeEvent(event);
}

void SmartSearchControl::paintEvent(QPaintEvent *event)
{
#if 0
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::red);
    painter.drawRect(rect());
#endif
    BasePlayback::paintEvent(event);
}

void SmartSearchControl::getNextMotionTime(qint64 current, int type, qint64 &begin, qint64 &end)
{
    begin = -1;
    end = -1;

    if (current < 0) {
        return;
    }

    if (m_backupMap.isEmpty()) {
        return;
    }

    qint64 sec_begin = m_backupMap.begin().key();
    qint64 sec_end = (--m_backupMap.end()).key();

    if (current > sec_begin) {
        sec_begin = current;
    }

    for (int i = sec_begin; i <= sec_end + 1; ++i) {
        int value = m_backupMap.value(i, 0);
        if (value & type) {
            if (begin < 0) {
                begin = i;
            }
        } else {
            if (begin > 0) {
                end = i - 1;
                break;
            }
        }
    }
}

void SmartSearchControl::getPreviousMotionTime(qint64 current, int type, qint64 &begin, qint64 &end)
{
    begin = -1;
    end = -1;

    if (m_backupMap.isEmpty()) {
        return;
    }

    qint64 dayEventBeginSec = m_backupMap.begin().key();
    //qint64 dayEventEndSec = (--m_backupMap.end()).key();

    for (int sec = current; sec >= dayEventBeginSec; sec--) {
        int value = m_backupMap.value(sec, 0);
        if (value & type) {
            if (end < 0) {
                end = sec;
            }
        } else {
            if (end > 0) {
                begin = sec + 1;
                break;
            }
        }
    }
}

void SmartSearchControl::clearCurrentMotionTime()
{
    m_currentMotionStartTime = -1;
    m_currentMotionEndTime = -1;
}

void SmartSearchControl::clearNextMotionTime()
{
    m_nextMotionStartTime = -1;
    m_nextMotionEndTime = -1;
}

void SmartSearchControl::onPlaybackRealTime(QDateTime dateTime)
{
    if (playbackType() != GeneralPlayback) {
        return;
    }

    if (!isSmartSearchEnable()) {
        return;
    }

    if (s_playbackBar->isSeeking()) {
        return;
    }

    if (dateTime.date() != playbackDate()) {
        return;
    }

    if (m_fakerPause) {
        return;
    }

    if (!hasRecord()) {
        return;
    }

    //seek后忽略一次
    if (m_isSeeked) {
        m_isSeeked = false;
        return;
    }

    qint64 sec = dateTime.toTime_t();
    //
    if (playbackDirection() == PB_FORWARD) {
        switch (m_state) {
        case StateNone:
            getNextMotionTime(sec, REC_EVENT_MOTION, m_currentMotionStartTime, m_currentMotionEndTime);
            //找到下一段motion
            if (m_currentMotionStartTime > 0) {
                if (sec >= m_currentMotionStartTime && sec <= m_currentMotionEndTime) {
                    m_state = StateMotion;
                } else {
                    seekPlayback(m_currentMotionStartTime);
                    m_state = StateMotion;
                    //继续找下一段
                    getNextMotionTime(m_currentMotionEndTime + 1, REC_EVENT_MOTION, m_nextMotionStartTime, m_nextMotionEndTime);
                }
            } else {
                //没有找到motion，暂停在最后一个motion的最后一秒
                pauseAllPlayback();
                if (!m_backupMap.isEmpty()) {
                    auto iter = m_backupMap.constEnd();
                    iter--;
                    qint64 secs = iter.key();
                    seekPlayback(secs);
                }
                m_fakerPause = true;
            }
            break;
        case StateMotion:
            if (sec > m_currentMotionEndTime) {
                if (m_nextMotionStartTime < 0) {
                    //查找下一段
                    getNextMotionTime(m_currentMotionEndTime + 1, REC_EVENT_MOTION, m_nextMotionStartTime, m_nextMotionEndTime);
                    if (m_nextMotionStartTime < 0) {
                        //没有motion了，暂停在最后一个motion的最后一秒
                        pauseAllPlayback();
                        if (!m_backupMap.isEmpty()) {
                            auto iter = m_backupMap.constEnd();
                            iter--;
                            qint64 secs = iter.key();
                            seekPlayback(secs);
                        }
                        m_fakerPause = true;
                    }
                } else {
                    m_currentMotionStartTime = m_nextMotionStartTime;
                    m_currentMotionEndTime = m_nextMotionEndTime;
                    seekPlayback(m_currentMotionStartTime);
                    //继续找下一段
                    getNextMotionTime(m_currentMotionEndTime + 1, REC_EVENT_MOTION, m_nextMotionStartTime, m_nextMotionEndTime);
                }
            }
            break;
        }
    } else if (playbackDirection() == PB_BACKWARD) {
        switch (m_state) {
        case StateNone:
            getPreviousMotionTime(sec, REC_EVENT_MOTION, m_currentMotionStartTime, m_currentMotionEndTime);
            //找到前一段motion
            if (m_currentMotionStartTime > 0) {
                if (sec >= m_currentMotionStartTime && sec <= m_currentMotionEndTime) {
                    m_state = StateMotion;
                } else {
                    seekPlayback(m_currentMotionEndTime);
                    m_state = StateMotion;
                    //继续找下一段
                    getPreviousMotionTime(m_currentMotionStartTime - 1, REC_EVENT_MOTION, m_nextMotionStartTime, m_nextMotionEndTime);
                }
            } else {
                //没有找到motion，暂停第一个motion的第一秒
                pauseAllPlayback();
                if (!m_backupMap.isEmpty()) {
                    auto iter = m_backupMap.constBegin();
                    qint64 secs = iter.key();
                    seekPlayback(secs);
                }
                m_fakerPause = true;
            }
            break;
        case StateMotion:
            if (sec < m_currentMotionStartTime) {
                if (m_nextMotionStartTime < 0) {
                    //查找下一段
                    getPreviousMotionTime(m_currentMotionStartTime - 1, REC_EVENT_MOTION, m_nextMotionStartTime, m_nextMotionEndTime);
                    if (m_nextMotionStartTime < 0) {
                        //没有motion了，暂停在第一个motion的第一秒
                        pauseAllPlayback();
                        if (!m_backupMap.isEmpty()) {
                            auto iter = m_backupMap.constBegin();
                            qint64 secs = iter.key();
                            seekPlayback(secs);
                        }
                        m_fakerPause = true;
                    }
                } else {
                    m_currentMotionStartTime = m_nextMotionStartTime;
                    m_currentMotionEndTime = m_nextMotionEndTime;
                    seekPlayback(m_currentMotionEndTime);
                    //继续找下一段
                    getPreviousMotionTime(m_currentMotionStartTime - 1, REC_EVENT_MOTION, m_nextMotionStartTime, m_nextMotionEndTime);
                }
            }
            break;
        }
    }
}

void SmartSearchControl::onDrawFinished()
{
    if (!isSmartSearchMode()) {
        return;
    }
    m_isSearchFinished = false;
    m_state = StateNone;
    clearCurrentMotionTime();
    clearNextMotionTime();

    memset(m_region, 0, sizeof(char) * 300);
    m_drawMotion->getRegion(m_region);
    if (!QString(m_region).contains("1")) {
        //closeWait();
        return;
    }

    QString strRegion;
    strRegion.append("\n");
    for (int row = 0; row < MOTION_H_CELL; ++row) {
        for (int column = 0; column < MOTION_W_CELL; ++column) {
            int index = row * MOTION_W_CELL + column;
            strRegion.append(QString(m_region[index]));
            strRegion.append(" ");
        }
        strRegion.append("\n");
    }
    qMsDebug() << strRegion;

    //showWait();
    //
    int channel = currentChannel();
    struct req_search_event_backup event_backup;
    memset(&event_backup, 0, sizeof(req_search_event_backup));
    makeChannelMask(channel, event_backup.chnMaskl, sizeof(event_backup.chnMaskl));
    event_backup.chnNum = 1;
    event_backup.enType = playbackStream();
    event_backup.enMajor = INFO_MAJOR_MOTION;
    event_backup.ahead = 0;
    event_backup.delay = 0;
    snprintf(event_backup.pStartTime, sizeof(event_backup.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    QDateTime searchEndDateTime = commonBackupEndDateTime(channel);
    if (searchEndDateTime.isValid()) {
        snprintf(event_backup.pEndTime, sizeof(event_backup.pEndTime), "%s", searchEndDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    } else {
        snprintf(event_backup.pEndTime, sizeof(event_backup.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    }
    memcpy(event_backup.pMotMap, m_region, sizeof(event_backup.pMotMap));
    //
    event_backup.all = MF_YES;
    event_backup.close = MF_YES;
    //
    m_search->setSearchInfo(event_backup);
    m_search->startSearch();
    //m_eventLoop.exec();
    //closeWait();
    //
    const QList<resp_search_event_backup> &backupList = m_search->backupList();
    if (!backupList.isEmpty()) {
        m_backupMap.clear();
        for (int i = 0; i < backupList.size(); ++i) {
            const resp_search_event_backup &backup = backupList.at(i);
            qint64 startDateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss").toTime_t();
            qint64 endDateTime = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss").toTime_t();
            for (qint64 sec = startDateTime; sec <= endDateTime; ++sec) {
                int value = m_backupMap.value(i);
                value |= REC_EVENT_MOTION;
                m_backupMap.insert(sec, value);
            }
        }
    } else {
        ShowMessageBox(GET_TEXT("PLAYBACK/80134", "No matching videos."));
    }
    updateTimeLine();

    //
    m_isSearchFinished = true;
    //
    if (m_fakerPause) {
        if (playbackState() != PlaybackState_Pause) {
            restartAllPlayback();
        }
        m_fakerPause = false;
    }
}

void SmartSearchControl::onSearchFinished(int channel)
{
    if (channel == currentChannel()) {
        //m_eventLoop.exit();
    }
}

int SmartSearchControl::mode() const
{
    return m_mode;
}

void SmartSearchControl::setMode(int newMode)
{
    if (m_mode == newMode) {
        return;
    }
    m_mode = newMode;
    emit modeChanged(m_mode);
}

QList<resp_search_event_backup> SmartSearchControl::backupList() const
{
    return m_search->backupList();
}
