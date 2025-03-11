#include "PlaybackBar.h"
#include "ui_PlaybackBar.h"
#include "CustomTag.h"
#include "FilterEventPanel.h"
#include "LiveView.h"
#include "LiveViewSub.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "PlaybackData.h"
#include "PlaybackEventData.h"
#include "PlaybackLayout.h"
#include "PlaybackList.h"
#include "PlaybackRealTimeThread.h"
#include "PlaybackSplit.h"
#include "PlaybackTagData.h"
#include "PlaybackVideoBar.h"
#include "PlaybackWindow.h"
#include "SmartSearchControl.h"
#include "SmartSpeedPanel.h"
#include "SubControl.h"
#include "centralmessage.h"
#include "msuser.h"

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "FisheyeDewarpControl.h"
#endif

extern "C" {

}

PlaybackBar::PlaybackBar(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackBar)
{
    ui->setupUi(this);

    BasePlayback::s_playbackBar = this;

    connect(ui->timeLine, SIGNAL(timeClicked(QDateTime)), this, SLOT(onTimeLineClicked(QDateTime)));

    //
    m_playbackTimer = new QTimer(this);
    connect(m_playbackTimer, SIGNAL(timeout()), this, SLOT(onPlaybackTimer()));
    m_playbackTimer->setInterval(1000);

    //
    m_rockerTimer = new QTimer(this);
    connect(m_rockerTimer, SIGNAL(timeout()), this, SLOT(onRockerTimer()));
    m_rockerTimer->setInterval(1000);

    //
    m_animateToast = new AnimateToast(this);
    m_animateToast->setDestWidget(ui->toolButton_fileManagement);

    //
    m_fileManagement = new PlaybackFileManagement(this);

    //split playback
    QButtonGroup *splitGroup = new QButtonGroup(this);
    splitGroup->addButton(ui->toolButton_split_4, SplitLayout_4);
    splitGroup->addButton(ui->toolButton_split_9, SplitLayout_9);
    splitGroup->addButton(ui->toolButton_split_16, SplitLayout_16);
    connect(splitGroup, SIGNAL(buttonClicked(int)), this, SLOT(onSplitButtonGroupClicked(int)));
    ui->widget_split->hide();

    //
    ui->toolButton_bestDecoding->setVisible(qMsNvr->isSupportBestDecoding());

    //
    ui->toolButtonLockAll->setVisible(!qMsNvr->isSlaveMode());
    ui->toolButtonQuickTagAll->setVisible(!qMsNvr->isSlaveMode());
    ui->toolButtonCustomTagAll->setVisible(!qMsNvr->isSlaveMode());
    if (qMsNvr->isSlaveMode()) {
        gPlaybackVideoBar->setSnapshotButtonEnabled(false);
        gPlaybackVideoBar->setLockButtonEnabled(false);
        gPlaybackVideoBar->setQuickTagButtonEnable(false);
        gPlaybackVideoBar->setCustomTagButtonEnable(false);
    }
    int channelCount = qMsNvr->maxChannel();
    if (channelCount < 16) {
        ui->toolButton_split_16->hide();
    }
    if (channelCount < 9) {
        ui->toolButton_split_9->hide();
    }

    //
    connect(gPlaybackVideoBar, SIGNAL(smartSearchClicked()), this, SLOT(onSmartSearchClicked()));
    connect(gPlaybackVideoBar, SIGNAL(fisheyeClicked()), this, SLOT(onFisheyeClicked()));
    connect(gPlaybackVideoBar, SIGNAL(zoomClicked()), this, SLOT(onZoomClicked()));
    connect(gPlaybackVideoBar, SIGNAL(snapshotClicked()), this, SLOT(onSnapshotClicked()));
    connect(gPlaybackVideoBar, SIGNAL(audioClicked(bool)), this, SLOT(onAudioClicked(bool)));
    connect(gPlaybackVideoBar, SIGNAL(lockClicked()), this, SLOT(onLockClicked()));
    connect(gPlaybackVideoBar, SIGNAL(quickTagClicked()), this, SLOT(onQuickTagClicked()));
    connect(gPlaybackVideoBar, SIGNAL(customTagClicked()), this, SLOT(onCustomTagClicked()));

    //
    connect(PlaybackRealTimeThread::instance(), SIGNAL(playbackRealTime(QDateTime)), this, SLOT(onPlaybackRealTime(QDateTime)));
    connect(PlaybackRealTimeThread::instance(), SIGNAL(splitPlaybackRealTime(int, QMap<int, QDateTime>)), this, SLOT(onSplitPlaybackRealTime(int, QMap<int, QDateTime>)));

    //
    connect(&gPlaybackData, SIGNAL(playbackSpeedChanged(int)), this, SLOT(onPlaybackSpeedChanged(int)));
    connect(SmartSearchControl::instance(), SIGNAL(modeChanged(int)), this, SLOT(onSmartSearchModeChanged(int)));

    //
    connect(&gPlaybackEventData, SIGNAL(searchFinished(int)), this, SLOT(onEventSearchFinished(int)));

    //
    connect(&gPlaybackTagData, SIGNAL(tagSearchFinished(int)), this, SLOT(onTagSearchFinished(int)));
    connect(&gPlaybackTagData, SIGNAL(tagChanged(int)), this, SLOT(onTagChanged(int)));

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    m_eventSearchType = EventNone;

    //
#ifdef MS_FISHEYE_SOFT_DEWARP
#else
    gPlaybackVideoBar->setFisheyeButtonVisible(false);
#endif
}

PlaybackBar::~PlaybackBar()
{
    delete ui;
}

PlaybackTimeLine *PlaybackBar::timeline()
{
    return ui->timeLine;
}

void PlaybackBar::initializeData()
{
    ui->toolButton_smartSpeed->setChecked(false);
    ui->toolButton_bestDecoding->setChecked(false);
    ui->toolButtonFilterEvent->setChecked(false);
    if (SubControl::instance()->isSubEnable()) {
        ui->toolButton_bestDecoding->setEnabled(true);
    } else {
        ui->toolButton_bestDecoding->setEnabled(false);
    }
    ui->toolButtonPos->setChecked(true);
    on_toolButtonPos_clicked(true);
    //
    ui->toolButton_split_4->setChecked(true);
    PlaybackSplit *playbackSplit = PlaybackSplit::instance();
    if (playbackSplit) {
        playbackSplit->setLayoutMode(SplitLayout_4);
    }
    //
    initializeTimeLine();
}

void PlaybackBar::initializeTimeLine()
{
    setPlaybackButtonState(PlaybackState_None);
    ui->timeLine->setZoomLevel(TimeLineZoomLevel_1);
}

void PlaybackBar::setCurrentTimeLine(int channel)
{
    QMetaObject::invokeMethod(this, "onSetCurrentTimeLine", Qt::QueuedConnection, Q_ARG(int, channel));
}

void PlaybackBar::setCurrentTimeLine()
{
    const int &channel = currentChannel();
    QMetaObject::invokeMethod(this, "onSetCurrentTimeLine", Qt::QueuedConnection, Q_ARG(int, channel));
}

void PlaybackBar::setCurrentDateTime(const QDateTime &dateTime)
{
    setDateTimeText(dateTime);
    ui->timeLine->setCurrentTime(dateTime);
}

