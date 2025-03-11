#include "LiveVideo.h"
#include "ui_LiveVideo.h"
#include "BottomBar.h"
#include "CameraData.h"
#include "CustomLayoutData.h"
#include "DisplaySetting.h"
#include "DynamicDisplayData.h"
#include "EventDetectionRegionManager.h"
#include "LiveView.h"
#include "LogoutChannel.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "RunnableGetDatabaseCamera.h"
#include "SubControl.h"
#include "TargetInfoManager.h"
#include "VideoContainer.h"
#include "centralmessage.h"
#include "mainwindow.h"
#include "msuser.h"
#include "settingcontent.h"
#include <QMouseEvent>
#include <QPainter>
#include <QThreadPool>

extern "C" {
#include "recortsp.h"
}

QDebug operator<<(QDebug dbg, LiveVideo *video)
{
    QString text = QString("LiveVideo(channel:%1)")
                       .arg(video->channel());
    dbg << text << VapiWinIdString(video->vapiWinId());
    return dbg.space();
}

QColor LiveVideo::s_infoColor;
bool LiveVideo::s_showStreamInfo = false;
bool LiveVideo::s_showChannelName = false;
bool LiveVideo::s_showBorder = false;
int LiveVideo::s_showChannelNameFontSize = 0;

QMap<int, LiveVideo::State> LiveVideo::s_mapVideoState;
QMap<int, bool> LiveVideo::s_mapAnprState;
QMap<int, bool> LiveVideo::s_mapSmartEventState;
QMap<int, bool> LiveVideo::s_mapMotionState;
QMap<int, bool> LiveVideo::s_mapRecordState;
QMap<int, bool> LiveVideo::s_mapVideoLossState;
QMap<int, bool> LiveVideo::s_mapNoResourceState;
QMap<int, resp_camera_status> LiveVideo::s_mapStreamInfo;

bool LiveVideo::s_isDragging = false;

enum {
    CHECK_EVENT_LOOP = QEvent::User + 1
};

