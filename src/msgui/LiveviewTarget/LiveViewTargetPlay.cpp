#include "LiveViewTargetPlay.h"
#include "ui_LiveViewTargetPlay.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "LiveView.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "SubControl.h"
#include "testanprplay.h"
#include <QDesktopWidget>
#include <QMouseEvent>

#define ANPR_WINDOW_INDEX MAX_REAL_CAMERA

LiveViewTargetPlay *LiveViewTargetPlay::s_targetPlay = nullptr;

LiveViewTargetPlay::LiveViewTargetPlay(QWidget *parent)
    : BaseDialog(parent)
    , ui(new Ui::LiveViewTargetPlay)
{
    ui->setupUi(this);

    s_targetPlay = this;
    setWindowModality(Qt::ApplicationModal);

    connect(ui->slider_time, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onRealPlaybackTime()));
    m_timer->setInterval(1000);

    m_waitting = new MsWaitting(this);
    m_waitting->setWindowModality(Qt::ApplicationModal);
    m_waitting->setCustomPos(true);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    //
    connect(qMsNvr, SIGNAL(noResourceChanged(int, int)), this, SLOT(onNoResourceChanged(int, int)));
}

LiveViewTargetPlay::~LiveViewTargetPlay()
{
    s_targetPlay = nullptr;
    delete ui;
}

LiveViewTargetPlay *LiveViewTargetPlay::instance()
{
    return s_targetPlay;
}

int LiveViewTargetPlay::waitForSearchAnprPlayback(int channel, const QDateTime &dateTime, const QRect &rc)
{
    showNoResource(false);
    m_channel = channel;

    m_videoRect = rc;
    m_videoRect.setBottom(rc.bottom() - 40);
    m_videoRect.setTop(rc.top() + 1);
    m_videoRect.setLeft(rc.left() + 1);
    m_videoRect.setRight(rc.right() - 1);

    //兼容Qt获取的屏幕分辨率和底层实际分辨率不符合的情况。
    int nvrWidth = 0;
    int nvrHeight = 0;
    QRect m_nvrRect;
    vapi_get_screen_res(SubControl::instance()->currentScreen(), &nvrWidth, &nvrHeight);
    //
    QRect screenRect = qApp->desktop()->geometry();
    if (screenRect.width() == nvrWidth && screenRect.height() == nvrHeight) {
        m_nvrRect = m_videoRect;
    } else {
        qreal xRatio = (qreal)screenRect.width() / nvrWidth;
        qreal yRatio = (qreal)screenRect.height() / nvrHeight;
        m_nvrRect.setLeft(m_videoRect.x() / xRatio);
        m_nvrRect.setTop(m_videoRect.y() / yRatio);
        m_nvrRect.setWidth(m_videoRect.width() / xRatio);
        m_nvrRect.setHeight(m_videoRect.height() / yRatio);
    }

    memset(&m_anpr_pb_info, 0, sizeof(m_anpr_pb_info));
    m_anpr_pb_info.enable = 1;
    m_anpr_pb_info.chnId = m_channel;
    m_anpr_pb_info.shsid = -1;
    m_anpr_pb_info.pbsid = -1;
    snprintf(m_anpr_pb_info.pTime, sizeof(m_anpr_pb_info.pTime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    m_anpr_pb_info.stZone.enMode = ZONE_MODE_USER;
    //test
    //    static TestAnprPlay *testPlay = new TestAnprPlay(this);
    //    testPlay->show();
    //    testPlay->setPlayRect(m_nvrRect);
    //    m_nvrRect = testPlay->playRect();
    //
    m_anpr_pb_info.stZone.x = m_nvrRect.left();
    m_anpr_pb_info.stZone.y = m_nvrRect.top();
    m_anpr_pb_info.stZone.w = m_nvrRect.width();
    m_anpr_pb_info.stZone.h = m_nvrRect.height();
    m_anpr_pb_info.winid = VAPI_WIN_ID(SubControl::instance()->currentScreen(), ANPR_WINDOW_INDEX);
    qDebug() << QString("REQUEST_FLAG_SMARTAI_LIVE_PLAYBACK, channel: %1, %2").arg(m_channel).arg(VapiWinIdString(m_anpr_pb_info.winid)) << m_nvrRect;
    sendMessage(REQUEST_FLAG_SMARTAI_LIVE_PLAYBACK, (void *)&m_anpr_pb_info, sizeof(m_anpr_pb_info));

    //避免用户频繁点击
    QRect waittingRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, m_waitting->size(), m_videoRect);
    m_waitting->move(waittingRect.topLeft());
    //m_waitting->//showWait();

    //int result = m_eventLoop.exec();

    //m_waitting->//closeWait();

    return 1;
}

void LiveViewTargetPlay::execAnprPlayback(const QRect &rc)
{
    ui->slider_time->setValue(0);
    //
    const QRect &screenRect = qApp->desktop()->geometry();
    setGeometry(screenRect);

    QRect playbarRect = rc;
    playbarRect.setTop(rc.bottom() - 40);
    ui->widget_playbar->setGeometry(playbarRect);
    exec();
}

void LiveViewTargetPlay::closeAnprPlayback()
{
    on_pushButton_close_clicked();
}

void LiveViewTargetPlay::showNoResource(const QMap<int, bool> &map)
{
    QMap<int, bool>::const_iterator iter = map.constBegin();
    for (; iter != map.constEnd(); ++iter) {
        int channel = iter.key();
        bool value = iter.value();
        if (channel == m_channel) {
            showNoResource(value);
            break;
        }
    }
}

void LiveViewTargetPlay::networkPlay()
{
    if (!isVisible()) {
        return;
    }

    QMetaObject::invokeMethod(this, "on_pushButton_play_clicked", Qt::QueuedConnection);
}

void LiveViewTargetPlay::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SMARTAI_LIVE_PLAYBACK:
        ON_RESPONSE_FLAG_SET_ANPR_LIVE_PLAYBACK(message);
        break;
    }
}