void PlaybackBar::updatePlaybackButtonState()
{
    switch (playbackState()) {
    case PlaybackState_None: {
        setPlaybackSpeed(PLAY_SPEED_1X);
        ui->label_speed->setText(playbackSpeedString());
        setPlaybackDirection(PB_FORWARD);
        m_playbackTimer->stop();
        //
        setPlaybackTime(QTime(0, 0));
        const QDateTime &dateTime = playbackDateTime();
        setCurrentDateTime(dateTime);
        //
        if (isFisheyeDewarpEnable()) {
            PlaybackWindow::instance()->closeFisheyePanel();
        }
        setSnapshotButtonEnable(false);
        gPlaybackVideoBar->setAudioButtonEnabled(false);
        gPlaybackVideoBar->setZoomButtonEnable(false);
        gPlaybackVideoBar->setNVRDewarpingEnable(false);

        ui->toolButton_speedDown->setEnabled(false);
        ui->toolButton_speedUp->setEnabled(false);
        ui->toolButton_stepReverse->setEnabled(false);
        ui->toolButton_stepForward->setEnabled(false);
        ui->toolButton_rewind->setChecked(false);
        ui->toolButton_rewind->setEnabled(false);
        ui->toolButton_stop->setEnabled(false);
        ui->toolButton_play->setChecked(false);
        ui->toolButton_play->setEnabled(false);
        if (playbackSpeed() == PLAY_SPEED_1X) {
            ui->toolButtonPos->setEnabled(true);
        }

        ui->toolButton_cut->setEnabled(false);
        setLockButtonEnable(false);
        setQuickTagButtonEnable(false);
        setCustomTagButtonEnable(false);
        break;
    }
    case PlaybackState_Stop: {
        setPlaybackSpeed(PLAY_SPEED_1X);
        ui->label_speed->setText(playbackSpeedString());
        setPlaybackDirection(PB_FORWARD);
        m_playbackTimer->stop();
        //
        if (isFisheyeDewarpEnable()) {
            PlaybackWindow::instance()->closeFisheyePanel();
        }
        setSnapshotButtonEnable(false);
        gPlaybackVideoBar->setAudioButtonEnabled(false);
        gPlaybackVideoBar->setZoomButtonEnable(false);
        gPlaybackVideoBar->setNVRDewarpingEnable(false);

        ui->toolButton_speedDown->setEnabled(false);
        ui->toolButton_speedUp->setEnabled(false);
        ui->toolButton_stepReverse->setEnabled(false);
        ui->toolButton_stepForward->setEnabled(false);
        ui->toolButton_rewind->setChecked(false);
        ui->toolButton_rewind->setEnabled(false);
        ui->toolButton_stop->setEnabled(false);
        ui->toolButton_play->setChecked(false);
        ui->toolButton_play->setEnabled(true);
        if (playbackSpeed() == PLAY_SPEED_1X) {
            ui->toolButtonPos->setEnabled(true);
        }

        ui->toolButton_cut->setEnabled(false);
        setLockButtonEnable(false);
        setQuickTagButtonEnable(false);
        setCustomTagButtonEnable(false);
        break;
    }
    case PlaybackState_Play: {
        m_playbackTimer->start();

        setSnapshotButtonEnable(true);
        setPlaybackDirection(PB_FORWARD);
        if (playbackSpeed() == PLAY_SPEED_1X) {
            gPlaybackVideoBar->setAudioButtonEnabled(true);
        }
        gPlaybackVideoBar->setZoomButtonEnable(true);
        gPlaybackVideoBar->setNVRDewarpingEnable(true);
        if (s_smartSpeed->isSmartPlaySpeedEnable() && getFilterEvent() == INFO_MAJOR_NONE) {
            ui->toolButton_speedDown->setEnabled(false);
            ui->toolButton_speedUp->setEnabled(false);
        } else {
            ui->toolButton_speedDown->setEnabled(true);
            ui->toolButton_speedUp->setEnabled(true);
        }
        ui->toolButton_stepReverse->setEnabled(false);
        ui->toolButton_stepForward->setEnabled(false);
        ui->toolButton_rewind->setChecked(false);
        ui->toolButton_rewind->setEnabled(true);
        ui->toolButton_stop->setEnabled(true);
        ui->toolButton_play->setChecked(true);
        ui->toolButton_play->setEnabled(true);
        if (playbackSpeed() == PLAY_SPEED_1X) {
            ui->toolButtonPos->setEnabled(true);
        }

        ui->toolButton_cut->setEnabled(true);
        setLockButtonEnable(true);
        setQuickTagButtonEnable(true);
        setCustomTagButtonEnable(true);
        break;
    }
    case PlaybackState_Pause: {
        //m_playbackTimer->stop();

        PlaybackWindow::instance()->closeFisheyePanel();
        setSnapshotButtonEnable(true);
        gPlaybackVideoBar->setAudioButtonEnabled(false);
        gPlaybackVideoBar->setZoomButtonEnable(false);
        gPlaybackVideoBar->setNVRDewarpingEnable(false);
        if (s_smartSpeed->isSmartPlaySpeedEnable() && getFilterEvent() == INFO_MAJOR_NONE) {
            ui->toolButton_speedDown->setEnabled(false);
            ui->toolButton_speedUp->setEnabled(false);
        } else {
            ui->toolButton_speedDown->setEnabled(true);
            ui->toolButton_speedUp->setEnabled(true);
        }
        ui->toolButton_stepReverse->setEnabled(true);
        ui->toolButton_stepForward->setEnabled(true);
        ui->toolButton_rewind->setChecked(false);
        ui->toolButton_rewind->setEnabled(true);
        ui->toolButton_stop->setEnabled(true);
        ui->toolButton_play->setChecked(false);
        ui->toolButton_play->setEnabled(true);
        if (playbackSpeed() == PLAY_SPEED_1X) {
            ui->toolButtonPos->setEnabled(true);
        }

        ui->toolButton_cut->setEnabled(true);
        setLockButtonEnable(true);
        setQuickTagButtonEnable(true);
        setCustomTagButtonEnable(true);
        break;
    }
    case PlaybackState_Reverse: {
        //m_playbackTimer->start();

        setSnapshotButtonEnable(true);
        setPlaybackDirection(PB_BACKWARD);
        gPlaybackVideoBar->setAudioButtonEnabled(false);
        gPlaybackVideoBar->setZoomButtonEnable(true);
        gPlaybackVideoBar->setNVRDewarpingEnable(false);

        if (s_smartSpeed->isSmartPlaySpeedEnable() && getFilterEvent() == INFO_MAJOR_NONE) {
            ui->toolButton_speedDown->setEnabled(false);
            ui->toolButton_speedUp->setEnabled(false);
        } else {
            ui->toolButton_speedDown->setEnabled(true);
            ui->toolButton_speedUp->setEnabled(true);
        }
        ui->toolButton_stepReverse->setEnabled(false);
        ui->toolButton_stepForward->setEnabled(false);
        ui->toolButton_rewind->setChecked(true);
        ui->toolButton_rewind->setEnabled(true);
        ui->toolButton_stop->setEnabled(true);
        ui->toolButton_play->setChecked(false);
        ui->toolButton_play->setEnabled(true);
        ui->toolButtonPos->setEnabled(false);

        ui->toolButton_cut->setEnabled(true);
        setLockButtonEnable(true);
        setQuickTagButtonEnable(true);
        setCustomTagButtonEnable(true);
        break;
    }
    default:
        break;
    }

    updateSmartSearchButtonState();
    updateFisheyeButtonState();
    updateZoomButtonState();

    //
    if (ui->toolButton_rewind->isChecked()) {
        ui->toolButton_rewind->setToolTip(GET_TEXT("PLAYBACK/80003", "Pause"));
    } else {
        ui->toolButton_rewind->setToolTip(GET_TEXT("PLAYBACK/80012", "Rewind"));
    }
    ui->toolButton_stop->setToolTip(GET_TEXT("PTZCONFIG/36038", "Stop"));
    if (ui->toolButton_play->isChecked()) {
        ui->toolButton_play->setToolTip(GET_TEXT("PLAYBACK/80003", "Pause"));
    } else {
        ui->toolButton_play->setToolTip(GET_TEXT("PTZCONFIG/36022", "Play"));
    }
}

void PlaybackBar::setPlaybackButtonState(MsPlaybackState state)
{
    qDebug() << QString("PlaybackBar::setPlaybackButtonState, state: %1").arg(playbackStateString(state));
    setPlaybackState(state);
    updatePlaybackButtonState();
}

void PlaybackBar::updateTimeLine()
{
    ui->timeLine->updateTimeLine();
}

void PlaybackBar::updateSplitChannel()
{
    PlaybackSplit *split = PlaybackSplit::instance();
    int channel = split->channel();
    int sid = split->selectedSid();
    ui->label_channel->setText(QString("%1-%2").arg(channel + 1).arg(sid + 1));

    if (split->hasCommonBackup()) {
        const QDateTime &selectedDateTime = m_splitDateTimeMap.value(sid);
        setPlaybackTime(selectedDateTime.time());
        setDateTimeText(selectedDateTime);
        ui->timeLine->setCurrentTime(selectedDateTime);
    }

    //audio button
    if (PlaybackSplit::instance()->isSelectedAudioOpen()) {
        gPlaybackVideoBar->setAudioButtonChecked(true);
        gPlaybackVideoBar->setAudioButtonToolTip(GET_TEXT("LIVEVIEW/20023", "Audio off"));
    } else {
        gPlaybackVideoBar->setAudioButtonChecked(false);
        gPlaybackVideoBar->setAudioButtonToolTip(GET_TEXT("LIVEVIEW/20022", "Audio on"));
    }

#ifdef MS_FISHEYE_SOFT_DEWARP
    s_fisheyeControl->closeFisheyePanel();
    gPlaybackData.setFisheyeMode(false);
#endif
    updateFisheyeButtonState();
    updateZoomButtonState();
}

void PlaybackBar::updatePlaybackSpeedString()
{
    ui->label_speed->setText(playbackSpeedString());
}

void PlaybackBar::closePlaybackCut()
{
    if (m_cut) {
        m_cut->closeCut();
    }
}

void PlaybackBar::switchScreen()
{
    closePlaybackCut();
    ui->toolButton_bestDecoding->setChecked(false);
}

bool PlaybackBar::isBestDecodeMode() const
{
    return ui->toolButton_bestDecoding->isChecked();
}

bool PlaybackBar::isSeeking() const
{
    return m_isSeeking;
}

bool PlaybackBar::isRealtimeActive()
{
    return m_playbackTimer->isActive();
}

bool PlaybackBar::isShowPosData() const
{
    return ui->toolButtonPos->isChecked();
}

QMap<int, QDateTime> PlaybackBar::splitDateTimeMap()
{
    return m_splitDateTimeMap;
}

QDateTime PlaybackBar::splitDateTime()
{
    int sid = PlaybackSplit::instance()->selectedSid();
    if (!m_splitDateTimeMap.contains(sid)) {
        qMsWarning() << "sid:" << sid << m_splitDateTimeMap;
    }
    return m_splitDateTimeMap.value(sid);
}