LiveVideo::LiveVideo(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::LiveVideo)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    m_iconSize.setWidth(21);
    m_iconSize.setHeight(21);

    m_channel = -1;

    showVideoAdd(false);

    updateDisplayInfo();

    connect(ui->toolButton_add, SIGNAL(clicked(bool)), this, SLOT(onToolButtonAddClicked()));

    //
    m_doubleClickTimer = new QTimer(this);
    connect(m_doubleClickTimer, SIGNAL(timeout()), this, SLOT(onDoubleClickTimer()));
    m_doubleClickTimer->setSingleShot(true);
    m_doubleClickTimer->setInterval(1000);

    ui->label_anpr->hide();
    ui->widget_info->hide();

    //
    connect(qMsNvr, SIGNAL(noResourceChanged(int, int)), this, SLOT(onNoResourceChanged(int, int)));
    //
    if (!m_drawView) {
        m_drawView = new DrawView(this);
        m_drawView->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    if (!m_videoScene) {
        m_videoScene = new VideoScene(this);
        m_drawView->setScene(m_videoScene);
    }
    //
    connect(&gTargetInfoManager, SIGNAL(vcaAlarm(MS_VCA_ALARM)), this, SLOT(onVcaAlarm(MS_VCA_ALARM)));

    //track
    m_timerVcaTrackCheck = new QTimer(this);
    connect(m_timerVcaTrackCheck, SIGNAL(timeout()), this, SLOT(onVcaTrackCheck()));
    m_timerVcaTrackCheck->setSingleShot(true);
    m_timerVcaTrackCheck->setInterval(2000);
    m_timerPtzTrackCheck = new QTimer(this);
    connect(m_timerPtzTrackCheck, SIGNAL(timeout()), this, SLOT(onPtzTrackCheck()));
    m_timerPtzTrackCheck->setSingleShot(2000);
    m_timerPtzTrackCheck->setInterval(2000);

    //pos
    connect(&gDynamicDisplayData, SIGNAL(posDataReceived(PosData)), this, SLOT(onPosData(PosData)));

    //
    ui->labelMessage->hide();
    connect(&gCameraData, SIGNAL(cameraFacePrivacyState(int, int)), this, SLOT(onCameraFacePrivacyState(int, int)));

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

LiveVideo::~LiveVideo()
{
    delete ui;
}

void LiveVideo::setAnprState(int channel, bool state)
{
    s_mapAnprState.insert(channel, state);
}

void LiveVideo::setSmartEventState(int channel, bool state)
{
    s_mapSmartEventState.insert(channel, state);
}

void LiveVideo::setMotionState(int channel, bool state)
{
    s_mapMotionState.insert(channel, state);
}

void LiveVideo::setRecordState(int channel, bool state)
{
    s_mapRecordState.insert(channel, state);
}

void LiveVideo::setVideoLossState(int channel, bool state)
{
    s_mapVideoLossState.insert(channel, state);
}

bool LiveVideo::isVideoLoss(int channel)
{
    return s_mapVideoLossState.value(channel, false);
}

void LiveVideo::setNoResourceState(int winid, bool state)
{
    s_mapNoResourceState.insert(winid, state);
}

void LiveVideo::clearStreamInfo()
{
    s_mapStreamInfo.clear();
}

void LiveVideo::setStreamInfo(int channel, const resp_camera_status &info)
{
    bool isConnect = false;

    switch (info.stream_type) {
    case STREAM_TYPE_MAINSTREAM: {
        const stream_status &status = info.status[STREAM_TYPE_MAINSTREAM];
        if (status.connection == RTSP_CLIENT_CONNECT) {
            isConnect = true;
        }
        break;
    }
    case STREAM_TYPE_SUBSTREAM: {
        const stream_status &status = info.status[STREAM_TYPE_SUBSTREAM];
        if (status.connection == RTSP_CLIENT_CONNECT) {
            isConnect = true;
        }
        break;
    }
    default:
        break;
    }

    if (!isConnect) {
        setAnprState(channel, false);
        setSmartEventState(channel, false);
        setMotionState(channel, false);
    }

    //
    s_mapStreamInfo.insert(channel, info);
}

resp_camera_status LiveVideo::streamStatus(int channel)
{
    if (s_mapStreamInfo.contains(channel)) {
        return s_mapStreamInfo.value(channel);
    } else {
        resp_camera_status status;
        status.chnid = -1;
        return status;
    }
}

int LiveVideo::streamFormat()
{
    if (m_channel >= 0 && s_mapStreamInfo.contains(m_channel) && isVapiWinIdValid()) {
        int stream_type = STREAM_TYPE_MAINSTREAM;
        return stream_type;
    } else {
        return -1;
    }
}

bool LiveVideo::isConnected(int channel)
{
    bool isConnect = false;

    if (s_mapStreamInfo.contains(channel)) {
        const resp_camera_status &camera_status = s_mapStreamInfo.value(channel);
        switch (camera_status.stream_type) {
        case STREAM_TYPE_MAINSTREAM: {
            const stream_status &status = camera_status.status[STREAM_TYPE_MAINSTREAM];
            if (status.connection == RTSP_CLIENT_CONNECT) {
                isConnect = true;
            }
            break;
        }
        case STREAM_TYPE_SUBSTREAM: {
            const stream_status &status = camera_status.status[STREAM_TYPE_SUBSTREAM];
            if (status.connection == RTSP_CLIENT_CONNECT) {
                isConnect = true;
            }
            break;
        }
        default:
            break;
        }
    }
    return isConnect;
}

QRect LiveVideo::globalGeometry() const
{
    return QRect(mapToGlobal(QPoint(0, 0)), size());
}

int LiveVideo::channel() const
{
    return m_channel;
}

void LiveVideo::setChannel(int channel)
{
    m_channel = channel;
    showChannelName();
    //
    m_videoScene->setChannel(m_channel);
}

int LiveVideo::screen() const
{
    int value = -1;
    if (m_container) {
        value = m_container->screen();
    }
    return value;
}

void LiveVideo::setContainer(VideoContainer *container)
{
    m_container = container;
}

void LiveVideo::clearContainer()
{
    m_container = nullptr;
}

int LiveVideo::layoutIndex() const
{
    if (m_container) {
        return m_container->index();
    } else {
        return -1;
    }
}

int LiveVideo::vapiWinId() const
{
    if (m_container) {
        return m_container->vapiWinId();
    } else {
        return -1;
    }
}

bool LiveVideo::isVapiWinIdValid() const
{
    return vapiWinId() >= 0;
}

void LiveVideo::setVideoDragEnable(bool enable)
{
    m_dragEnable = enable;
}

/**
 * @brief LiveVideo::isChannelEnable
 * @return
 */
bool LiveVideo::isChannelEnable() const
{
    return gCameraData.isCameraEnable(channel());
}

/**
 * @brief LiveVideo::isChannelValid
 * @return false: 通道超出最大通道数
 */
bool LiveVideo::isChannelValid() const
{
    static int maxDevice = qMsNvr->maxChannel();
    if (m_channel >= maxDevice || m_channel < 0) {
        return false;
    } else {
        return true;
    }
}

bool LiveVideo::isVideoLoss() const
{
    if (!s_mapVideoLossState.contains(m_channel)) {
        return false;
    }
    bool isVideoLoss = s_mapVideoLossState.value(m_channel);
    return isVideoLoss;
}

bool LiveVideo::isConnected() const
{
    return gCameraData.isCameraConnected(channel());
}

void LiveVideo::updateStreamInfo()
{
    showChannelName();

    //
    bool showStreamInfo = true;

    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        ui->widget_info->setVisible(false);
        return;
    }
    if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel())) {
        ui->widget_info->setVisible(false);
        return;
    }

    QString bitRate;
    QString frameRate;
    QString frameSize;
    bool isConnect = false;

    if (!s_showStreamInfo) {
        showStreamInfo = false;
    }
    if (m_channel >= 0 && s_mapStreamInfo.contains(m_channel) && isVapiWinIdValid()) {
        int frameWidth = 0;
        int frameHeight = 0;
        int codec = 0;
        const resp_camera_status &camera_status = s_mapStreamInfo.value(m_channel);
        int stream_type = STREAM_TYPE_MAINSTREAM;
        switch (stream_type) {
        case STREAM_TYPE_MAINSTREAM: {
            const stream_status &status = camera_status.status[STREAM_TYPE_MAINSTREAM];
            bitRate = QString("%1: %2Kbps").arg(GET_TEXT("CHANNELMANAGE/30060", "Bit Rate")).arg(status.bit_rate / 1000);
            frameRate = QString("%1: %2fps").arg(GET_TEXT("CHANNELMANAGE/30066", "Frame Rate")).arg(status.frame_rate);
            frameSize = QString("%1: %2*%3").arg(GET_TEXT("CHANNELMANAGE/30058", "Frame Size")).arg(status.cur_res.width).arg(status.cur_res.height);
            frameWidth = status.cur_res.width;
            frameHeight = status.cur_res.height;
            codec = status.codec;
            if (status.connection == RTSP_CLIENT_CONNECT) {
                isConnect = true;
            }
            break;
        }
        case STREAM_TYPE_SUBSTREAM: {
            const stream_status &status = camera_status.status[STREAM_TYPE_SUBSTREAM];
            bitRate = QString("%1: %2Kbps").arg(GET_TEXT("CHANNELMANAGE/30060", "Bit Rate")).arg(status.bit_rate / 1000);
            frameRate = QString("%1: %2fps").arg(GET_TEXT("CHANNELMANAGE/30066", "Frame Rate")).arg(status.frame_rate);
            frameSize = QString("%1: %2*%3").arg(GET_TEXT("CHANNELMANAGE/30058", "Frame Size")).arg(status.cur_res.width).arg(status.cur_res.height);
            frameWidth = status.cur_res.width;
            frameHeight = status.cur_res.height;
            codec = status.codec;
            if (status.connection == RTSP_CLIENT_CONNECT) {
                isConnect = true;
            }
            break;
        }
        default:
            break;
        }
        //
        if (frameWidth != m_frameWidth || frameHeight != m_frameHeight) {
            m_frameWidth = frameWidth;
            m_frameHeight = frameHeight;
            adjustDrawView();
        }
        if (m_codec != codec) {
            m_codec = codec;
        }
        //m_cameraInfo.enable = true;
        setRecordState(camera_status.chnid, camera_status.record);
        setVideoLossState(m_channel, !isConnect);
    } else {
        //m_cameraInfo.enable = false;
        showStreamInfo = false;
        setRecordState(m_channel, false);
    }
    if (!isConnect) {
        showStreamInfo = false;
    }

    //
    ui->label_bitRate->setText(bitRate);
    ui->label_frameRate->setText(frameRate);
    ui->label_frameSize->setText(frameSize);
    ui->widget_info->setVisible(showStreamInfo);
    adjustStreamInfo();

    //
    updateRecord();
    updateMotion();
    updateSmartEvent();
    updateAnprEvent();
    //
    updateVideoLoss();
    //
    showNoResource();
    //恢复toolbar为默认
    if (!isConnected()) {
        s_mapVideoState[m_channel].isAudio = false;
        s_mapVideoState[m_channel].isEmergencyRecording = false;

        qMsNvr->closeTalkback(m_channel);
        qMsNvr->closeLiveviewAudio(m_channel);
        updateAudioState();
        updateTalkState();
    }
}

