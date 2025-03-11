#include "LiveViewPlayback.h"
#include "ui_LiveViewPlayback.h"
#include "centralmessage.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "SubControl.h"
#include <QDesktopWidget>
#include <QMouseEvent>
#include "StreamKey.h"
#include "CustomLayoutData.h"

LiveViewPlayback *LiveViewPlayback::s_liveViewPlayback = nullptr;

LiveViewPlayback::LiveViewPlayback(QWidget *parent)
    : BaseDialog(parent)
    , ui(new Ui::LiveViewPlayback)
{
    ui->setupUi(this);
    s_liveViewPlayback = this;
    setWindowModality(Qt::ApplicationModal);

    connect(ui->slider_time, SIGNAL(valueEdited(int)), this, SLOT(onSliderValueChanged(int)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onRealPlaybackTime()));
    m_timer->setInterval(1000);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    //
    showNoResource(false);

    //
    connect(qMsNvr, SIGNAL(noResourceChanged(int, int)), this, SLOT(onNoResourceChanged(int, int)));
}

LiveViewPlayback::~LiveViewPlayback()
{
    s_liveViewPlayback = nullptr;
    delete ui;
}

LiveViewPlayback *LiveViewPlayback::instance()
{
    return s_liveViewPlayback;
}

int LiveViewPlayback::currentChannel() const
{
    return m_channel;
}

void LiveViewPlayback::closePlaybackForPopup()
{
    m_result = 1;
    on_pushButton_close_clicked();
}

void LiveViewPlayback::closePlaybackForScreen()
{
    m_result = 2;
    on_pushButton_close_clicked();
}

int LiveViewPlayback::execPlayback(int channel, const QRect &videoRect, int winId)
{
    m_channel = channel;
    m_videoRect = videoRect;
    m_nvrRect = SubControl::instance()->mapToNvrRect(m_videoRect);
    m_winId = winId;
    m_result = 0;
    ui->slider_time->setValue(0);
    //
    QRect screenRect = SubControl::instance()->logicalMainScreenGeometry();
    int x = m_videoRect.left() + m_videoRect.width() / 2 - ui->widget_shadow->width() / 2;
    int y = m_videoRect.bottom() - 9;
    if (x < screenRect.left()) {
        x = screenRect.left();
    }
    if (x + ui->widget_shadow->width() > screenRect.right()) {
        x = screenRect.right() - ui->widget_shadow->width();
    }
    if ((y + ui->widget_shadow->height()) >= screenRect.bottom()) {
        y = m_videoRect.top() - ui->widget_shadow->height() + 9;
    }
    //全屏
    if (y < screenRect.top()) {
        y = screenRect.top();
    }
    ui->widget_shadow->move(x, y);
    //
    setLivePlayback();
    //
    exec();
    m_channel = -1;
    return m_result;
}

NetworkResult LiveViewPlayback::dealNetworkCommond(const QString &commond)
{
    qDebug() << "====LiveViewPlayback::dealNetworkCommond====";
    qDebug() << "====commond:" << commond;
    qDebug() << "----channel:" << m_channel;

    if (m_channel < 0) {
        return NetworkReject;
    }

    //默认为accept，保证及时回放时不响应其他指令
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

void LiveViewPlayback::showNoResource(const QMap<int, bool> &map)
{
    for (auto iter = map.constBegin(); iter != map.constEnd(); ++iter) {
        int channel = iter.key();
        bool no_resource = iter.value();
        if (channel == m_channel && no_resource) {
            pauseLivePlayback();
        }
    }
}

void LiveViewPlayback::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_LIVE_PLAYBACK:
        ON_RESPONSE_FLAG_SET_LIVE_PLAYBACK(message);
        break;
    }
}