void PlaybackBar::clearSplitDateTimeMap()
{
    m_splitDateTimeMap.clear();
}

void PlaybackBar::waitForSearchGeneralEventPlayBack()
{
    if (playbackType() == GeneralPlayback && getFilterEvent() != INFO_MAJOR_NONE) {
        if (!gDebug.checkCategorySet("pb_search_event", "off")) {
            struct req_search_event_backup event_backup;
            memset(&event_backup, 0, sizeof(req_search_event_backup));
            makeChannelMask(currentChannel(), event_backup.chnMaskl, sizeof(event_backup.chnMaskl));
            event_backup.chnNum = 1;
            event_backup.enType = playbackStream();
            event_backup.enMajor = static_cast<uint>(getFilterEvent());
            if (getFilterEvent() == INFO_MAJOR_SMART) {
                event_backup.enMinor = INFO_MINOR_ALL;
            }
            if (getFilterEvent() == INFO_MAJOR_VCA) {
                event_backup.enMinor = INFO_MINOR_ALL_0;
            }
            event_backup.ahead = 0;
            event_backup.delay = 0;
            snprintf(event_backup.pStartTime, sizeof(event_backup.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
            snprintf(event_backup.pEndTime, sizeof(event_backup.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
            event_backup.all = MF_YES;
            event_backup.close = MF_YES;
            if (gPlaybackEventData.hasEventBackup(currentChannel(), static_cast<uint>(getFilterEvent()))) {
                updateTimeLine();
            } else {
                //showWait();
                PlaybackEventData::State result = gPlaybackEventData.searchEventBackup(event_backup);
                if (result != PlaybackEventData::StateStart) {
                    //closeWait();
                } else {
                    execWait();
                }
            }
        }
        //
        if (!gDebug.checkCategorySet("pb_search_tag", "off")) {
            searchTagPlayback(currentChannel());
        }
    } else if (playbackType() == GeneralPlayback) {
        updateTimeLine();
    }
}

NetworkResult PlaybackBar::dealNetworkCommond(const QString &commond)
{
    qDebug() << "PlaybackBar::dealNetworkCommond begin," << commond;

    NetworkResult result = NetworkReject;
    if (commond.startsWith("Video_Play")) {
        if (ui->toolButton_play->isEnabled() && ui->toolButton_play->isVisible()) {
            if (!ui->toolButton_play->isChecked()) {
                ui->toolButton_play->setChecked(true);
                on_toolButton_play_clicked(true);
                result = NetworkAccept;
            }
        }
    } else if (commond.startsWith("Video_Pause")) {
        if (ui->toolButton_play->isEnabled() && ui->toolButton_play->isVisible()) {
            if (ui->toolButton_play->isChecked()) {
                ui->toolButton_play->setChecked(false);
                on_toolButton_play_clicked(false);
                result = NetworkAccept;
            }
        }
    } else if (commond.startsWith("Video_Stop")) {
        if (ui->toolButton_stop->isEnabled() && ui->toolButton_stop->isVisible()) {
            on_toolButton_stop_clicked();
            result = NetworkAccept;
        }
    } else if (commond.startsWith("Dial_Out_Add")) {
        if (ui->toolButton_speedUp->isEnabled() && ui->toolButton_speedUp->isVisible()) {
            on_toolButton_speedUp_clicked();
            result = NetworkAccept;
        }
    } else if (commond.startsWith("Dial_Out_Sub")) {
        if (ui->toolButton_speedDown->isEnabled() && ui->toolButton_speedDown->isVisible()) {
            on_toolButton_speedDown_clicked();
            result = NetworkAccept;
        }
    } else if (commond.startsWith("Video_Forward")) {
        m_rockerMode = 0;
        m_rockerTimer->start();
        onRockerTimer();
        result = NetworkAccept;
    } else if (commond.startsWith("Video_Backward")) {
        m_rockerMode = 1;
        m_rockerTimer->start();
        onRockerTimer();
        result = NetworkAccept;
    } else if (commond.startsWith("Video_Here")) {
        m_rockerTimer->stop();
        result = NetworkAccept;
    }
    qDebug() << "PlaybackBar::dealNetworkCommond end," << result;
    return result;
}

void PlaybackBar::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    switch (message->type()) {
    case RESPONSE_FLAG_START_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_START_ALL_PLAYBACK(message);
        break;
    case RESPONSE_FLAG_STOP_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(message);
        break;
    case RESPONSE_FLAG_GET_PLAYBACK_REALTIME:
        ON_RESPONSE_FLAG_GET_PLAYBACK_REALTIME(message);
        break;
    case RESPONSE_FLAG_GET_SPLIT_PLAYBACK_TIME:
        ON_RESPONSE_FLAG_GET_SPLIT_PLAYBACK_TIME(message);
        break;
    case RESPONSE_FLAG_SEEK_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_SEEK_ALL_PLAYBACK(message);
        break;
    case RESPONSE_FLAG_SEEK_SPLIT_PLAYBACK:
        ON_RESPONSE_FLAG_SEEK_SPLIT_PLAYBACK(message);
        break;
    case RESPONSE_FLAG_CHECK_AUDIOTALK:
        ON_RESPONSE_FLAG_CHECK_AUDIOTALK(message);
        message->accept();
        break;
    }
}

void PlaybackBar::ON_RESPONSE_FLAG_START_ALL_PLAYBACK(MessageReceive *message)
{
    qDebug() << "PlaybackBar::ON_RESPONSE_FLAG_START_ALL_PLAYBACK";

    req_pb_state *state_array = static_cast<req_pb_state *>(message->data);
    int size = message->header.size / sizeof(req_pb_state);
    for (int i = 0; i < size; ++i) {
        const req_pb_state &state = state_array[i];
        if (state.state != -1) {
            setChannelPlayState(state.chnid, MsPlayingState);
        }
    }

    setPlaybackButtonState(PlaybackState_Play);
    setCurrentTimeLine();
}

void PlaybackBar::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message)
{
    qDebug() << "PlaybackBar::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK";

    req_pb_state *state_array = static_cast<req_pb_state *>(message->data);
    int size = message->header.size / sizeof(req_pb_state);
    for (int i = 0; i < size; ++i) {
        const req_pb_state &state = state_array[i];
        if (state.state != -1) {
            setChannelPlayState(state.chnid, MsStopedState);
        }
    }

    //
    if (channelCheckedList().isEmpty()) {
        if (s_smartSearch->isSmartSearchMode()) {
            s_smartSearch->closeSmartSearch();
        }
        setPlaybackButtonState(PlaybackState_None);
    } else {
        bool hasRecord = false;
        QList<int> channels = channelCheckedList();
        for (int i = 0; i < channels.size(); ++i) {
            int channel = channels.at(i);
            if (isChannelHasCommonBackup(channel)) {
                hasRecord = true;
                break;
            }
        }
        if (!hasRecord) {
            setPlaybackButtonState(PlaybackState_None);
        }
    }
    setCurrentTimeLine();
    PlaybackLayout::instance()->clearNoResource();
}

void PlaybackBar::ON_RESPONSE_FLAG_GET_PLAYBACK_REALTIME(MessageReceive *message)
{
    if (!message->data) {
        return;
    }
    if (m_isSeeking) {
        return;
    }
    quint32 time = *((quint32 *)message->data);
    const QDateTime &dateTime = QDateTime::fromTime_t(time);

    qMsCDebug("qt_playback_realtime") << QString("PlaybackBar::ON_RESPONSE_FLAG_GET_PLAYBACK_REALTIME, %1").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss"));
    if (dateTime.date() != playbackDate()) {
        qWarning() << QString("PlaybackBar::ON_RESPONSE_FLAG_GET_PLAYBACK_REALTIME, real datetime = %1, play date = %2")
                          .arg(dateTime.toString("yyyy-MM-dd HH:mm:ss"))
                          .arg(playbackDate().toString("yyyy-MM-dd"));
        return;
    }

    setPlaybackTime(dateTime.time());
    setDateTimeText(dateTime);
    ui->timeLine->setCurrentTime(dateTime);
}

void PlaybackBar::ON_RESPONSE_FLAG_GET_SPLIT_PLAYBACK_TIME(MessageReceive *message)
{
    if (m_isSeeking) {
        return;
    }

    QString text;
    text.append("\n----RESPONSE_FLAG_GET_SPLIT_PLAYBACK_TIME----");
    resp_split_pb_time *split_time = (resp_split_pb_time *)(message->data);
    if (split_time) {
        PlaybackSplit *split = PlaybackSplit::instance();
        Q_ASSERT(split);

        int channel = split->playingChannel();
        int sid = split->selectedSid();
        int sidCount = split->layoutMode();
        text.append(QString("\n----chnid: %1").arg(split_time->chnid));
        if (channel == split_time->chnid) {
            for (int i = 0; i < sidCount; ++i) {
                uint time = split_time->nTime[i];
                const QDateTime &dateTime = QDateTime::fromTime_t(time);
                m_splitDateTimeMap.insert(i, dateTime);
                text.append(QString("\n----[%1]%2").arg(i).arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));
            }

            const QDateTime &currentDateTime = m_splitDateTimeMap.value(sid);
            setPlaybackTime(currentDateTime.time());
            //qWarning() << sid << currentDateTime << currentDateTime.toString("yyyy-MM-dd HH:mm:ss");
            setDateTimeText(currentDateTime);
            ui->timeLine->setCurrentTime(currentDateTime);
        }
    }
    qMsCDebug("qt_playback_splittime") << text;
}

void PlaybackBar::ON_RESPONSE_FLAG_SEEK_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)
    m_isSeeking = false;
}