void LiveVideo::updateAnprEvent()
{
    if (!isChannelValid() || !isChannelEnable()) {
        ui->label_anpr->setVisible(false);
        return;
    }
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        ui->label_anpr->setVisible(false);
        return;
    }
    if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel())) {
        ui->label_anpr->setVisible(false);
        return;
    }
    if (!s_mapAnprState.contains(m_channel)) {
        ui->label_anpr->setVisible(false);
        return;
    }
    if (!gCameraData.isCameraConnected(channel())) {
        ui->label_anpr->setVisible(false);
        return;
    }
    bool isAnprEvent = s_mapAnprState.value(m_channel);
    ui->label_anpr->setVisible(isAnprEvent);
}

void LiveVideo::updateSmartEvent()
{
    if (!isChannelValid() || !isChannelEnable()) {
        ui->label_smartEvent->setVisible(false);
        return;
    }
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        ui->label_smartEvent->setVisible(false);
        return;
    }
    if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel())) {
        ui->label_smartEvent->setVisible(false);
        return;
    }
    if (!s_mapSmartEventState.contains(m_channel)) {
        ui->label_smartEvent->setVisible(false);
        return;
    }
    if (!gCameraData.isCameraConnected(channel())) {
        ui->label_smartEvent->setVisible(false);
        return;
    }
    bool isSmartEvent = s_mapSmartEventState.value(m_channel);
    ui->label_smartEvent->setVisible(isSmartEvent);
}

void LiveVideo::updateMotion()
{
    if (!isChannelValid() || !isChannelEnable()) {
        ui->label_motion->setVisible(false);
        return;
    }
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        ui->label_motion->setVisible(false);
        return;
    }
    if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel())) {
        ui->label_motion->setVisible(false);
        return;
    }
    if (!s_mapMotionState.contains(m_channel)) {
        ui->label_motion->setVisible(false);
        return;
    }
    if (!gCameraData.isCameraConnected(channel())) {
        ui->label_motion->setVisible(false);
        return;
    }
    bool isMotion = s_mapMotionState.value(m_channel);
    ui->label_motion->setVisible(isMotion);
}

void LiveVideo::updateRecord()
{
    if (!isChannelValid() || !isChannelEnable()) {
        ui->label_record->setVisible(false);
        return;
    }
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        ui->label_record->setVisible(false);
        return;
    }
    if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel())) {
        ui->label_record->setVisible(false);
        return;
    }
    if (!s_mapRecordState.contains(m_channel)) {
        ui->label_record->setVisible(false);
        return;
    }
    bool isRecord = s_mapRecordState.value(m_channel);
    ui->label_record->setVisible(isRecord);
}

void LiveVideo::updateVideoLoss()
{
    if (!isChannelValid() || !isChannelEnable()) {
        ui->widget_disconnected->setVisible(false);
        return;
    }
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        ui->widget_disconnected->setVisible(false);
        return;
    }
    if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel())) {
        ui->widget_disconnected->setVisible(false);
        return;
    }
    if (!s_mapVideoLossState.contains(m_channel)) {
        ui->widget_disconnected->setVisible(false);
        return;
    }
    bool isLoss = s_mapVideoLossState.value(m_channel);
    ui->widget_disconnected->setVisible(isLoss);

    ui->label_channelName->setText(gCameraData.cameraName(channel()));
    //DDNS添加的显示DDNS域名
    QString strDDNS = gCameraData.cameraDDNS(channel());
    if (strDDNS.isEmpty()) {
        ui->label_ip->setText(gCameraData.cameraIPv4Address(channel()));
    } else {
        ui->label_ip->setText(strDDNS);
    }

    //避免断开和no resource同时显示
    if (isLoss) {
        ui->label_noResource->setVisible(false);
        ui->labelMessage->setVisible(false);
        //
        m_drawView->hide();
    } else {
        showNoResource();
        //
        m_drawView->show();
    }
}