NetworkResult LiveViewTargetPlay::dealNetworkCommond(const QString &commond)
{
    NetworkResult result = NetworkAccept;

    if (commond.startsWith("Video_Play")) {
        if (ui->pushButton_play->isVisible()) {
            QMetaObject::invokeMethod(this, "on_pushButton_play_clicked", Qt::QueuedConnection);
        }
    } else if (commond.startsWith("Video_Pause")) {
        if (ui->pushButton_pause->isVisible()) {
            QMetaObject::invokeMethod(this, "on_pushButton_pause_clicked", Qt::QueuedConnection);
        }
    } else if (commond.startsWith("Video_Stop")) {
        QMetaObject::invokeMethod(this, "on_pushButton_close_clicked", Qt::QueuedConnection);
    }

    return result;
}

void LiveViewTargetPlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        qDebug() << QString("LiveViewTargetPlay::mousePressEvent, Qt::RightButton");
        on_pushButton_close_clicked();
    }
}

bool LiveViewTargetPlay::isAddToVisibleList()
{
    return true;
}

void LiveViewTargetPlay::ON_RESPONSE_FLAG_SET_ANPR_LIVE_PLAYBACK(MessageReceive *message)
{
    m_listCommonBackup.clear();

    struct resp_search_common_backup *common_backup_array = (struct resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);
    if (!common_backup_array) {
        qDebug() << QString("LiveViewTargetPlay::ON_RESPONSE_FLAG_SET_ANPR_LIVE_PLAYBACK, channel: %1 data is null.").arg(m_channel);
        m_eventLoop.exit(-1);
        return;
    }
    if (count == 0) {
        qDebug() << QString("LiveViewTargetPlay::ON_RESPONSE_FLAG_SET_ANPR_LIVE_PLAYBACK, channel: %1 no record.").arg(m_channel);
        m_eventLoop.exit(-2);
        return;
    }
    for (int i = 0; i < count; ++i) {
        m_listCommonBackup.append(common_backup_array[i]);
    }
    m_sid = common_backup_array->sid;
    m_startDateTime = QDateTime::fromString(common_backup_array[0].pStartTime, "yyyy-MM-dd HH:mm:ss");
    m_currentDateTime = m_startDateTime;
    m_endDateTime = QDateTime::fromString(common_backup_array[count - 1].pEndTime, "yyyy-MM-dd HH:mm:ss");
    ui->slider_time->setTimeRange(m_startDateTime, m_endDateTime);
    ui->slider_time->setCurrentDateTime(m_currentDateTime);
    ui->label_time->setText(common_backup_array[0].pStartTime);
    qDebug() << QString("LiveViewTargetPlay::ON_RESPONSE_FLAG_SET_ANPR_LIVE_PLAYBACK, channel: %1, start time: %2, end time: %3")
                    .arg(common_backup_array->chnid)
                    .arg(common_backup_array[0].pStartTime)
                    .arg(common_backup_array[count - 1].pEndTime);
    m_timer->start();
    ui->pushButton_play->hide();
    ui->pushButton_pause->show();
    setAudioEnable(true);
    m_eventLoop.exit(0);
}

void LiveViewTargetPlay::stopAnprPlayback()
{
    if (m_sid < 0) {
        return;
    }

    m_anpr_pb_info.enable = 0;
    m_anpr_pb_info.chnId = m_channel;
    m_anpr_pb_info.shsid = m_sid;
    m_anpr_pb_info.pbsid = m_sid;
    qDebug() << QString("LiveViewTargetPlay::stopAnprPlayback, channel: %1, sid: %2, winid: %3").arg(m_channel).arg(m_sid).arg(VapiWinIdString(m_anpr_pb_info.winid));
    sendMessageOnly(REQUEST_FLAG_SMARTAI_LIVE_PLAYBACK, (void *)&m_anpr_pb_info, sizeof(m_anpr_pb_info));

    m_sid = -1;
    setAudioEnable(false);
}