void PlaybackBar::ON_RESPONSE_FLAG_SEEK_SPLIT_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)
    m_isSeeking = false;
}

void PlaybackBar::ON_RESPONSE_FLAG_CHECK_AUDIOTALK(MessageReceive *message)
{
    m_nvrTalkState = *(int *)(message->data);
    m_eventLoop.exit();
}

void PlaybackBar::setSnapshotButtonEnable(bool enable)
{
    if (qMsNvr->isSlaveMode()) {
        gPlaybackVideoBar->setSnapshotButtonEnabled(false);
        return;
    }
    gPlaybackVideoBar->setSnapshotButtonEnabled(enable);
}

void PlaybackBar::setLockButtonEnable(bool enable)
{
    if (qMsNvr->isSlaveMode()) {
        gPlaybackVideoBar->setLockButtonEnabled(false);
        ui->toolButtonLockAll->setEnabled(false);
        return;
    }
    gPlaybackVideoBar->setLockButtonEnabled(enable);
    ui->toolButtonLockAll->setEnabled(enable);
}

void PlaybackBar::setQuickTagButtonEnable(bool enable)
{
    if (qMsNvr->isSlaveMode()) {
        gPlaybackVideoBar->setQuickTagButtonEnable(false);
        ui->toolButtonQuickTagAll->setEnabled(false);
        return;
    }
    gPlaybackVideoBar->setQuickTagButtonEnable(enable);
    ui->toolButtonQuickTagAll->setEnabled(enable);
}

void PlaybackBar::setCustomTagButtonEnable(bool enable)
{
    if (qMsNvr->isSlaveMode()) {
        gPlaybackVideoBar->setCustomTagButtonEnable(false);
        ui->toolButtonCustomTagAll->setEnabled(false);
        return;
    }
    gPlaybackVideoBar->setCustomTagButtonEnable(enable);
    ui->toolButtonCustomTagAll->setEnabled(enable);
}

void PlaybackBar::setDateTimeText(const QDateTime &dateTime)
{
    QString text = dateTime.toString("yyyy-MM-dd HH:mm:ss");
    setDateTimeText(text);
}

void PlaybackBar::setDateTimeText(const QString &text)
{
    if (text.isEmpty()) {
        ui->label_datetime->setText(QString("%1 00:00:00").arg(playbackDate().toString("yyyy-MM-dd")));
    } else {
        ui->label_datetime->setText(text);
    }
}

void PlaybackBar::updateSmartSearchButtonState()
{
    bool buttonEnable = true;
    bool buttonChecked = true;

    //
    switch (playbackState()) {
    case PlaybackState_None:
    case PlaybackState_Stop:
        buttonEnable = false;
        buttonChecked = false;
        break;
    default:
        break;
    }

    //
    if (playbackType() != GeneralPlayback) {
        buttonEnable = false;
    }

    //
    if (PlaybackZoom::instance()->isZooming()) {
        buttonEnable = false;
        buttonChecked = false;
    }

    if (s_smartSpeed->isSmartPlaySpeedEnable() || getFilterEvent() != INFO_MAJOR_NONE) {
        buttonEnable = false;
        buttonChecked = false;
    }

#ifdef MS_FISHEYE_SOFT_DEWARP
    if (s_fisheyeControl->isVisible()) {
        buttonEnable = false;
        buttonChecked = false;
    }
#endif

    //
    if (!s_smartSearch->isVisible()) {
        buttonChecked = false;
    }

    //
    gPlaybackVideoBar->setSmartSearchButtonState(buttonEnable, buttonChecked);
    gPlaybackVideoBar->setSmartButtonEnabled(getFilterEvent() == INFO_MAJOR_NONE);

    //
    ui->toolButton_smartSpeed->setEnabled(!buttonChecked && getFilterEvent() == INFO_MAJOR_NONE);
}

void PlaybackBar::updateFisheyeButtonState()
{
    bool fisheyeButtonEnable = true;
    bool fisheyeButtonChecked = true;

    int channel = currentChannel();

    //
    switch (playbackState()) {
    case PlaybackState_Play:
    case PlaybackState_Reverse:
        break;
    default:
        fisheyeButtonEnable = false;
        fisheyeButtonChecked = false;
        break;
    }

#ifdef MS_FISHEYE_SOFT_DEWARP
    //
    FisheyeKey fKey;
    if (playbackType() == SplitPlayback) {
        int sid = PlaybackSplit::instance()->selectedSid();
        fKey = FisheyeKey(FisheyeDewarpControl::ModePlaybackSplit, channel, sid, SubControl::instance()->currentScreen());
    } else {
        fKey = FisheyeKey(FisheyeDewarpControl::ModePlayback, channel, -1, SubControl::instance()->currentScreen());
    }
    if (s_fisheyeControl->fisheyeChannel((FisheyeDewarpControl::Mode)fKey.mode) != fKey) {
        fisheyeButtonChecked = false;
    }
#else
    Q_UNUSED(channel)
#endif

    //
    if (PlaybackZoom::instance()->isZooming()) {
        fisheyeButtonEnable = false;
        fisheyeButtonChecked = false;
    }

    if (s_smartSearch->isSmartSearchMode()) {
        fisheyeButtonEnable = false;
        fisheyeButtonChecked = false;
    }

    //
    gPlaybackVideoBar->setFisheyeButtonState(fisheyeButtonEnable, fisheyeButtonChecked);
}

void PlaybackBar::updateZoomButtonState()
{
    bool zoomButtonEnable = true;
    bool zoomButtonChecked = true;

    int channel = currentChannel();

    //
    switch (playbackState()) {
    case PlaybackState_Play:
    case PlaybackState_Reverse:
        break;
    default:
        zoomButtonEnable = false;
        zoomButtonChecked = false;
        break;
    }

#ifdef MS_FISHEYE_SOFT_DEWARP
    FisheyeKey fKey;
    if (playbackType() == SplitPlayback) {
        int sid = PlaybackSplit::instance()->selectedSid();
        fKey = FisheyeKey(FisheyeDewarpControl::ModePlaybackSplit, channel, sid, SubControl::instance()->currentScreen());
    } else {
        fKey = FisheyeKey(FisheyeDewarpControl::ModePlayback, channel, -1, SubControl::instance()->currentScreen());
    }
    if (s_fisheyeControl->fisheyeChannel((FisheyeDewarpControl::Mode)fKey.mode) == fKey) {
        zoomButtonEnable = false;
    }
    if (s_fisheyeControl->isVisible()) {
        zoomButtonEnable = false;
        zoomButtonChecked = false;
    }
#else
    Q_UNUSED(channel)
#endif

    //
    if (s_smartSearch->isSmartSearchMode()) {
        zoomButtonEnable = false;
        zoomButtonChecked = false;
    }

    //
    if (!PlaybackZoom::instance()->isZooming()) {
        zoomButtonChecked = false;
    }

    gPlaybackVideoBar->setZoomButtonState(zoomButtonEnable, zoomButtonChecked);
}

void PlaybackBar::updateFilterButtonState()
{
    if (s_smartSearch->isSmartSearchMode() || s_smartSpeed->isSmartPlaySpeedEnable()) {
        ui->toolButtonFilterEvent->setEnabled(false);
    } else {
        ui->toolButtonFilterEvent->setEnabled(true);
    }
}

void PlaybackBar::onPlaybackModeChanged(MsPlaybackType mode)
{
    gPlaybackEventData.clearAll();
    //
    if (mode == GeneralPlayback) {
        gPlaybackVideoBar->setSmartSearchButtonVisible(true);
        ui->toolButton_smartSpeed->setVisible(true);
        ui->toolButtonFilterEvent->setVisible(true);
    } else {
        gPlaybackVideoBar->setSmartSearchButtonVisible(false);
        ui->toolButton_smartSpeed->setVisible(false);
        ui->toolButtonFilterEvent->setVisible(false);
        ui->toolButtonFilterEvent->setChecked(false);
    }
    //
    if (mode == SplitPlayback) {
        ui->toolButton_split_4->setChecked(true);
        PlaybackSplit *playbackSplit = PlaybackSplit::instance();
        if (playbackSplit) {
            playbackSplit->setLayoutMode(SplitLayout_4);
        }
        ui->widget_split->show();
    } else {
        ui->widget_split->hide();
    }

#ifdef MS_FISHEYE_SOFT_DEWARP
    BasePlayback::s_fisheyeControl->clearFisheyeChannel(FisheyeDewarpControl::ModePlayback);
    BasePlayback::s_fisheyeControl->clearFisheyeChannel(FisheyeDewarpControl::ModePlaybackSplit);
#endif
    //
    if (s_smartSearch->isVisible()) {
        s_smartSearch->closeSmartSearch();
    }
    s_smartSpeed->closeSmartSpeed();
    s_filterEventPanel->close();

    closePlaybackCut();
}