void LiveVideo::showNoResource()
{
    if (!isChannelValid() || !isChannelEnable()) {
        ui->label_noResource->setVisible(false);
        return;
    }
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        ui->label_noResource->setVisible(false);
        return;
    }
    if (LogoutChannel::instance()->instance()->isLogout() && !LogoutChannel::instance()->instance()->isLogoutChannel(channel())) {
        ui->label_noResource->setVisible(false);
        return;
    }
    //避免断开和no resource同时显示
    if (ui->widget_disconnected->isVisible()) {
        ui->label_noResource->setVisible(false);
        return;
    }
    //
    bool show = false;
    int scaleDown = 15;
    #if defined(_HI3536A_)
    RATIO_E ratio = videoRatio(channel(), screen());
    if (ratio == RATIO_VO_FULL) {
      scaleDown = 30;
    }
    #endif
    if (double(m_frameWidth) / width() > scaleDown || double(m_frameHeight) / height() > scaleDown) {
        qMsCDebug("qt_noresource") << QString("frame: %1x%2, widget: %3x%4").arg(m_frameWidth).arg(m_frameHeight).arg(width()).arg(height());
        show = true;
    } else {
        if (s_mapNoResourceState.contains(vapiWinId())) {
            show = s_mapNoResourceState.value(vapiWinId());
        }
    }
    QString noresource = QString(GET_TEXT("LIVEVIEW/20031", "No Resource"));
#if defined(_NT98323_) || defined (_HI3536C_) || defined (_HI3798_)
    if (m_frameWidth > 4000 || m_frameHeight > 4000) {
        show = true;
    }
#elif defined (_HI3536G_)
    if ((m_frameWidth > 4000 || m_frameHeight > 4000) && m_codec == CODECTYPE_H265) {
        show = true;
    }
#endif
    //
    if (show) {
        ui->label_noResource->setText(GET_TEXT("LIVEVIEW/20031", "No Resource"));
    } else {
        if (m_isFacePrivacyEnable) {
            ui->labelMessage->show();
        }
    }
    ui->label_noResource->setVisible(show);
}

void LiveVideo::updateTalkState()
{
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        ui->label_talk->setVisible(false);
        return;
    }
    if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel())) {
        ui->label_talk->setVisible(false);
        return;
    }
    if (qMsNvr->talkbackChannel() == m_channel && m_channel >= 0) {
        //MsDebug() << "show talkback, channel:" << m_channel;
        ui->label_talk->show();
    } else {
        //MsDebug() << "hide talkback, channel:" << m_channel;
        ui->label_talk->hide();
    }
}

void LiveVideo::updateAudioState()
{
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        ui->label_audio->setVisible(false);
        return;
    }
    if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel())) {
        ui->label_audio->setVisible(false);
        return;
    }
    if (qMsNvr->liveviewAudioChannel() == m_channel && m_channel >= 0) {
        //MsDebug() << "show audio, channel:" << m_channel;
        ui->label_audio->show();
    } else {
        //MsDebug() << "hide audio, channel:" << m_channel;
        ui->label_audio->hide();
    }
}

void LiveVideo::updateVideoScene()
{
    if (m_videoScene) {
        if (isConnected()) {
            m_videoScene->onVideoShow();
        } else {
            m_videoScene->onVideoHide();
        }
    }
}

void LiveVideo::showVideoAdd(bool show)
{
    adjustVideoAdd();
    if (qMsNvr->isSlaveMode()) {
        ui->toolButton_add->setVisible(false);
    } else {
        ui->toolButton_add->setVisible(show);
    }
}

void LiveVideo::showChannelName()
{
    bool show = true;
    if (!isChannelValid() || !isChannelEnable()) {
        show = false;
    }
    if (!s_showChannelName) {
        show = false;
    }

    if (show) {
        QFont font = ui->label_name->font();
        font.setPixelSize(s_showChannelNameFontSize);
        ui->label_name->setFont(font);
        //自动换行
        int videoWidth = this->width() - (m_iconSize.width() * 2);
        QString strText = gCameraData.cameraName(channel());
        QString strName;
        ui->label_name->setText("");
        for (int i = 0; i < strText.size(); i++) {
            int textWidth = QFontMetrics(font).width(strName + strText[i]);
            if (textWidth >= videoWidth) {
                strName += "\n";
                ui->label_name->setText(ui->label_name->text() + strName);
                strName = "";
            }
            strName += strText[i];
        }
        ui->label_name->setText(ui->label_name->text() + strName);
        ui->widgetChannelName->adjustSize();
        ui->widgetChannelName->move(m_iconSize.width(), m_iconSize.height());
    }
    ui->widgetChannelName->setVisible(show);
}

QSize LiveVideo::mainStreamFrameSize() const
{
    QSize frameSize(0, 0);
    if (m_channel >= 0 && s_mapStreamInfo.contains(m_channel) && isVapiWinIdValid()) {
        const resp_camera_status &camera_status = s_mapStreamInfo.value(m_channel);
        const stream_status &status = camera_status.status[STREAM_TYPE_MAINSTREAM];
        frameSize.setWidth(status.cur_res.width);
        frameSize.setHeight(status.cur_res.height);
    }
    return frameSize;
}

