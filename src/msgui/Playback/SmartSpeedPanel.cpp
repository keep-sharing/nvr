#include "SmartSpeedPanel.h"
#include "ui_SmartSpeedPanel.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackBar.h"
#include "PlaybackEventData.h"
#include "PlaybackMode.h"
#include "PlaybackRealTimeThread.h"
#include "SmartSearchControl.h"
#include "centralmessage.h"

//#define SMART_SPEED_DEBUG

SmartSpeedPanel::SmartSpeedPanel(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::SmartSpeedPanel)
{
    ui->setupUi(this);
    s_smartSpeed = this;

    ui->comboBox_smartPlaySpeed->clear();
    ui->comboBox_smartPlaySpeed->addItem(GET_TEXT("COMMON/1013", "Off"), false);
    ui->comboBox_smartPlaySpeed->addItem(GET_TEXT("COMMON/1012", "On"), true);

    ui->comboBox_skipGeneralVideo->clear();
    ui->comboBox_skipGeneralVideo->addItem(GET_TEXT("COMMON/1018", "Disable"), false);
    ui->comboBox_skipGeneralVideo->addItem(GET_TEXT("COMMON/1009", "Enable"), true);

#ifdef SMART_SPEED_DEBUG
    m_debug = new SmartSpeedDebug(this);
    connect(m_debug, SIGNAL(refresh()), this, SLOT(onRefreshDebugInfo()));
    m_debug->hide();
#else
    ui->pushButton_debug->hide();
#endif

    if (s_playbackMode) {
        connect(s_playbackMode, SIGNAL(playbackModeChanged(MsPlaybackType)), this, SLOT(onPlaybackModeChanged(MsPlaybackType)));
    } else {
        qWarning() << "SmartSpeedPanel, s_playbackMode is nullptr.";
    }

    connect(PlaybackRealTimeThread::instance(), SIGNAL(playbackRealTime(QDateTime)), this, SLOT(onPlaybackRealTime(QDateTime)));

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    qputenv("qt_smart_speed", "1");
}

SmartSpeedPanel::~SmartSpeedPanel()
{
    s_smartSpeed = nullptr;
    delete ui;
}

void SmartSpeedPanel::initializeData()
{
    ui->comboBox_smartPlaySpeed->setCurrentIndexFromData(m_smartPlaySpeedEnable);
    ui->comboBox_skipGeneralVideo->setCurrentIndexFromData(m_skipGeneralVideo);
    ui->horizontalSlider_speedForGeneral->setSpeedValue(m_speedForGeneral);
    ui->horizontalSlider_speedForEvent->setSpeedValue(m_speedForEvent);
}

void SmartSpeedPanel::closeSmartSpeed()
{
    m_smartPlaySpeedEnable = false;
    m_skipGeneralVideo = true;
    m_speedForGeneral = PLAY_SPEED_1X;
    m_speedForEvent = PLAY_SPEED_1X;

    ui->comboBox_smartPlaySpeed->setCurrentIndexFromData(m_smartPlaySpeedEnable);
    ui->comboBox_skipGeneralVideo->setCurrentIndexFromData(m_skipGeneralVideo);
    ui->horizontalSlider_speedForGeneral->setSpeedValue(m_speedForGeneral);
    ui->horizontalSlider_speedForEvent->setSpeedValue(m_speedForEvent);

    m_state = StateNone;
    clearTimeInfo();
    closePanel();
    emit smartSpeedStateChanged(m_smartPlaySpeedEnable);
}

void SmartSpeedPanel::clearTimeInfo()
{
    m_generalSecMap.clear();
    m_eventSecMap.clear();
}

void SmartSpeedPanel::closePanel()
{
    close();
}

void SmartSpeedPanel::temporarilyDisableForSmartSearch()
{
    if (!isSmartPlaySpeedEnable()) {
        return;
    }
    if (m_isTemporarilyDisabled) {
        return;
    }

    //
    m_isTemporarilyDisabled = true;
    //
    closePanel();
    m_smartPlaySpeedEnable = false;
    setPlaybackSpeed(PLAY_SPEED_1X);
    s_playbackBar->updatePlaybackSpeedString();
    m_state = StateNone;
    emit smartSpeedStateChanged(m_smartPlaySpeedEnable);
}