void PlaybackBar::onSelectedDateChanged(const QDate &date)
{
    setDateTimeText(QString("%1 00:00:00").arg(date.toString("yyyy-MM-dd")));
    ui->timeLine->setJumped();
}

void PlaybackBar::onFisheyePanelButtonStateChanged(int state)
{
    setFisheyeDewarpState(state);

    updateFisheyeButtonState();
    updateSmartSearchButtonState();
    updateZoomButtonState();
}

void PlaybackBar::onFisheyeDewarpControlClosed()
{
    if (playbackType() == SplitPlayback) {
        PlaybackLayoutSplit::instance()->closeFullPlayback();
    } else {
        PlaybackLayout::instance()->leaveSingleLayout();
    }

    updateFisheyeButtonState();
    updateSmartSearchButtonState();
    updateZoomButtonState();

    //
    gPlaybackData.setFisheyeMode(false);
}

void PlaybackBar::onSmartSpeedPanelButtonStateChanged(int state)
{
    switch (state) {
    case 0:
        ui->toolButton_smartSpeed->setChecked(false);
        break;
    case 1:
        ui->toolButton_smartSpeed->setChecked(true);
        break;
    default:
        break;
    }
    //
    updatePlaybackButtonState();
    updateFilterButtonState();
}

void PlaybackBar::onZoomStateChanged(int state)
{
    Q_UNUSED(state)
    //qWarning() << "zoom state changed:" << state;
    updateZoomButtonState();
    updateSmartSearchButtonState();
    updateFisheyeButtonState();
    ui->toolButtonPos->setEnabled(state == 0);
}

void PlaybackBar::onChannelStarted(int channel)
{
    setChannelPlayState(channel, MsPlayingState);
    setPlaybackButtonState(PlaybackState_Play);
    setCurrentTimeLine();
}

void PlaybackBar::onChannelStoped(int channel)
{
    setChannelPlayState(channel, MsStopedState);
    //停止后清除对应的事件录像信息
    gPlaybackEventData.clear(channel);

    //
    if (channelCheckedList().isEmpty()) {
        if (s_smartSearch->isSmartSearchMode()) {
            s_smartSearch->closeSmartSearch();
        }
        setPlaybackButtonState(PlaybackState_None);
    } else {
        bool hasRecord = false;
        QList<int> channels = channelCheckedList();
        for (int i = 0; i < channels.size(); ++i) {
            int channel = channels.at(i);
            if (isChannelHasCommonBackup(channel)) {
                hasRecord = true;
                break;
            }
        }
        if (!hasRecord) {
            setPlaybackButtonState(PlaybackState_None);
        }
    }
    setCurrentTimeLine();
}

void PlaybackBar::onLanguageChanged()
{
    ui->label_channel_2->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));

    ui->toolButton_speedDown->setToolTip(GET_TEXT("PLAYBACK/80000", "Speed Down"));
    ui->toolButton_stepReverse->setToolTip(GET_TEXT("PLAYBACK/80006", "Reverse Step"));
    if (ui->toolButton_rewind->isChecked()) {
        ui->toolButton_rewind->setToolTip(GET_TEXT("PLAYBACK/80003", "Pause"));
    } else {
        ui->toolButton_rewind->setToolTip(GET_TEXT("PLAYBACK/80012", "Rewind"));
    }
    ui->toolButton_stop->setToolTip(GET_TEXT("PTZCONFIG/36038", "Stop"));
    if (ui->toolButton_play->isChecked()) {
        ui->toolButton_play->setToolTip(GET_TEXT("PLAYBACK/80003", "Pause"));
    } else {
        ui->toolButton_play->setToolTip(GET_TEXT("PTZCONFIG/36022", "Play"));
    }
    ui->toolButton_stepForward->setToolTip(GET_TEXT("PLAYBACK/80005", "Forward Step"));
    ui->toolButton_speedUp->setToolTip(GET_TEXT("PLAYBACK/80001", "Speed Up"));

    ui->toolButton_cut->setToolTip(GET_TEXT("PLAYBACK/80038", "Cut"));
    ui->toolButtonLockAll->setToolTip(GET_TEXT("PLAYBACK/80136", "Lock for All"));
    ui->toolButtonQuickTagAll->setToolTip(GET_TEXT("PLAYBACK/80137", "Quick Tag for All"));
    ui->toolButtonCustomTagAll->setToolTip(GET_TEXT("PLAYBACK/80138", "Custom Tag for All"));
    ui->toolButton_fileManagement->setToolTip(GET_TEXT("PLAYBACK/80088", "File Management"));

    ui->toolButton_smartSpeed->setToolTip(GET_TEXT("PLAYBACK/80125", "Smart Play Speed"));
    ui->toolButton_bestDecoding->setToolTip(GET_TEXT("PLAYBACK/80117", "Best Decoding Performance"));
    ui->toolButtonPos->setToolTip(GET_TEXT("POS/130024", "POS Information"));

    ui->toolButtonFilterEvent->setToolTip(GET_TEXT("PLAYBACK/167000", "Filter"));
}

void PlaybackBar::onPlaybackTimer()
{
    static int ignoreCount = 0;
    if (m_isSeeking) {
        if (ignoreCount < 5) {
            ignoreCount++;
            return;
        }
    }
    ignoreCount = 0;

    if (playbackType() == SplitPlayback) {
        PlaybackSplit *split = PlaybackSplit::instance();
        Q_ASSERT(split);

        //        req_split_play req;
        //        memset(&req, 0, sizeof(req));
        //        req.chnid = split->channel();
        //        split->makeSidMask(req.sidMaskl, sizeof(req.sidMaskl));
        //        MsCDebug("qt_playback_splittime") << "\n----REQUEST_FLAG_GET_SPLIT_PLAYBACK_TIME----"
        //                                        << "\n----chnid:" << req.chnid
        //                                        << "\n----sidMaskl:" << req.sidMaskl;
        //        MsSendMessage(SOCKET_TYPE_GUI, REQUEST_FLAG_GET_SPLIT_PLAYBACK_TIME, &req, sizeof(req));

        QString sidMaskl;
        if (ui->toolButton_split_4->isChecked()) {
            sidMaskl = QString("1111");
        } else if (ui->toolButton_split_9->isChecked()) {
            sidMaskl = QString("111111111");
        } else if (ui->toolButton_split_16->isChecked()) {
            sidMaskl = QString("1111111111111111");
        }
        PlaybackRealTimeThread::instance()->getSplitPlaybackRealTime(split->channel(), sidMaskl);
    } else {
        //MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_PLAYBACK_REALTIME, NULL, 0);
        PlaybackRealTimeThread::instance()->getPlaybackRealTime();
    }
}

void PlaybackBar::onPlaybackRealTime(QDateTime dateTime)
{
    if (m_isSeeking) {
        return;
    }

    qMsCDebug("qt_playback_realtime") << QString("PlaybackBar::ON_RESPONSE_FLAG_GET_PLAYBACK_REALTIME, %1").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss"));
    if (dateTime.date() != playbackDate()) {
        //        qWarning() << QString("PlaybackBar::ON_RESPONSE_FLAG_GET_PLAYBACK_REALTIME, real datetime = %1, play date = %2")
        //                          .arg(dateTime.toString("yyyy-MM-dd HH:mm:ss"))
        //                          .arg(playbackDate().toString("yyyy-MM-dd"));
        return;
    }
    setPlaybackTime(dateTime.time());
    setDateTimeText(dateTime);
    ui->timeLine->setCurrentTime(dateTime);
}

void PlaybackBar::onSplitPlaybackRealTime(int channel, QMap<int, QDateTime> dateTimeMap)
{
    if (m_isSeeking) {
        return;
    }

    QString text;
    text.append("\n----RESPONSE_FLAG_GET_SPLIT_PLAYBACK_TIME----");

    PlaybackSplit *split = PlaybackSplit::instance();
    Q_ASSERT(split);

    int playChannel = split->playingChannel();
    int sid = split->selectedSid();
    int sidCount = split->layoutMode();
    text.append(QString("\n----chnid: %1").arg(channel));
    if (playChannel == channel) {
        for (int i = 0; i < sidCount; ++i) {
            if (dateTimeMap.contains(i)) {
                QDateTime dateTime = dateTimeMap.value(i);
                m_splitDateTimeMap.insert(i, dateTime);
                text.append(QString("\n----[%1]%2").arg(i).arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));
            } else {
                text.append(QString("\n----[%1]invalid time").arg(i));
            }
        }

        const QDateTime &currentDateTime = m_splitDateTimeMap.value(sid);
        setPlaybackTime(currentDateTime.time());
        //qWarning() << sid << currentDateTime << currentDateTime.toString("yyyy-MM-dd HH:mm:ss");
        setDateTimeText(currentDateTime);
        ui->timeLine->setCurrentTime(currentDateTime);
    }

    qMsCDebug("qt_playback_splittime") << text;
}