QRect LiveVideo::videoFrameRect() const
{
    QRect videoRect;
    RATIO_E ratio = videoRatio(channel(), m_container->screen());
    switch (ratio) {
    case RATIO_VO_FULL: {
        videoRect = rect();
        break;
    }
    case RATIO_VO_AUTO: {
        QRect rc = rect();
        qreal ratio = (qreal)m_frameWidth / m_frameHeight;
        if ((qreal)rc.width() / rc.height() > ratio) {
            rc.setWidth(rc.height() * ratio);
        } else {
            rc.setHeight(rc.width() / ratio);
        }
        rc.moveCenter(rect().center());
        videoRect = rc;
        break;
    }
    case RATIO_VO_MANUAL: {
        break;
    }
    default:
        break;
    }
    return videoRect;
}

void LiveVideo::clearPos()
{
    for (auto iter = m_posTextItemMap.constBegin(); iter != m_posTextItemMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->clear();
    }
}

void LiveVideo::adjustChannelName()
{
    ui->widgetChannelName->adjustSize();
    ui->widgetChannelName->move(m_iconSize.width(), m_iconSize.height());
}

LiveVideo::State LiveVideo::state(int channel)
{
    return s_mapVideoState.value(channel);
}

void LiveVideo::setEmergencyRecord(int channel, bool enable)
{
    s_mapVideoState[channel].isEmergencyRecording = enable;
}

void LiveVideo::setAudio(int channel, bool enable)
{
    for (auto iter = s_mapVideoState.begin(); iter != s_mapVideoState.end(); ++iter) {
        LiveVideo::State &state = iter.value();
        state.isAudio = false;
    }
    s_mapVideoState[channel].isAudio = enable;
}

bool LiveVideo::audioState(int channel)
{
    if (s_mapVideoState.contains(channel)) {
        return s_mapVideoState.value(channel).isAudio;
    } else {
        return false;
    }
}

void LiveVideo::setVideoRatio(int channel, int screen, const RATIO_E &ratio)
{
    switch (screen) {
    case SCREEN_MAIN:
        s_mapVideoState[channel].ratioMain = ratio;
        break;
    case SCREEN_SUB:
        s_mapVideoState[channel].ratioSub = ratio;
        break;
    }
}

RATIO_E LiveVideo::videoRatio(int channel, int screen)
{
    if (s_mapVideoState.contains(channel)) {
        switch (screen) {
        case SCREEN_MAIN:
            return s_mapVideoState.value(channel).ratioMain;
        case SCREEN_SUB:
            return s_mapVideoState.value(channel).ratioSub;
        }
    }
    return RATIO_VO_FULL;
}

void LiveVideo::setVideoRatio(int screen, const RATIO_E &ratio)
{
    setVideoRatio(channel(), screen, ratio);
    adjustDrawView();
}

bool LiveVideo::isDraging()
{
    return s_isDragging;
}

void LiveVideo::setDisplayInfo(const display &display_info)
{
    s_infoColor = DisplaySetting::s_displayColor;
    s_showStreamInfo = display_info.camera_info;
    s_showChannelName = display_info.show_channel_name;
    s_showBorder = display_info.border_line_on;
    switch (display_info.fontSize) {
    case FONT_SIZE_SMALL:
        s_showChannelNameFontSize = 14;
        break;
    case FONT_SIZE_MEDIUM:
        s_showChannelNameFontSize = 18;
        break;
    case FONT_SIZE_LARGE:
        s_showChannelNameFontSize = 22;
        break;
    default:
        s_showChannelNameFontSize = 14;
        break;
    }
}

void LiveVideo::updateDisplayInfo()
{
    ui->label_name->setStyleSheet(QString("color: %1").arg(s_infoColor.name()));
    ui->widget_info->setStyleSheet(QString("color: %1").arg(s_infoColor.name()));

    //边框
    update();
    //
    updateStreamInfo();
    //
    showChannelName();
}

void LiveVideo::resizeEvent(QResizeEvent *event)
{
    adjustChannelName();
    adjustAnprEvent();
    adjustSmartEvent();
    adjustMotion();
    adjustRecord();
    adjustVideoLoss();
    adjustVideoAdd();
    adjustStreamInfo();
    adjustNoResource();
    adjustMessage();

    ui->labelNoPermission->setGeometry(rect());
    ui->widgetLock->setGeometry(rect());

    m_drawView->setGeometry(rect());

    QWidget::resizeEvent(event);

    //
    adjustAll();
}

void LiveVideo::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    //
    bool showVideoInfo = true;
    //permission
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        showVideoInfo = false;
        //
        ui->labelNoPermission->setGeometry(rect());
        ui->labelNoPermission->setText(GET_TEXT("LIVEVIEW/20109", "No Permission"));
        ui->labelNoPermission->show();
    } else {
        ui->labelNoPermission->hide();
        if (m_isFacePrivacyEnable) {
            ui->labelMessage->show();
        }
    }
    //logout
    if (LogoutChannel::instance()->isLogout()) {
        if (LogoutChannel::instance()->isLogoutChannel(channel())) {
            ui->widgetLock->hide();
        } else {
            showVideoInfo = false;
            //
            if (!ui->labelNoPermission->isVisible() && channel() != -1) {
                ui->labelMessage->hide();
                ui->widgetLock->setGeometry(rect());
                ui->widgetLock->show();
            }
        }
    } else {
        ui->widgetLock->hide();
        if (m_isFacePrivacyEnable) {
            ui->labelMessage->show();
        }
    }
//软解fisheye Mode 不显示检测框
#ifdef MS_FISHEYE_SOFT_DEWARP
    if (LiveView::instance()->BottomBarMode() == BottomBar::ModeFisheye) {
        showVideoInfo = false;
    }