bool SmartSpeedPanel::isTemporarilyDisableForSmartSearch() const
{
    return m_isTemporarilyDisabled;
}

void SmartSpeedPanel::resumeSmartSpeed()
{
    m_isTemporarilyDisabled = false;
    //
    m_smartPlaySpeedEnable = true;
    m_state = StateNone;
    emit smartSpeedStateChanged(m_smartPlaySpeedEnable);
}

bool SmartSpeedPanel::isSmartPlaySpeedEnable() const
{
    return m_smartPlaySpeedEnable;
}

bool SmartSpeedPanel::isSkipGeneralVideo() const
{
    return m_skipGeneralVideo;
}

PLAY_SPEED SmartSpeedPanel::speedForGeneral() const
{
    return m_speedForGeneral;
}

PLAY_SPEED SmartSpeedPanel::speedForEvent() const
{
    return m_speedForEvent;
}

void SmartSpeedPanel::manualSeek(const QDateTime &dateTime)
{
    qMsCDebug("qt_smart_speed") << dateTime.toString("yyyy-MM-dd HH:mm:ss");
    m_state = StateNone;
    clearEventTime();
    clearGeneralTime();
    if (m_fakerPause) {
        if (playbackState() != PlaybackState_Pause) {
            restartAllPlayback();
        }
        m_fakerPause = false;
    }
    //
    m_isSeeked = true;
}

void SmartSpeedPanel::updateEventRecord()
{
    if (!m_smartPlaySpeedEnable) {
        return;
    }
    QList<int> channels = playingChannels();
    if (channels.size() != 1) {
        qMsWarning() << "playingChannels:" << channels;
        return;
    }

    const QList<resp_search_event_backup> &backupList = gPlaybackEventData.backupList(channels.first(), static_cast<uint>(getFilterEvent()));
    for (int i = 0; i < backupList.size(); ++i) {
        const resp_search_event_backup &backup = backupList.at(i);
        insertEventTime(backup);
    }
}

void SmartSpeedPanel::changePlayDirection()
{
    qMsCDebug("qt_smart_speed") << playbackDirection();
    m_state = StateNone;
    clearEventTime();
    clearGeneralTime();
}

void SmartSpeedPanel::stepForward()
{
    if (m_fakerPause) {
        m_fakerPause = false;
    }
}

void SmartSpeedPanel::stepBackward()
{
    if (m_fakerPause) {
        m_fakerPause = false;
    }
}

void SmartSpeedPanel::startEventPlayBack()
{
    m_smartPlaySpeedEnable = true;
    m_skipGeneralVideo = true;

    m_fakerPause = false;

    clearTimeInfo();
    m_currentEventEndSec = playbackDateTime().toTime_t();
    //
    if (gPlaybackEventData.backupList(currentChannel(), static_cast<uint>(getFilterEvent())).isEmpty()) {
        //同步web，保持当前播放时间停止通常录像播放
        stopAllPlayback();
        s_playbackBar->setPlaybackButtonState(PlaybackState_None);
    }
    //
    int channel = currentChannel();
    const QList<resp_search_event_backup> &eventlist = eventBackupList(channel);
    for (int i = 0; i < eventlist.size(); ++i) {
        const resp_search_event_backup &backup = eventlist[i];
        insertEventTime(backup);
    }
    //
    updateEventRecord();
}

void SmartSpeedPanel::closeEventPlayBack()
{
    if (playbackState() == PlaybackState_None && isChannelHasCommonBackup(currentChannel())) {
        s_playbackBar->setPlaybackButtonState(PlaybackState_Stop);
    }
    m_smartPlaySpeedEnable = false;
}

void SmartSpeedPanel::dealMessage(MessageReceive *message)
{
    if (playbackType() != GeneralPlayback) {
        return;
    }

    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN:
        ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(message);
        break;
    case RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE:
        ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE(message);
        break;
    case RESPONSE_FLAG_STOP_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(message);
        break;
    }
}

void SmartSpeedPanel::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(MessageReceive *message)
{
    resp_search_common_backup *common_backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    dealCommonTimeInfo(common_backup_array, count);
}