void PlaybackBar::onSetCurrentTimeLine(int channel)
{
    qDebug() << "PlaybackBar::onSetCurrentTimeLine, channel:" << channel;
    if (channel < 0) {
        ui->label_channel->setText("-");
    } else {
        ui->label_channel->setText(QString::number(channel + 1));
    }
    //
    if (playbackType() == SplitPlayback) {

    } else {
        if (channel >= 0 && channel == audioChannel()) {
            gPlaybackVideoBar->setAudioButtonChecked(true);
            gPlaybackVideoBar->setAudioButtonToolTip(GET_TEXT("LIVEVIEW/20023", "Audio off"));
        } else {
            gPlaybackVideoBar->setAudioButtonChecked(false);
            gPlaybackVideoBar->setAudioButtonToolTip(GET_TEXT("LIVEVIEW/20022", "Audio on"));
        }
    }
    //
    ui->timeLine->updateTimeLine();
    //
    updateFisheyeButtonState();
    updateZoomButtonState();
}

void PlaybackBar::onTimeLineClicked(const QDateTime &dateTime)
{
    setPlaybackTime(dateTime.time());
    setDateTimeText(dateTime);

    switch (playbackState()) {
    case PlaybackState_None:
    case PlaybackState_Stop:
        return;
    default:
        break;
    }

    if (playbackType() == SplitPlayback) {
        PlaybackSplit *splitPlayback = PlaybackSplit::instance();
        Q_ASSERT(splitPlayback);
        int channel = splitPlayback->channel();
        int sid = splitPlayback->selectedSid();

        req_seek_time seek_time;
        memset(&seek_time, 0, sizeof(seek_time));
        seek_time.chnid = channel;
        seek_time.sid = sid;
        snprintf(seek_time.pseektime, sizeof(seek_time.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        qMsDebug() << "\n----chnid:" << seek_time.chnid
                   << "\n----sid:" << seek_time.sid
                   << "\n----pseektime:" << seek_time.pseektime;
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEEK_SPLIT_PLAYBACK, &seek_time, sizeof(seek_time));
    } else {
        seekPlayback();
    }

    m_isSeeking = true;
    //
    s_smartSpeed->manualSeek(dateTime);
    s_smartSearch->manualSeek(dateTime);
}

void PlaybackBar::onPlaybackSpeedChanged(int speed)
{
    if (speed == PLAY_SPEED_1X) {
        ui->toolButtonPos->setEnabled(true);
        emit posClicked(ui->toolButtonPos->isChecked());
    } else {
        ui->toolButtonPos->setEnabled(false);
        emit posClicked(false);
    }
}

void PlaybackBar::onSmartSearchModeChanged(int mode)
{
    qMsDebug() << mode;
    if (mode) {
        gPlaybackLayout->enterSingleLayout(currentChannel());
        s_smartSpeed->temporarilyDisableForSmartSearch();
    } else {
        gPlaybackLayout->leaveSingleLayout();
    }
    //
    updateSmartSearchButtonState();
    updateFisheyeButtonState();
    updateZoomButtonState();
    updateFilterButtonState();
    //
    updateTimeLine();
}

void PlaybackBar::onEventSearchFinished(int channel)
{
    if (currentChannel() == channel) {
        updateTimeLine();

        //
        if (s_smartSpeed) {
            s_smartSpeed->updateEventRecord();
        }
    }
    //closeWait();
    exitWait();
}

void PlaybackBar::onTagSearchFinished(int channel)
{
    if (currentChannel() == channel) {
        updateTimeLine();
    }
}

void PlaybackBar::onTagChanged(int channel)
{
    if (currentChannel() == channel) {
        updateTimeLine();
    }
}

void PlaybackBar::on_toolButton_stop_clicked()
{
    if (PlaybackZoom::instance()->isZooming()) {
        PlaybackZoom::instance()->closeZoom();
    }
    //
    if (playbackType() == SplitPlayback) {
        PlaybackSplit *split = PlaybackSplit::instance();
        split->stopSplitPlayback();
    } else {
        stopAllPlayback();
    }
    //
    if (s_smartSearch->isSmartSearchMode()) {
        s_smartSearch->closeSmartSearch();
    }
    //
    setPlaybackButtonState(PlaybackState_Stop);
}

void PlaybackBar::on_toolButton_play_clicked(bool checked)
{
    if (checked) {
        //
        if (playbackState() == PlaybackState_Stop) {
            //已经停止，重新播放
            if (playbackType() == SplitPlayback) {
                PlaybackSplit::instance()->startSplitPlayback();
            } else {
                waitForStartAllPlayback();
                if (getIsEventOnly()) {
                    s_smartSpeed->startEventPlayBack();
                }
            }
        } else {
            //暂停等，恢复播放
            if (playbackType() == SplitPlayback) {
                PlaybackSplit::instance()->forwardSplitPlayback();
                PlaybackSplit::instance()->resumeSplitPlayback();
            } else {
                resumeAllPlayback();
            }
        }
        setPlaybackButtonState(PlaybackState_Play);
        emit pauseClicked(false);
    } else {
        //暂停
        if (playbackType() == SplitPlayback) {
            PlaybackSplit::instance()->pauseSplitPlayback();
        } else {
            pauseAllPlayback();
        }
        setPlaybackButtonState(PlaybackState_Pause);
        emit pauseClicked(true);
    }
}

void PlaybackBar::on_toolButton_rewind_clicked(bool checked)
{
    if (checked) {
        if (playbackType() == SplitPlayback) {
            PlaybackSplit::instance()->backwardSplitPlayback();
            PlaybackSplit::instance()->resumeSplitPlayback();
        } else {
            backwardAllPlayback();
        }
        setPlaybackButtonState(PlaybackState_Reverse);
    } else {
        //暂停
        if (playbackType() == SplitPlayback) {
            PlaybackSplit::instance()->pauseSplitPlayback();
        } else {
            pauseAllPlayback();
        }
        setPlaybackButtonState(PlaybackState_Pause);
    }
}

void PlaybackBar::on_toolButton_stepForward_clicked()
{
    if (playbackType() == SplitPlayback) {
        PlaybackSplit *split = PlaybackSplit::instance();
        split->forwardSplitPlayback();
        split->stepSplitPlayback();
    } else {
        stepForwardAllPlayback();
    }
}

void PlaybackBar::on_toolButton_stepReverse_clicked()
{
    if (playbackType() == SplitPlayback) {
        PlaybackSplit *split = PlaybackSplit::instance();
        split->backwardSplitPlayback();
        split->stepSplitPlayback();
    } else {
        stepBackwardAllPlayback();
    }

    //
    ui->toolButtonPos->setChecked(false);
    emit posClicked(false);
}

void PlaybackBar::on_toolButton_speedUp_clicked()
{
    speedUpAllPlayback();

    ui->label_speed->setText(playbackSpeedString());
    if (playbackSpeed() != PLAY_SPEED_1X) {
        setAudioEnable(false);
        gPlaybackVideoBar->setAudioButtonEnabled(false);
    } else {
        gPlaybackVideoBar->setAudioButtonEnabled(true);
    }

    switch (playbackSpeed()) {
    case PLAY_SPEED_0_03125X:
    case PLAY_SPEED_0_0625X:
    case PLAY_SPEED_0_125X:
    case PLAY_SPEED_0_25X:
    case PLAY_SPEED_0_5X:
    case PLAY_SPEED_1X:
        m_playbackTimer->setInterval(1000);
        break;
    case PLAY_SPEED_2X:
        m_playbackTimer->setInterval(1000 / 2.0);
        break;
    case PLAY_SPEED_4X:
        m_playbackTimer->setInterval(1000 / 4.0);
        break;
    case PLAY_SPEED_8X:
        m_playbackTimer->setInterval(1000 / 8.0);
        break;
    case PLAY_SPEED_16X:
        m_playbackTimer->setInterval(1000 / 16.0);
        break;
    case PLAY_SPEED_32X:
        m_playbackTimer->setInterval(1000 / 32.0);
        break;
    case PLAY_SPEED_64X:
        m_playbackTimer->setInterval(1000 / 64.0);
        break;
    case PLAY_SPEED_128X:
        m_playbackTimer->setInterval(1000 / 128.0);
        break;
    default:
        m_playbackTimer->setInterval(1000);
        break;
    }
}

void PlaybackBar::on_toolButton_speedDown_clicked()
{
    speedDownAllPlayback();
    //降速后跳转当前时间(底层速度跟不上)
    //seekPlayback();

    ui->label_speed->setText(playbackSpeedString());
    if (playbackSpeed() != PLAY_SPEED_1X) {
        setAudioEnable(false);
        gPlaybackVideoBar->setAudioButtonEnabled(false);
    } else {
        gPlaybackVideoBar->setAudioButtonEnabled(true);
    }
}

void PlaybackBar::onSmartSearchClicked()
{
    if (SmartSearchControl::instance()->isSmartSearchMode()) {
        SmartSearchControl::instance()->closeSmartSearch();
    } else {
        const QList<int> &channelList = channelCheckedList();
        if (channelList.size() > 1) {
            ShowMessageBox(GET_TEXT("FISHEYE/12006", "Only available when playback with single channel."));
            return;
        }
        SmartSearchControl::instance()->showSmartSearch();
    }
}

void PlaybackBar::onFisheyeClicked()
{
#ifdef MS_FISHEYE_SOFT_DEWARP
    FisheyeKey fKey;
    if (playbackType() == SplitPlayback) {
        PlaybackLayoutSplit::instance()->showFullPlayback();

        int channel = PlaybackSplit::instance()->channel();
        int sid = PlaybackSplit::instance()->selectedSid();
        fKey = FisheyeKey(FisheyeDewarpControl::ModePlaybackSplit, channel, sid, SubControl::instance()->currentScreen());
    } else {
        const QList<int> &channelList = channelCheckedList();
        int channel = currentChannel();
        if (channelList.size() > 1) {
            PlaybackLayout::instance()->enterSingleLayout(channel);
        }
        fKey = FisheyeKey(FisheyeDewarpControl::ModePlayback, channel, -1, SubControl::instance()->currentScreen());
    }
    FisheyeDewarpControl::setCurrentChannel(fKey);
    FisheyeDewarpControl::setVapiWinId(VAPI_WIN_ID(SubControl::instance()->currentScreen(), 0));
    s_fisheyeControl->showFisheyeDewarpControl(fKey);

    QMetaObject::invokeMethod(this, "showFisheyePanel", Qt::QueuedConnection);

    updateFisheyeButtonState();
    updateZoomButtonState();
    updateSmartSearchButtonState();

    //
    s_smartSpeed->closePanel();
#endif
}

void PlaybackBar::showFisheyePanel()
{
    QPoint p = mapToGlobal(QPoint(0, 0));
    emit fisheyePanelButtonClicked(p.x(), p.y());
}

void PlaybackBar::onZoomClicked()
{
    //
    if (BasePlayback::playbackType() == SplitPlayback) {
        PlaybackLayoutSplit::instance()->showFullPlayback();
    } else {
        if (!PlaybackLayout::instance()->isSingleLayout()) {
            int channel = currentChannel();
            PlaybackLayout::instance()->enterSingleLayout(channel);
        }
    }

    //
    if (playbackType() == SplitPlayback) {
        PlaybackLayoutSplit::instance()->dealZoom();
    } else {
        int channel = currentChannel();
        PlaybackLayout::instance()->dealZoom(channel);
    }
    updateZoomButtonState();
    updateSmartSearchButtonState();
    updateFisheyeButtonState();
}

void PlaybackBar::onSnapshotClicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_SNAPSHOT)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    //
    int win_id = 0;
    if (playbackType() == SplitPlayback) {
        win_id = PlaybackLayoutSplit::instance()->currentWinId();
    } else {
        win_id = PlaybackLayout::instance()->vapiWinId(currentChannel());
    }
    if (win_id == 0) {
        qMsWarning() << "win_id is 0";
        ShowMessageBox(GET_TEXT("LIVEVIEW/20057", "Snapshot Failed"));
        return;
    }
    qMsDebug() << VapiWinIdString(win_id);
    if (qMsNvr->isNoResource(win_id)) {
        qMsDebug() << "no resource";
        ShowMessageBox(GET_TEXT("LIVEVIEW/20057", "Snapshot Failed"));
        return;
    }
    //
    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeSnapshot);
    m_animateToast->showToast(GET_TEXT("PLAYBACK/80095", "Snapshotting..."));
    //等待剪切完毕
    int succeedCount = m_fileManagement->waitForSnapshot(win_id);
    //超时
    if (succeedCount == -2) {
        m_animateToast->hideToast();
        ShowMessageBox("Operation timed out, please try again.");
    } else {
        //剪切完毕
        m_animateToast->startAnimationLater(2000);
    }

    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeNone);
}