#endif
    //
    if (showVideoInfo) {
        //这里不再立即刷新stream info，避免更新发生在在vapi即将改变之前
        //相关bug，MSHN-9154 QT-Live View：鱼眼Multi-Stream添加的机子，双击全屏后，会出现断开连接提示，然后消失。添加正常机子并无该现象
        //updateStreamInfo();
        updateAnprEvent();
        updateSmartEvent();
        updateMotion();
        updateRecord();
        updateVideoLoss();
        showNoResource();
        updateTalkState();
        updateAudioState();
        if (CustomLayoutData::instance()->isSingleLayout(LiveView::instance()->currentLayoutMode())) {
            checkHumanDetection();
            checkPtzTracking();
        }
        //
        QTimer::singleShot(0, this, SLOT(onDrawViewShow()));
    } else {
        ui->widget_info->setVisible(false);
        ui->label_anpr->setVisible(false);
        ui->label_smartEvent->setVisible(false);
        ui->label_motion->setVisible(false);
        ui->label_record->setVisible(false);
        ui->widget_disconnected->setVisible(false);
        ui->label_noResource->setVisible(false);
        ui->label_talk->setVisible(false);
        ui->label_audio->setVisible(false);
    }

    //
    adjustAll();
}

void LiveVideo::hideEvent(QHideEvent *event)
{
    showVideoAdd(false);
    closeHumanDetection();
    closePtzTracking();
    closeFisheyeTracking();
    setNoResourceState(vapiWinId(), false);
    //
    EventDetectionRegionManager::instance()->unregisterVideo(this);
    m_drawView->hide();
    m_videoScene->onVideoHide();
    //
    QWidget::hideEvent(event);
}

void LiveVideo::mousePressEvent(QMouseEvent *event)
{
    qMsCDebug("qt_camera_press") << this << channel() << VapiWinIdString(vapiWinId());

    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (!m_isDoubleClicked) {
            m_pressed = true;
            m_pressedPoint = event->pos();
            s_isDragging = true;

            emit mouseClicked(channel());
        }
    }
    resetAndCheckEventLoop();
    m_dragTimer.restart();
}

void LiveVideo::mouseReleaseEvent(QMouseEvent *)
{
    qMsCDebug("qt_camera_press") << this << channel() << VapiWinIdString(vapiWinId());

    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        return;
    }

    if (m_pressed) {
        m_pressed = false;
        s_isDragging = false;
    }
}

void LiveVideo::mouseDoubleClickEvent(QMouseEvent *event)
{
    qMsCDebug("qt_camera_press") << this << channel() << VapiWinIdString(vapiWinId());

    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        return;
    }

    if (event->button() != Qt::LeftButton)
        return;

    if (!isEventLoopStillNormal(1))
        return;

    //double click发生在两次mouse click之间，抖动经常会导致发生拖动操作，这里延时一秒再去判断mouse press
    m_isDoubleClicked = true;
    m_doubleClickTimer->start();

    emit mouseDoubleClicked(m_channel);
}

void LiveVideo::mouseMoveEvent(QMouseEvent *event)
{
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        return;
    }
    if (!m_dragEnable)
        return;

    if (!m_pressed)
        return;

    if ((event->pos() - m_pressedPoint).manhattanLength() < qApp->startDragDistance())
        return;

    if (!isChannelValid())
        return;

    if (m_dragTimer.elapsed() > 300 && isEventLoopStillNormal(3)) {
        //检查拖拽的参数，有时会有-1的情况
        if (layoutIndex() < 0) {
            qMsWarning() << QString("LiveVideo drag, channel: %1, index: -1").arg(m_channel);
            return;
        }
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setData("channel", QByteArray::number(m_channel));
        mimeData->setData("index", QByteArray::number(layoutIndex()));
        drag->setMimeData(mimeData);

        s_isDragging = true;
        qMsDebug() << this << "exec drag";
        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
        qMsDebug() << this << dropAction;
        if (dropAction == Qt::IgnoreAction) {
        }
        s_isDragging = false;
    }
}

void LiveVideo::dragEnterEvent(QDragEnterEvent *event)
{
    if (!m_dragEnable) {
        return;
    }

    const int &channel = event->mimeData()->data("channel").toInt();
    if (channel == m_channel || m_channel < 0) {
        event->ignore();
    } else {
        event->accept();

        //LiveView::instance()->showSwapVideo(channel, m_channel);
    }
}

void LiveVideo::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)

    if (!m_dragEnable) {
        return;
    }
}

void LiveVideo::dropEvent(QDropEvent *event)
{
    if (!m_dragEnable) {
        return;
    }

    const int &srcChannel = event->mimeData()->data("channel").toInt();
    const int &srcIndex = event->mimeData()->data("index").toInt();

    REQ_EXCHANGE_S exchange;
    memset(&exchange, 0, sizeof(REQ_EXCHANGE_S));
    exchange.srcDevId = srcChannel;
    exchange.srcVoutId = srcIndex;
    exchange.desDevId = channel();
    exchange.desVoutId = layoutIndex();
    exchange.enScreen = SubControl::instance()->currentScreen();
    qMsDebug() << QString("REQUEST_FLAG_EXCHANGE_WINDOW, srcChannel: %1, srcWin: %2, destChannel: %3, destWin: %4")
                      .arg(exchange.srcDevId)
                      .arg(exchange.srcVoutId)
                      .arg(exchange.desDevId)
                      .arg(exchange.desVoutId);
    sendMessageOnly(REQUEST_FLAG_EXCHANGE_WINDOW, (void *)&exchange, sizeof(REQ_EXCHANGE_S));

    LiveView::instance()->swapFinished(exchange.srcVoutId, exchange.srcDevId, exchange.desVoutId, exchange.desDevId);
    qMsDebug() << this << "drop end";
}