void SmartSpeedPanel::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE(MessageReceive *message)
{
    resp_search_common_backup *common_backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    dealCommonTimeInfo(common_backup_array, count);
}

void SmartSpeedPanel::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)

    m_generalSecMap.clear();
    m_eventSecMap.clear();
    m_state = StateNone;
}

void SmartSpeedPanel::dealCommonTimeInfo(resp_search_common_backup *common_array, int count)
{
    if (!m_smartPlaySpeedEnable) {
        return;
    }

    if (common_array) {
        for (int i = 0; i < count; ++i) {
            const resp_search_common_backup &backup = common_array[i];
            if (backup.enEvent == REC_EVENT_TIME) {
                insertGeneralTime(backup);
            } else {
                insertEventTime(backup);
            }
        }
    }
}

void SmartSpeedPanel::dealEventTimeInfo(resp_search_event_backup *event_array, int count)
{
    if (!m_smartPlaySpeedEnable) {
        return;
    }

    if (event_array) {
        for (int i = 0; i < count; ++i) {
            const resp_search_event_backup &backup = event_array[i];
            insertEventTime(backup);
        }
    }
}

void SmartSpeedPanel::insertGeneralTime(const resp_search_common_backup &backup)
{
    qint64 startSec = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss").toTime_t();
    qint64 endSec = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss").toTime_t();
    for (qint64 sec = startSec; sec <= endSec; ++sec) {
        int value = m_generalSecMap.value(sec);
        value |= backup.enEvent;
        m_generalSecMap.insert(sec, value);
    }
}

void SmartSpeedPanel::insertEventTime(const resp_search_common_backup &backup)
{
    qint64 startSec = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss").toTime_t();
    qint64 endSec = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss").toTime_t();
    for (qint64 sec = startSec; sec <= endSec; ++sec) {
        int value = m_eventSecMap.value(sec);
        value |= backup.enEvent;
        m_eventSecMap.insert(sec, value);
    }
}

void SmartSpeedPanel::insertEventTime(const resp_search_event_backup &backup)
{
    qint64 startSec = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss").toTime_t();
    qint64 endSec = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss").toTime_t();
    for (qint64 sec = startSec; sec <= endSec; ++sec) {
        int value = m_eventSecMap.value(sec);
        //NOTE 底层返回的事件类型有问题
        value |= backup.enEvent;
        m_eventSecMap.insert(sec, value);
    }
}