void PlaybackBar::waitForCheckNvrTalkSupport()
{
    m_nvrTalkState = 0;

    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_CHECK_AUDIOTALK, nullptr, 0);
    //m_eventLoop.exec();
}

void PlaybackBar::onAudioClicked(bool checked)
{
    qMsDebug() << checked;

    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_AUDIO)) {
        gPlaybackVideoBar->setAudioButtonChecked(false);
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    setAudioEnable(checked);
}

void PlaybackBar::setAudioEnable(bool checked)
{
    if (checked) {
        if (qMsNvr->isLiveviewAudioOpen() || audioChannel() != -1) {
            int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20105", "The other audio will be disabled, continue?"));
            if (result == MessageBox::Cancel) {
                gPlaybackVideoBar->setAudioButtonChecked(false);
                return;
            }
            qMsNvr->closeAudio();
            closePlaybackAudio();
        }
        if (playbackType() == SplitPlayback && PlaybackSplit::instance()->isAudioOpen()) {
            int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20105", "The other audio will be disabled, continue?"));
            if (result == MessageBox::Cancel) {
                gPlaybackVideoBar->setAudioButtonChecked(false);
                return;
            }
            PlaybackSplit::instance()->closeAudio();
        }
        if (qMsNvr->isCamTalkbackOpen()) {
            int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20106", "The other Two-way audio will be disabled, continue?"));
            if (result == MessageBox::Cancel) {
                gPlaybackVideoBar->setAudioButtonChecked(false);
                return;
            }
            qMsNvr->closeTalkback();
        }
        //MsWaitting::showGlobalWait();
        waitForCheckNvrTalkSupport();
        //MsWaitting::closeGlobalWait();
        if (m_nvrTalkState == 1) {
            gPlaybackVideoBar->setAudioButtonChecked(false);
            ShowMessageBox(this, GET_TEXT("LIVEVIEW/20108", "Device is busy."));
            return;
        }
        //
        gPlaybackVideoBar->setAudioButtonChecked(true);
        if (playbackType() == SplitPlayback) {
            PlaybackSplit::instance()->openAudio();
        } else {
            openPlaybackAudio(currentChannel());
        }
        gPlaybackVideoBar->setAudioButtonToolTip(GET_TEXT("LIVEVIEW/20023", "Audio off"));
    } else {
        gPlaybackVideoBar->setAudioButtonChecked(false);
        if (playbackType() == SplitPlayback) {
            PlaybackSplit::instance()->closeAudio();
        } else {
            closePlaybackAudio();
        }
        gPlaybackVideoBar->setAudioButtonToolTip(GET_TEXT("LIVEVIEW/20022", "Audio on"));
    }
}

void PlaybackBar::onLockClicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_LOCK)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }

    //
    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeLock);
    m_animateToast->showToast(GET_TEXT("PLAYBACK/80097", "Locking..."));
    //等待lock
    int succeedCount = m_fileManagement->waitForPlaybackLock(playbackDateTime());
    //lock完毕
    if (succeedCount > 0) {
        m_animateToast->startAnimationLater(2000);
    } else {
        m_animateToast->hideToastLater(2000);
    }

    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeNone);
}

void PlaybackBar::onQuickTagClicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_TAG)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeTag);
    m_animateToast->showToast(GET_TEXT("PLAYBACK/80098", "Tagging..."));
    //等待tag
    int succeedCount = m_fileManagement->waitForTag(playbackDateTime(), "tag");
    //tag完毕
    if (succeedCount > 0) {
        m_animateToast->startAnimationLater(2000);
    } else {
        m_animateToast->hideToastLater(2000);
    }

    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeNone);
}

void PlaybackBar::onCustomTagClicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_TAG)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    ui->toolButtonCustomTagAll->clearUnderMouse();

    QDateTime dateTime;
    if (BasePlayback::playbackType() == SplitPlayback) {
        dateTime = BasePlayback::s_playbackBar->splitDateTime();
    } else {
        dateTime = playbackDateTime();
    }

    CustomTag customTag(this);
    customTag.addTag(GET_TEXT("PLAYBACK/161000", "Tag Add"));
    int result = customTag.exec();
    if (result == CustomTag::Accepted) {
        m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeTag);
        m_animateToast->showToast(GET_TEXT("PLAYBACK/80098", "Tagging..."));
        //等待tag
        int succeedCount = m_fileManagement->waitForTag(dateTime, customTag.tagName());
        //tag完毕
        if (succeedCount > 0) {
            m_animateToast->startAnimationLater(2000);
        } else {
            m_animateToast->hideToastLater(2000);
        }

        m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeNone);
    }
}

void PlaybackBar::on_toolButton_cut_clicked()
{
    //
    if (!m_cut) {
        m_cut = new PlaybackCut(this);
    }
    ui->timeLine->openCut();
    m_cut->move(geometry().right() - m_cut->width(), geometry().top() - m_cut->height());
    const int &result = m_cut->showCut();
    //
    if (result == PlaybackCut::Accepted) {
        m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeVideo);

        QDateTime beginTime = m_cut->beginDateTime();
        QDateTime endTime = m_cut->endDateTime();

        switch (playbackType()) {
        case EventPlayback:
        case TagPlayback: {
            if (beginTime < PlaybackList::s_beginDateTime) {
                beginTime = PlaybackList::s_beginDateTime;
            }
            if (endTime > PlaybackList::s_endDateTime) {
                endTime = PlaybackList::s_endDateTime;
            }
            break;
        }
        default:
            break;
        }
        m_animateToast->showToast(GET_TEXT("PLAYBACK/80096", "Cutting..."));
        //等待剪切完成
        int result = m_fileManagement->waitForCutPlayback(beginTime, endTime);
        //剪切完成
        switch (result) {
        case PlaybackError_NoError:
            m_animateToast->startAnimationLater(1000);
            break;
        default:
            m_animateToast->showToast(GET_TEXT("PLAYBACK/80099", "Cut failed"), 2000);
            break;
        }
        m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeNone);
    }
}