bool LiveVideo::event(QEvent *e)
{
    switch (static_cast<int>(e->type())) {
    case CHECK_EVENT_LOOP:
        if (eventLoopCheckCount < 10) {
            eventLoopCheckCount++;
            qApp->postEvent(this, new QEvent(static_cast<QEvent::Type>(CHECK_EVENT_LOOP)));
        }
        return true;
    }
    return QWidget::event(e);
}

void LiveVideo::adjustAll()
{
    adjustChannelName();
    adjustAnprEvent();
    adjustSmartEvent();
    adjustMotion();
    adjustRecord();
    adjustVideoLoss();
    adjustVideoAdd();
    adjustStreamInfo();
    adjustNoResource();

    ui->labelNoPermission->setGeometry(rect());
    ui->widgetLock->setGeometry(rect());

    m_drawView->setGeometry(rect());
}

void LiveVideo::resetAndCheckEventLoop()
{
    qApp->sendPostedEvents();
    eventLoopCheckCount = 0;
    qApp->postEvent(this, new QEvent(static_cast<QEvent::Type>(CHECK_EVENT_LOOP)));
}

bool LiveVideo::isEventLoopStillNormal(int level)
{
    if (Q_UNLIKELY(!qgetenv("LIVEVIDEO_CHECK_EVENT_LOOP").isEmpty())) {
        qDebug() << "eventLoop check count: " << eventLoopCheckCount;
        qDebug() << "eventloop check level: " << level;
    }
    return eventLoopCheckCount >= level;
}

void LiveVideo::adjustAnprEvent()
{
    adjustWidgetIcon();
}

void LiveVideo::adjustSmartEvent()
{
    adjustWidgetIcon();
}

void LiveVideo::adjustMotion()
{
    adjustWidgetIcon();
}

void LiveVideo::adjustRecord()
{
    adjustWidgetIcon();
}

void LiveVideo::adjustVideoLoss()
{
  if (rect().height() <=135) {
        ui->widget_disconnected->layout()->setContentsMargins(0, 0, 0, 0);
        ui->widget_disconnected->layout()->setSpacing(0);
        ui->widget_textBackground->layout()->setContentsMargins(0, 0, 0, 0);
        ui->widget_disconnected->layout()->setSpacing(0);
    } else {
        ui->widget_disconnected->layout()->setContentsMargins(9, 9, 9, 9);
        ui->widget_disconnected->layout()->setSpacing(6);
        ui->widget_textBackground->layout()->setContentsMargins(9, 9, 9, 9);
        ui->widget_disconnected->layout()->setSpacing(6);
    }
    ui->widget_disconnected->setGeometry(rect());
}

void LiveVideo::adjustVideoAdd()
{
    ui->toolButton_add->move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, ui->toolButton_add->size(), rect()).topLeft());
}

void LiveVideo::adjustStreamInfo()
{
    ui->widget_info->adjustSize();
    ui->widget_info->move(width() - ui->widget_info->width() - m_iconSize.width(), height() - ui->widget_info->height() - m_iconSize.height());
}

void LiveVideo::adjustNoResource()
{
    ui->label_noResource->setGeometry(rect());
}

void LiveVideo::adjustDrawView()
{
    if (!m_container) {
        //qMsWarning() << QString("container is nullptr, channel: %1").arg(channel());
        return;
    }

    RATIO_E ratio = videoRatio(channel(), m_container->screen());
    switch (ratio) {
    case RATIO_VO_FULL: {
        m_drawView->setGeometry(rect());
        break;
    }
    case RATIO_VO_AUTO: {
        QRect rc = rect();
        qreal ratio = (qreal)m_frameWidth / m_frameHeight;
        if ((qreal)rc.width() / rc.height() > ratio) {
            rc.setWidth(rc.height() * ratio);
        } else {
            rc.setHeight(rc.width() / ratio);
        }
        rc.moveCenter(rect().center());
        m_drawView->setGeometry(rc);
        break;
    }
    case RATIO_VO_MANUAL: {
        break;
    }
    default:
        break;
    }

    //
    for (auto iter = m_posTextItemMap.constBegin(); iter != m_posTextItemMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->setRect(m_videoScene->sceneRect());
    }
}

void LiveVideo::adjustMessage()
{
    ui->labelMessage->setGeometry(rect());
}

void LiveVideo::adjustWidgetIcon()
{
    QPoint p;
    p.setX(rect().right() - ui->widget_icon->width() - 10);
    p.setY(10);
    ui->widget_icon->move(p);
}

void LiveVideo::onLanguageChanged()
{
    ui->labelMessage->setText(GET_TEXT("LIVEVIEW/20113", "Cannot view in Face Privacy Mode"));
}

void LiveVideo::initializeTrack(TrackMode mode, bool enable)
{
    m_videoScene->setTrackMode(mode, enable);
}

void LiveVideo::checkHumanDetection()
{
    if (!m_container) {
        return;
    }
    if (!isVisible()) {
        return;
    }
    int screen = m_container->screen();
    if (gLiveView->isSingleLayout(screen)) {
        m_timerVcaTrackCheck->start();
    }
}

void LiveVideo::closeHumanDetection()
{
    initializeTrack(TrackModeVca, false);
    m_timerVcaTrackCheck->stop();
}

void LiveVideo::checkPtzTracking()
{
    if (!m_container) {
        return;
    }
    if (!isVisible()) {
        return;
    }
    if (qMsNvr->isFisheye(m_channel)) {
        return;
    }
    int screen = m_container->screen();
    if (gLiveView->isSingleLayout(screen)) {
        m_timerPtzTrackCheck->start();
    }
}

void LiveVideo::closePtzTracking()
{
    initializeTrack(TrackModePtz, false);
    m_timerPtzTrackCheck->stop();
}