void SmartSpeedPanel::getNextGeneralTime(qint64 current, int type, qint64 &begin, qint64 &end)
{
    Q_UNUSED(type)

    begin = -1;
    end = -1;

    if (m_generalSecMap.isEmpty()) {
        return;
    }

    qint64 sec_begin = m_generalSecMap.begin().key();
    qint64 sec_end = (--m_generalSecMap.end()).key();

    if (current > sec_begin) {
        sec_begin = current;
    }

    for (int i = sec_begin; i <= sec_end + 1; ++i) {
        int value = m_generalSecMap.value(i, -1);
        if (value != -1) {
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

void SmartSpeedPanel::getPreviousGeneralTime(qint64 current, int type, qint64 &begin, qint64 &end)
{
    Q_UNUSED(type)

    begin = -1;
    end = -1;

    if (m_generalSecMap.isEmpty()) {
        return;
    }

    qint64 dayEventBeginSec = m_generalSecMap.begin().key();
    qint64 dayEventEndSec = (--m_generalSecMap.end()).key();

    if (current > dayEventEndSec) {
        current = dayEventEndSec;
    }

    for (int sec = current; sec >= dayEventBeginSec - 1; sec--) {
        int value = m_generalSecMap.value(sec, -1);
        if (value != -1) {
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

void SmartSpeedPanel::getNextEventTime(qint64 current, int type, qint64 &begin, qint64 &end)
{
    Q_UNUSED(type)

    begin = -1;
    end = -1;

    if (current < 0) {
        return;
    }

    if (m_eventSecMap.isEmpty()) {
        return;
    }

    qint64 sec_begin = m_eventSecMap.begin().key();
    qint64 sec_end = (--m_eventSecMap.end()).key();

    if (current > sec_begin) {
        sec_begin = current;
    }

    for (qint64 i = sec_begin; i <= sec_end + 1; ++i) {
        int value = m_eventSecMap.value(i, -1);
        if (value != -1) {
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

void SmartSpeedPanel::getPreviousEventTime(qint64 current, int type, qint64 &begin, qint64 &end)
{
    Q_UNUSED(type)

    begin = -1;
    end = -1;

    if (m_eventSecMap.isEmpty()) {
        return;
    }

    qint64 dayEventBeginSec = m_eventSecMap.begin().key();
    qint64 dayEventEndSec = (--m_eventSecMap.end()).key();

    if (current > dayEventEndSec) {
        current = dayEventEndSec;
    }

    for (qint64 sec = current; sec >= dayEventBeginSec - 1; sec--) {
        int value = m_eventSecMap.value(sec, -1);
        if (value != -1) {
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

void SmartSpeedPanel::enterEventSpeed()
{
    if (getIsEventOnly()) {
        setPlaybackSpeed(playbackSpeed());
    } else {
        setPlaybackSpeed(m_speedForEvent);
    }

#ifdef SMART_SPEED_DEBUG
    m_debug->setSpeedText(playbackSpeedString(m_speedForEvent));
#endif
    s_playbackBar->updatePlaybackSpeedString();
}

void SmartSpeedPanel::enterGeneralSpeed()
{
    setPlaybackSpeed(m_speedForGeneral);
#ifdef SMART_SPEED_DEBUG
    m_debug->setSpeedText(playbackSpeedString(m_speedForGeneral));
#endif
    s_playbackBar->updatePlaybackSpeedString();
}

void SmartSpeedPanel::fakerPauseGeneralLastSec()
{
    pauseAllPlayback();
    if (!m_generalSecMap.isEmpty()) {
        auto iter = m_generalSecMap.constEnd();
        iter--;
        qint64 secs = iter.key();
        seekPlayback(secs);
    }
    m_fakerPause = true;
}

void SmartSpeedPanel::fakerPauseGeneralFirstSec()
{
    pauseAllPlayback();
    if (!m_generalSecMap.isEmpty()) {
        auto iter = m_generalSecMap.constBegin();
        qint64 secs = iter.key();
        seekPlayback(secs);
    }
    m_fakerPause = true;
}

void SmartSpeedPanel::fakerPauseEventLastSec()
{
    pauseAllPlayback();
    if (!m_eventSecMap.isEmpty()) {
        auto iter = m_eventSecMap.constEnd();
        iter--;
        qint64 secs = iter.key();
        seekPlayback(secs);
    } else if (!m_generalSecMap.isEmpty()) {
        auto iter = m_generalSecMap.constEnd();
        iter--;
        qint64 secs = iter.key();
        seekPlayback(secs);
    }
    m_fakerPause = true;
}

void SmartSpeedPanel::fakerPauseEventFirstSec()
{
    pauseAllPlayback();
    if (!m_eventSecMap.isEmpty()) {
        auto iter = m_eventSecMap.constBegin();
        qint64 secs = iter.key();
        seekPlayback(secs);
    }
    m_fakerPause = true;
}

void SmartSpeedPanel::clearEventTime()
{
    m_currentEventStartSec = -1;
    m_currentEventEndSec = -1;
    m_nextEventStartSec = -1;
    m_nextEventEndSec = -1;
}

void SmartSpeedPanel::clearGeneralTime()
{
    m_currentGeneralStartSec = -1;
    m_currentGeneralEndSec = -1;
    m_nextGeneralStartSec = -1;
    m_nextGeneralEndSec = -1;
}

QString SmartSpeedPanel::timeString(qint64 sec1, qint64 sec2)
{
    return QString("%1 - %2, %3 - %4")
        .arg(sec1)
        .arg(sec2)
        .arg(QDateTime::fromTime_t(sec1).toString("yyyy-MM-dd HH:mm:ss"))
        .arg(QDateTime::fromTime_t(sec2).toString("yyyy-MM-dd HH:mm:ss"));
}

void SmartSpeedPanel::onSmartSpeedPanelButtonClicked(int x, int y)
{
    if (!isVisible()) {
        initializeData();
        show();
        raise();
        QPoint p;
        p.setX(x - width() / 2);
        p.setY(y - height());
        move(p);
    } else {
        hide();
    }
}

void SmartSpeedPanel::onLanguageChanged()
{
    ui->label_smartPlaySpeed->setText(GET_TEXT("PLAYBACK/80125", "Smart Play Speed"));
    ui->comboBox_smartPlaySpeed->setItemText(0, GET_TEXT("COMMON/1013", "Off"));
    ui->comboBox_smartPlaySpeed->setItemText(1, GET_TEXT("COMMON/1012", "On"));
    ui->label_skipGeneralVideo->setText(GET_TEXT("PLAYBACK/80126", "Skip General Video"));
    ui->comboBox_skipGeneralVideo->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_skipGeneralVideo->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));
    ui->label_speedForGeneral->setText(GET_TEXT("PLAYBACK/80127", "Play Speed for General Video"));
    ui->label_speedForEvent->setText(GET_TEXT("PLAYBACK/80128", "Play Speed for Event Video"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void SmartSpeedPanel::onRefreshDebugInfo()
{
#ifdef SMART_SPEED_DEBUG
    int channel = currentChannel();
    const QList<resp_search_common_backup> &commonList = commonBackupList(channel);
    const QList<resp_search_event_backup> &eventList = eventBackupList(channel);
    m_debug->clearInfo();
    m_debug->setGeneralInfo(commonList);
    m_debug->setEventInfo(eventList);
    m_debug->setSmartRangeInfo(m_eventTimeMap);
#endif
}

void SmartSpeedPanel::onPlaybackModeChanged(MsPlaybackType mode)
{
    if (mode != GeneralPlayback) {
        closePanel();
    }
}

void SmartSpeedPanel::onPlaybackRealTime(QDateTime dateTime)
{
    if (playbackType() != GeneralPlayback) {
        return;
    }

    if (!isSmartPlaySpeedEnable()) {
        return;
    }

    if (gPlaybackEventData.isSearching()) {
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

    //seek后忽略一次
    if (m_isSeeked) {
        m_isSeeked = false;
        return;
    }
    qint64 sec = dateTime.toTime_t();

#ifdef SMART_SPEED_DEBUG
    m_debug->setRealTimeText(dateTime.toString("yyyy-MM-dd HH:mm:ss"));
#endif
    //
    if (playbackDirection() == PB_FORWARD) {
        switch (m_state) {
        case StateNone:
            getNextEventTime(sec, getRECFilterEvent(), m_currentEventStartSec, m_currentEventEndSec);
            getNextGeneralTime(sec, REC_EVENT_TIME, m_currentGeneralStartSec, m_currentGeneralEndSec);
            if (isSkipGeneralVideo()) {
                if (m_currentEventStartSec > 0) {
                    if (sec >= m_currentEventStartSec && sec <= m_currentEventEndSec) {
                        enterEventSpeed();
                        m_state = StateEvent;
                    } else {
                        enterEventSpeed();
                        seekPlayback(m_currentEventStartSec);
                        m_state = StateEvent;
                    }
                } else {
                    //后面没有event录像
                    fakerPauseEventLastSec();
                }
            } else {
                if (sec >= m_currentEventStartSec && sec <= m_currentEventEndSec) {
                    enterEventSpeed();
                    m_state = StateEvent;
                } else if (sec >= m_currentGeneralStartSec && sec <= m_currentGeneralEndSec) {
                    enterGeneralSpeed();
                    m_state = StateGeneral;
                } else {
                    //后面没有任何录像
                    fakerPauseGeneralLastSec();
                }
            }
            break;
        case StateGeneral:
            if (m_currentEventStartSec > 0 && sec > m_currentEventStartSec) {
                enterEventSpeed();
                seekPlayback(m_currentEventStartSec);
                m_state = StateEvent;
            }
            break;
        case StateEvent:
            if (sec > m_currentEventEndSec) {
                if (isSkipGeneralVideo()) {
                    getNextEventTime(m_currentEventEndSec + 1, getRECFilterEvent(), m_currentEventStartSec, m_currentEventEndSec);
                    qMsCDebug("qt_smart_speed") << timeString(m_currentEventStartSec, m_currentEventEndSec);
                    if (m_currentEventStartSec > 0) {
                        seekPlayback(m_currentEventStartSec);
                    } else {
                        //后面没有event录像
                        fakerPauseEventLastSec();
                    }
                } else {
                    int search_begin = m_currentEventEndSec + 1;
                    getNextEventTime(search_begin, getRECFilterEvent(), m_currentEventStartSec, m_currentEventEndSec);
                    qMsCDebug("qt_smart_speed") << timeString(m_currentEventStartSec, m_currentEventEndSec);
                    getNextGeneralTime(search_begin, REC_EVENT_TIME, m_currentGeneralStartSec, m_currentGeneralEndSec);
                    qMsCDebug("qt_smart_speed") << timeString(m_currentGeneralStartSec, m_currentGeneralEndSec);
                    qint64 next_sec;
                    if (m_currentGeneralStartSec == -1) {
                        next_sec = m_currentEventStartSec;
                    } else if (m_currentEventStartSec == -1) {
                        next_sec = m_currentGeneralStartSec;
                    } else {
                        next_sec = qMin(m_currentEventStartSec, m_currentGeneralStartSec);
                    }

                    if (next_sec != -1 && next_sec == m_currentEventStartSec) {
                        seekPlayback(m_currentEventStartSec);
                    } else if (next_sec != -1 && next_sec == m_currentGeneralStartSec) {
                        enterGeneralSpeed();
                        seekPlayback(m_currentGeneralStartSec);
                        m_state = StateGeneral;
                    } else {
                        //后面没有任何录像
                        fakerPauseGeneralLastSec();
                    }
                }
            }
            break;
        }
    } else {
        switch (m_state) {
        case StateNone:
            getPreviousEventTime(sec, getRECFilterEvent(), m_currentEventStartSec, m_currentEventEndSec);
            qMsCDebug("qt_smart_speed") << timeString(m_currentEventStartSec, m_currentEventEndSec);
            getPreviousGeneralTime(sec, REC_EVENT_TIME, m_currentGeneralStartSec, m_currentGeneralEndSec);
            qMsCDebug("qt_smart_speed") << timeString(m_currentGeneralStartSec, m_currentGeneralEndSec);
            if (isSkipGeneralVideo()) {
                if (m_currentEventStartSec > 0) {
                    if (sec >= m_currentEventStartSec && sec <= m_currentEventEndSec) {
                        enterEventSpeed();
                        m_state = StateEvent;
                    } else {
                        enterEventSpeed();
                        seekPlayback(m_currentEventEndSec);
                        m_state = StateEvent;
                    }
                } else {
                    //后面没有event录像
                    fakerPauseEventFirstSec();
                }
            } else {
                if (sec >= m_currentEventStartSec && sec <= m_currentEventEndSec) {
                    enterEventSpeed();
                    m_state = StateEvent;
                } else if (sec >= m_currentGeneralStartSec && sec <= m_currentGeneralEndSec) {
                    enterGeneralSpeed();
                    m_state = StateGeneral;
                } else {
                    //后面没有任何录像
                    fakerPauseGeneralFirstSec();
                }
            }
            break;
        case StateGeneral:
            if (m_currentEventStartSec > 0 && sec < m_currentEventEndSec) {
                enterEventSpeed();
                seekPlayback(m_currentEventEndSec);
                m_state = StateEvent;
            }
            break;
        case StateEvent:
            if (sec < m_currentEventStartSec || sec > m_currentEventEndSec) {
                if (isSkipGeneralVideo()) {
                    if (sec < m_currentEventStartSec) {
                        getPreviousEventTime(m_currentEventStartSec - 1, getRECFilterEvent(), m_currentEventStartSec, m_currentEventEndSec);
                        qMsCDebug("qt_smart_speed") << timeString(m_currentEventStartSec, m_currentEventEndSec);
                    }
                    if (m_currentEventStartSec > 0) {
                        seekPlayback(m_currentEventEndSec);
                    } else {
                        //后面没有event录像
                        fakerPauseEventFirstSec();
                    }
                } else {
                    int search_begin = m_currentEventStartSec - 1;
                    getPreviousEventTime(search_begin, getRECFilterEvent(), m_currentEventStartSec, m_currentEventEndSec);
                    qMsCDebug("qt_smart_speed") << timeString(m_currentEventStartSec, m_currentEventEndSec);
                    getPreviousGeneralTime(search_begin, REC_EVENT_TIME, m_currentGeneralStartSec, m_currentGeneralEndSec);
                    qMsCDebug("qt_smart_speed") << timeString(m_currentGeneralStartSec, m_currentGeneralEndSec);
                    qint64 next_sec = qMax(m_currentEventEndSec, m_currentGeneralEndSec);
                    if (next_sec != -1 && next_sec == m_currentEventEndSec) {
                        seekPlayback(m_currentEventEndSec);
                    } else if (next_sec != -1 && next_sec == m_currentGeneralEndSec) {
                        enterGeneralSpeed();
                        seekPlayback(m_currentGeneralEndSec);
                        m_state = StateGeneral;
                    } else {
                        //后面没有任何录像
                        fakerPauseGeneralFirstSec();
                    }
                }
            }
            break;
        }
    }
}

void SmartSpeedPanel::on_comboBox_smartPlaySpeed_currentIndexChanged(int index)
{
    if (index == 0) {
        ui->comboBox_skipGeneralVideo->setEnabled(false);
        ui->horizontalSlider_speedForGeneral->setEnabled(false);
        ui->horizontalSlider_speedForEvent->setEnabled(false);
    } else {
        ui->comboBox_skipGeneralVideo->setEnabled(true);
        ui->horizontalSlider_speedForGeneral->setEnabled(true);
        ui->horizontalSlider_speedForEvent->setEnabled(true);

        on_comboBox_skipGeneralVideo_currentIndexChanged(ui->comboBox_skipGeneralVideo->currentIndex());
    }
}

void SmartSpeedPanel::on_comboBox_skipGeneralVideo_currentIndexChanged(int index)
{
    if (index == 0) {
        if (ui->comboBox_smartPlaySpeed->currentData().toBool()) {
            ui->horizontalSlider_speedForGeneral->setEnabled(true);
        }
    } else {
        ui->horizontalSlider_speedForGeneral->setEnabled(false);
    }
}

void SmartSpeedPanel::on_pushButton_apply_clicked()
{
    const QList<int> &channelList = channelCheckedList();
    if (channelList.size() > 1) {
        ui->pushButton_apply->clearUnderMouse();
        ShowMessageBox(GET_TEXT("FISHEYE/12006", "Only available when playback with single channel."));
        return;
    }

    m_smartPlaySpeedEnable = ui->comboBox_smartPlaySpeed->currentData().toBool();
    m_skipGeneralVideo = ui->comboBox_skipGeneralVideo->currentData().toBool();
    m_speedForGeneral = ui->horizontalSlider_speedForGeneral->speedValue();
    m_speedForEvent = ui->horizontalSlider_speedForEvent->speedValue();

    m_fakerPause = false;

    if (isSmartPlaySpeedEnable()) {
        clearTimeInfo();
        //
        int channel = currentChannel();
        const QList<resp_search_common_backup> &commonList = commonBackupList(channel);
        for (int i = 0; i < commonList.size(); ++i) {
            const resp_search_common_backup &backup = commonList[i];
            if (backup.enEvent == REC_EVENT_TIME) {
                insertGeneralTime(backup);
            } else {
                insertEventTime(backup);
            }
        }
        //
        updateEventRecord();
    } else {
        setPlaybackSpeed(PLAY_SPEED_1X);
        s_playbackBar->updatePlaybackSpeedString();
    }

    m_state = StateNone;
    emit smartSpeedStateChanged(m_smartPlaySpeedEnable);
}

void SmartSpeedPanel::on_pushButton_cancel_clicked()
{
    hide();
}

void SmartSpeedPanel::on_pushButton_debug_clicked()
{
    m_debug->show();
}