void LiveViewPlayback::ON_RESPONSE_FLAG_SET_LIVE_PLAYBACK(MessageReceive *message)
{
    m_listCommonBackup.clear();

    struct resp_search_common_backup *common_backup_array = (struct resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);
    if (!common_backup_array || count == 0) {
        m_result = -1;
        on_pushButton_close_clicked();
        return;
    }
    for (int i = 0; i < count; ++i) {
        m_listCommonBackup.append(common_backup_array[i]);
    }
    m_sid = common_backup_array->sid;
    m_currentDateTime = QDateTime::fromString(common_backup_array[0].pStartTime, "yyyy-MM-dd HH:mm:ss");
    m_endDateTime = QDateTime::fromString(common_backup_array[count - 1].pEndTime, "yyyy-MM-dd HH:mm:ss");
    m_startDateTime = m_currentDateTime;
    ui->slider_time->setTimeRange(m_currentDateTime, m_endDateTime);
    ui->slider_time->setCurrentDateTime(m_currentDateTime);
    ui->label_time->setText(common_backup_array[0].pStartTime);
    m_timer->start();

    ui->pushButton_play->hide();
    ui->pushButton_pause->show();

    //
    setAudioEnable(m_sid);
}

void LiveViewPlayback::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        on_pushButton_close_clicked();
    }
    BaseDialog::mousePressEvent(event);
}

void LiveViewPlayback::showEvent(QShowEvent *event)
{
    showMaximized();
    BaseDialog::showEvent(event);
}

void LiveViewPlayback::hideEvent(QHideEvent *event)
{
    BaseDialog::hideEvent(event);
}

bool LiveViewPlayback::isAddToVisibleList()
{
    return true;
}

void LiveViewPlayback::escapePressed()
{
    on_pushButton_close_clicked();
}

void LiveViewPlayback::showNoResource(bool show)
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

void LiveViewPlayback::setLivePlayback()
{
    struct live_pb_info2 pb_info;
    memset(&pb_info, 0, sizeof(pb_info));
    pb_info.enable = 1;
    pb_info.chnId = m_channel;
    pb_info.shsid = -1;
    pb_info.pbsid = -1;
    pb_info.winid = m_winId;
    pb_info.stZone.enMode = ZONE_MODE_USER;
    pb_info.stZone.x = m_nvrRect.left();
    pb_info.stZone.y = m_nvrRect.top();
    pb_info.stZone.w = m_nvrRect.width();
    pb_info.stZone.h = m_nvrRect.height();
    qDebug() << QString("REQUEST_FLAG_SET_LIVE_PLAYBACK, channel: %1, %2").arg(m_channel).arg(VapiWinIdString(m_winId)) << m_nvrRect;
    sendMessage(REQUEST_FLAG_SET_LIVE_PLAYBACK, (void *)&pb_info, sizeof(pb_info));

    gMsMessage.refreshAllCameraStatus(1000);
}

void LiveViewPlayback::stopLivePlayback()
{
    if (m_sid < 0) {
        return;
    }

    struct live_pb_info2 pb_info;
    memset(&pb_info, 0, sizeof(pb_info));
    pb_info.enable = 0;
    pb_info.chnId = m_channel;
    pb_info.shsid = m_sid;
    pb_info.pbsid = m_sid;
    pb_info.winid = m_winId;
    pb_info.stZone.enMode = ZONE_MODE_USER;
    pb_info.stZone.x = m_nvrRect.left();
    pb_info.stZone.y = m_nvrRect.top();
    pb_info.stZone.w = m_nvrRect.width();
    pb_info.stZone.h = m_nvrRect.height();

    StreamKey key;
    key.currentChannel(m_channel);
    key.setScreen(WIN_SCREEN(m_winId));
    int result = CustomLayoutData::instance()->streamType(key);
    switch (result) {
    case STREAM_TYPE_MAINSTREAM:
        pb_info.avtype = AV_TYPE_MAIN;
        break;
    case STREAM_TYPE_SUBSTREAM:
        pb_info.avtype = AV_TYPE_SUB;
        break;
    default:
        break;
    }

    //停止播放没有回复
    sendMessageOnly(REQUEST_FLAG_SET_LIVE_PLAYBACK, (void *)&pb_info, sizeof(pb_info));

    setAudioEnable(-1);

    gMsMessage.refreshAllCameraStatus(1000);
}