void LiveVideo::closeFisheyeTracking()
{
    initializeTrack(TrackModeFisheye, false);
}

void LiveVideo::ON_RESPONSE_FLAG_GET_VCA_HUMANDETECTION(MessageReceive *message)
{
    if (!isVisible()) {
        return;
    }
    bool trackEnable = false;
    ms_smart_event_info *info = (ms_smart_event_info *)message->data;
    if (info) {
        if (info->chanid != channel()) {
            return;
        }
        trackEnable = info->enable && info->show_tracks_enable;
    } else {
        return;
    }

    qMsDebug() << QString("check vca track enable, channel: %1, enable: %2").arg(m_channel).arg(trackEnable);
    initializeTrack(TrackModeVca, trackEnable);
}

void LiveVideo::ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message)
{
    if (!isVisible()) {
        return;
    }
    bool trackEnable = false;
    ms_auto_tracking *auto_tracking = (ms_auto_tracking *)message->data;
    if (auto_tracking) {
        if (auto_tracking->chanid != channel()) {
            return;
        }
        trackEnable = auto_tracking->enable && auto_tracking->show;
    } else {
        return;
    }

    qMsDebug() << QString("check ptz track enable, channel: %1, enable: %2").arg(m_channel).arg(trackEnable);
    if (auto_tracking && qMsNvr->isFisheye(auto_tracking->chanid)) {
        //鱼眼要判断的比较多，只在鱼眼模式显示Track
    } else {
        initializeTrack(TrackModePtz, trackEnable);
    }
}

void LiveVideo::onVcaTrackCheck()
{
    qMsDebug() << QString("check vca track enable, channel: %1").arg(m_channel);
    sendMessage(REQUEST_FLAG_GET_VCA_HUMANDETECTION, &m_channel, sizeof(int));
}

void LiveVideo::onPtzTrackCheck()
{
    qMsDebug() << QString("check ptz track enable, channel: %1").arg(m_channel);
    sendMessage(REQUEST_FLAG_GET_AUTO_TRACKING, &m_channel, sizeof(int));
}

VideoScene *LiveVideo::drawScene()
{
    return m_videoScene;
}

void LiveVideo::dealMessage(MessageReceive *message)
{
    if (m_videoScene) {
        m_videoScene->dealMessage(message);
    }
}

void LiveVideo::dealEventDetectionRegionMessage(MessageReceive *message)
{
    if (m_videoScene) {
        m_videoScene->dealEventDetectionRegionMessage(message);
    }
}

void LiveVideo::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_VCA_HUMANDETECTION:
        ON_RESPONSE_FLAG_GET_VCA_HUMANDETECTION(message);
        break;
    case RESPONSE_FLAG_GET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_GET_AUTO_TRACKING(message);
        break;
    }
}

void LiveVideo::onToolButtonAddClicked()
{
    emit videoAddButtonClicked(m_channel);
}

void LiveVideo::onDoubleClickTimer()
{
    m_isDoubleClicked = false;
}

void LiveVideo::onDragTimer()
{
}

void LiveVideo::onNoResourceChanged(int winid, int bNoResource)
{
    if (winid == vapiWinId()) {
        setNoResourceState(winid, bNoResource);
        showNoResource();
    }
}

void LiveVideo::onDrawViewShow()
{
    //可能显示之后马上又隐藏了
    if (!isVisible()) {
        return;
    }
    EventDetectionRegionManager::instance()->registerVideo(this);
    adjustDrawView();
    m_videoScene->onVideoShow();
}

void LiveVideo::onVcaAlarm(const MS_VCA_ALARM &alarm)
{
    if (alarm.chnid != channel()) {
        return;
    }

    setSmartEventState(alarm.chnid, alarm.alarm);
    updateSmartEvent();
    //
    m_videoScene->setVcaAlarm(alarm);
}

void LiveVideo::onPosData(PosData data)
{
    if (!isVisible()) {
        return;
    }
    if (data.channel() != channel()) {
        return;
    }
    if (LiveViewPlayback::instance() && LiveViewPlayback::instance()->currentChannel() == channel()) {
        if (data.streamFrom() != SST_LOCAL_PB) {
            return;
        }
        if (data.streamFormat() != STREAM_MAIN) {
            return;
        }
    } else {
        if (data.streamFrom() != SST_IPC_STREAM) {
            return;
        }
        if (data.streamFormat() != streamFormat()) {
            return;
        }
    }
    //Best Decoding Performance
    if (SubControl::instance()->subLiveViewScreen() == screen()) {
        return;
    }
    //permission
    if (!gMsUser.hasLiveViewChannelPermission(channel())) {
        return;
    }
    //logout
    if (LogoutChannel::instance()->isLogout()) {
        if (!LogoutChannel::instance()->isLogoutChannel(channel())) {
            return;
        }
    }
    //
    auto *item = m_posTextItemMap.value(data.posId());
    if (!item) {
        item = new GraphicsItemPosText();
        m_videoScene->addItem(item);
        item->setRect(m_videoScene->sceneRect());
        item->setZValue(data.posId());
        m_posTextItemMap.insert(data.posId(), item);
    }
    item->setPosData(data);
}

void LiveVideo::onCameraFacePrivacyState(int chan, int state)
{
    if (channel() != chan) {
        return;
    }

    if (state == 0 || qMsNvr->isNT(chan) || qMsNvr->is3536a()) {
        m_isFacePrivacyEnable = false;
        ui->labelMessage->hide();
    } else {
        m_isFacePrivacyEnable = true;
        if (ui->labelNoPermission->isVisible()) {

        } else if (ui->label_noResource->isVisible()) {

        } else if (ui->widgetLock->isVisible()) {

        } else {
            ui->labelMessage->show();
        }
    }
}