void LiveViewTargetPlay::seekAnprPlayback(const QDateTime &dateTime)
{
    struct req_seek_time seek_time;
    memset(&seek_time, 0, sizeof(struct req_seek_time));
    seek_time.chnid = m_channel;
    seek_time.sid = m_sid;
    snprintf(seek_time.pseektime, sizeof(seek_time.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_PLAYSEEK_BACKUP, (void *)&seek_time, sizeof(struct req_seek_time));
}

void LiveViewTargetPlay::pauseAnprPlayback()
{
    m_timer->stop();
    sendMessage(REQUEST_FLAG_PLAYPAUSE_COM_BACKUP, (void *)&(m_sid), sizeof(int));
}

void LiveViewTargetPlay::resumeAnprPlayback()
{
    m_timer->start();
    sendMessage(REQUEST_FLAG_PLAYRESTART_COM_BACKUP, (void *)&(m_sid), sizeof(int));
}

void LiveViewTargetPlay::immediatelyUpdatePlayTime()
{
    ui->slider_time->setCurrentDateTime(m_currentDateTime);
    ui->label_time->setText(m_currentDateTime.toString("yyyy-MM-dd HH:mm:ss"));
}

void LiveViewTargetPlay::setAudioEnable(bool enable)
{
    struct req_set_audiochn audio;
    memset(&audio, 0, sizeof(struct req_set_audiochn));
    audio.chn = enable ? m_channel : -1;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(audio));
}

void LiveViewTargetPlay::showNoResource(bool show)
{
    if (show) {
        ui->label_noResource->setGeometry(m_videoRect);
        ui->label_noResource->setText(GET_TEXT("LIVEVIEW/20031", "No Resource"));

        //MSHN-7212
        //Qt-即时回放：播放该处如果是No Resource时，建议底下进度条也要停下来不要变成实时在播放的状态。
        on_pushButton_pause_clicked();
    }
    ui->label_noResource->setVisible(show);
}

void LiveViewTargetPlay::onLanguageChanged()
{
    ui->pushButton_play->setToolTip(GET_TEXT("PLAYBACK/80089", "Play"));
    ui->pushButton_pause->setToolTip(GET_TEXT("COMMONBACKUP/100046", "Pause "));
    ui->pushButton_close->setToolTip(GET_TEXT("PLAYBACK/80090", "Close"));
}

void LiveViewTargetPlay::onSliderValueChanged(int value)
{
    m_currentDateTime = QDateTime::fromTime_t(value);
    ui->label_time->setText(m_currentDateTime.toString("yyyy-MM-dd HH:mm:ss"));
    seekAnprPlayback(m_currentDateTime);
}

void LiveViewTargetPlay::onRealPlaybackTime()
{
    m_currentDateTime = m_currentDateTime.addSecs(1);
    if (m_currentDateTime > m_endDateTime) {
        on_pushButton_pause_clicked();
        return;
    }

    bool isValid = false;
    for (int i = 0; i < m_listCommonBackup.size(); ++i) {
        const resp_search_common_backup &backup = m_listCommonBackup.at(i);
        QDateTime beginDateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        QDateTime endDateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        if (m_currentDateTime >= beginDateTime && m_currentDateTime <= endDateTime) {
            isValid = true;
            break;
        }
    }
    if (!isValid) {
        for (int i = 0; i < m_listCommonBackup.size(); ++i) {
            const resp_search_common_backup &backup = m_listCommonBackup.at(i);
            QDateTime beginDateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
            if (m_currentDateTime < beginDateTime) {
                m_currentDateTime = beginDateTime;
                break;
            }
        }
    }
    ui->slider_time->setCurrentDateTime(m_currentDateTime);
    ui->label_time->setText(m_currentDateTime.toString("yyyy-MM-dd HH:mm:ss"));
}

void LiveViewTargetPlay::on_pushButton_play_clicked()
{
    ui->pushButton_play->hide();
    ui->pushButton_pause->show();
    if (m_currentDateTime > m_endDateTime) {
        //play from start time
        m_currentDateTime = m_startDateTime;
        immediatelyUpdatePlayTime();
        seekAnprPlayback(m_currentDateTime);
    }
    resumeAnprPlayback();
}

void LiveViewTargetPlay::on_pushButton_pause_clicked()
{
    ui->pushButton_play->show();
    ui->pushButton_pause->hide();
    pauseAnprPlayback();
}

void LiveViewTargetPlay::on_pushButton_close_clicked()
{
    m_timer->stop();
    stopAnprPlayback();
    close();
}

void LiveViewTargetPlay::onNoResourceChanged(int winid, int bNoResource)
{
    if (winid == 0) {
        return;
    }
    if (winid == m_anpr_pb_info.winid) {
        showNoResource(bNoResource);
    }
}