void LiveViewPlayback::seekLivePlayback(const QDateTime &dateTime)
{
    struct req_seek_time seek_time;
    memset(&seek_time, 0, sizeof(struct req_seek_time));
    seek_time.chnid = m_channel;
    seek_time.sid = m_sid;
    snprintf(seek_time.pseektime, sizeof(seek_time.pseektime), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    sendMessage(REQUEST_FLAG_PLAYSEEK_BACKUP, (void *)&seek_time, sizeof(struct req_seek_time));
}

void LiveViewPlayback::pauseLivePlayback()
{
    m_timer->stop();
    sendMessage(REQUEST_FLAG_PLAYPAUSE_COM_BACKUP, (void *)&(m_sid), sizeof(int));
}

void LiveViewPlayback::resumeLivePlayback()
{
    m_timer->start();
    sendMessage(REQUEST_FLAG_PLAYRESTART_COM_BACKUP, (void *)&(m_sid), sizeof(int));
}

void LiveViewPlayback::immediatelyUpdatePlayTime()
{
    ui->slider_time->setCurrentDateTime(m_currentDateTime);
    ui->label_time->setText(m_currentDateTime.toString("yyyy-MM-dd HH:mm:ss"));
}

void LiveViewPlayback::setAudioEnable(int sid)
{
    struct req_set_audiochn audio;
    memset(&audio, 0, sizeof(struct req_set_audiochn));
    audio.chn = sid;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, &audio, sizeof(audio));
}

void LiveViewPlayback::onLanguageChanged()
{
    ui->pushButton_play->setToolTip(GET_TEXT("PLAYBACK/80089", "Play"));
    ui->pushButton_pause->setToolTip(GET_TEXT("COMMONBACKUP/100046", "Pause "));
    ui->pushButton_close->setToolTip(GET_TEXT("PLAYBACK/80090", "Close"));
}

void LiveViewPlayback::onSliderValueChanged(int value)
{
    m_currentDateTime = QDateTime::fromTime_t(value);
    ui->label_time->setText(m_currentDateTime.toString("yyyy-MM-dd HH:mm:ss"));
    seekLivePlayback(m_currentDateTime);
}

void LiveViewPlayback::onRealPlaybackTime()
{
    m_currentDateTime = m_currentDateTime.addSecs(1);
    if (m_currentDateTime > m_endDateTime) {
        qDebug() << m_currentDateTime << m_endDateTime;
        //finished
        on_pushButton_pause_clicked();
        return;
    }

    bool isValid = false;
    for (int i = 0; i < m_listCommonBackup.size(); ++i) {
        const resp_search_common_backup &backup = m_listCommonBackup.at(i);
        QDateTime beginDateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        QDateTime endDateTime = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
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

void LiveViewPlayback::on_pushButton_play_clicked()
{
    ui->pushButton_play->hide();
    ui->pushButton_pause->show();

    if (m_currentDateTime > m_endDateTime) {
        //play from start time
        m_currentDateTime = m_startDateTime;
        immediatelyUpdatePlayTime();
        seekLivePlayback(m_currentDateTime);
    }
    resumeLivePlayback();
}

void LiveViewPlayback::on_pushButton_pause_clicked()
{
    ui->pushButton_play->show();
    ui->pushButton_pause->hide();

    pauseLivePlayback();
}

void LiveViewPlayback::on_pushButton_close_clicked()
{
    m_timer->stop();
    stopLivePlayback();
    close();
}

void LiveViewPlayback::onNoResourceChanged(int winid, int bNoResource)
{
    if (winid == m_winId) {
        showNoResource(bNoResource);
    }
}