void PlaybackBar::on_toolButtonLockAll_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_LOCK)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    ui->toolButtonLockAll->clearUnderMouse();

    //
    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeLock);
    m_animateToast->showToast(GET_TEXT("PLAYBACK/80097", "Locking..."));
    //等待lock
    int succeedCount = m_fileManagement->waitForPlaybackLockAll(playbackDateTime());
    //lock完毕
    if (succeedCount > 0) {
        m_animateToast->startAnimationLater(2000);
    } else {
        m_animateToast->hideToastLater(2000);
    }

    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeNone);
}

void PlaybackBar::on_toolButtonQuickTagAll_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_TAG)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    ui->toolButtonQuickTagAll->clearUnderMouse();

    QMap<int, QDateTime> splitDateTimeMap;
    if (BasePlayback::playbackType() == SplitPlayback) {
        splitDateTimeMap = BasePlayback::s_playbackBar->splitDateTimeMap();
    } else {
        splitDateTimeMap.insert(0, playbackDateTime());
    }

    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeTag);
    m_animateToast->showToast(GET_TEXT("PLAYBACK/80098", "Tagging..."));
    //等待tag
    int succeedCount = m_fileManagement->waitForTagAll(splitDateTimeMap, "tag");
    //tag完毕
    if (succeedCount > 0) {
        m_animateToast->startAnimationLater(2000);
    } else {
        m_animateToast->hideToastLater(2000);
    }

    m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeNone);
}

void PlaybackBar::on_toolButtonCustomTagAll_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_TAG)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    ui->toolButtonCustomTagAll->clearUnderMouse();

    QMap<int, QDateTime> splitDateTimeMap;
    if (BasePlayback::playbackType() == SplitPlayback) {
        splitDateTimeMap = BasePlayback::s_playbackBar->splitDateTimeMap();
    } else {
        splitDateTimeMap.insert(0, playbackDateTime());
    }

    CustomTag customTag(this);
    customTag.addTag(GET_TEXT("PLAYBACK/80078", "Tags Add"));
    int result = customTag.exec();
    if (result == CustomTag::Accepted) {
        m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeTag);
        m_animateToast->showToast(GET_TEXT("PLAYBACK/80098", "Tagging..."));
        //等待tag
        int succeedCount = m_fileManagement->waitForTagAll(splitDateTimeMap, customTag.tagName());
        //tag完毕
        if (succeedCount > 0)
            m_animateToast->startAnimationLater(2000);
        else
            m_animateToast->hideToastLater(2000);

        m_fileManagement->setCurrentMode(PlaybackFileManagement::ModeNone);
    }
}

void PlaybackBar::on_toolButton_fileManagement_clicked()
{
    ui->toolButton_fileManagement->clearUnderMouse();

    //点击文件管理时暂停播放
    const MsPlaybackState &state = playbackState();
    switch (state) {
    case PlaybackState_Play:
        ui->toolButton_play->setChecked(false);
        on_toolButton_play_clicked(false);
        break;
    case PlaybackState_Pause:
        break;
    case PlaybackState_Reverse:
        ui->toolButton_rewind->setChecked(false);
        on_toolButton_rewind_clicked(false);
        break;
    default:
        break;
    }
    //
    m_fileManagement->initializeData();
    m_fileManagement->exec();
    //
    switch (state) {
    case PlaybackState_Play:
        ui->toolButton_play->setChecked(true);
        on_toolButton_play_clicked(true);
        break;
    case PlaybackState_Pause:
        break;
    case PlaybackState_Reverse:
        ui->toolButton_rewind->setChecked(true);
        on_toolButton_rewind_clicked(true);
        break;
    default:
        break;
    }
}

void PlaybackBar::on_toolButton_smartSpeed_clicked()
{
    ui->toolButton_smartSpeed->setChecked(s_smartSpeed->isSmartPlaySpeedEnable());

    const QList<int> &channelList = channelCheckedList();
    if (channelList.size() > 1) {
        ui->toolButton_smartSpeed->clearUnderMouse();
        ShowMessageBox(GET_TEXT("FISHEYE/12006", "Only available when playback with single channel."));
        return;
    }

    QPoint p = ui->toolButton_smartSpeed->mapToGlobal(QPoint(0, 0));
    int x = p.x() + ui->toolButton_smartSpeed->width() / 2;
    emit smartSpeedPanelButtonClicked(x, p.y() - 5);

    //
#ifdef MS_FISHEYE_SOFT_DEWARP
    s_fisheyeControl->closeFisheyePanel();
    gPlaybackData.setFisheyeMode(false);
#endif
}

void PlaybackBar::on_toolButton_bestDecoding_clicked(bool checked)
{
    if (checked) {
        ui->toolButton_bestDecoding->clearUnderMouse();
        const int result = MessageBox::question(this, GET_TEXT("PLAYBACK/80118", "It will affect the live view on the other monitor when playing back. Still enable?"));
        if (result == MessageBox::Yes) {
            LiveView::instance()->stopSubScreen();
            if (LiveViewSub::instance()) {
                LiveViewSub::instance()->clearAllPosData();
            }
            if (playbackType() == SplitPlayback) {
                PlaybackLayoutSplit::instance()->resetSplitLayout();
                PlaybackSplit::instance()->resumeSplitPlayback();
            } else {
                PlaybackLayout::instance()->refreshLayout();
            }
        } else {
            ui->toolButton_bestDecoding->setChecked(false);
        }
    } else {
        LiveView::instance()->initializeSubScreenLayout();
    }
}

void PlaybackBar::on_toolButtonPos_clicked(bool checked)
{
    emit posClicked(checked);
}

void PlaybackBar::onSplitButtonGroupClicked(int id)
{
    PlaybackWindow::instance()->closeFisheyeDewarp();
    if (PlaybackZoom::instance()->isZooming()) {
        PlaybackZoom::instance()->closeZoom();
    }

    //
    MsSplitLayoutMode mode = static_cast<MsSplitLayoutMode>(id);

    PlaybackSplit *playbackSplit = PlaybackSplit::instance();
    if (playbackSplit) {
        playbackSplit->setLayoutMode(mode);
    }
}

void PlaybackBar::onRockerTimer()
{
    if (m_rockerMode == 0) {
        ui->timeLine->forwardSec(30);
    } else {
        ui->timeLine->backwardSec(30);
    }
}

void PlaybackBar::on_toolButtonFilterEvent_clicked()
{
    ui->toolButtonFilterEvent->clearUnderMouse();
    ui->toolButtonFilterEvent->setChecked(getFilterEvent() != INFO_MAJOR_NONE);
    const QList<int> &channelList = channelCheckedList();
    if (channelList.size() > 1) {
        ui->toolButtonFilterEvent->clearUnderMouse();
        ShowMessageBox(GET_TEXT("FISHEYE/12006", "Only available when playback with single channel."));
        return;
    }
    if (s_smartSpeed->isVisible()) {
        s_smartSpeed->closePanel();
    }
    QPoint p = ui->toolButtonFilterEvent->mapToGlobal(QPoint(0, 0));
    int x = p.x() + ui->toolButtonFilterEvent->width() / 2;
    emit filterEventPanelButtonClicked(x, p.y() - 10);
}

void PlaybackBar::onToolButtionFilterEventSearch()
{
    ui->toolButtonFilterEvent->clearUnderMouse();
    ui->toolButtonFilterEvent->setChecked(getFilterEvent() != INFO_MAJOR_NONE);
    if (isSmartSearchMode() || channelCheckedList().isEmpty()) {
        if (getFilterEvent() == INFO_MAJOR_NONE) {
            s_smartSpeed->closeEventPlayBack();
        }
        return;
    }
    waitForSearchGeneralEventPlayBack();
    if (getFilterEvent() != INFO_MAJOR_NONE && !gPlaybackEventData.hasEventBackup(currentChannel(), static_cast<uint>(getFilterEvent()))) {
        ShowMessageBox(GET_TEXT("PLAYBACK/167001", "No data."));
        if (playbackState() == PlaybackState_None && isChannelHasCommonBackup(currentChannel())) {
            s_playbackBar->setPlaybackButtonState(PlaybackState_Stop);
        } else {
            pauseAllPlayback();

            setPlaybackButtonState(PlaybackState_Pause);
        }

        updateTimeLine();
        emit pauseClicked(true);
    }
    ui->toolButton_smartSpeed->setEnabled(getFilterEvent() == INFO_MAJOR_NONE);
    if (getIsEventOnly()) {
        s_smartSpeed->startEventPlayBack();
    } else {
        s_smartSpeed->closeEventPlayBack();
    }
}
