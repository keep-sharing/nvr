#include "VideoBar.h"
#include "ui_VideoBar.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "SubControl.h"
#include "centralmessage.h"
#include "mainwindow.h"
#include "msuser.h"


VideoBar::VideoBar(QWidget *parent)
    : BaseWidget(parent)
    , ui(new Ui::VideoBar)
{
    ui->setupUi(this);

    m_recordAnimationTimer = new QTimer(this);
    connect(m_recordAnimationTimer, SIGNAL(timeout()), this, SLOT(onRecordAnimation()));
    m_recordAnimationTimer->setInterval(500);

    ui->toolButton_talkback->setVisible(qMsNvr->isSupportTalkback());

    if (qMsNvr->isnt98323() || qMsNvr->isnt98633()) {
        ui->toolButton_ratio->hide();
    }
    if (qMsNvr->isSlaveMode()) {
        ui->toolButton_record->hide();
        ui->toolButton_screenshot->hide();
    }

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

VideoBar::~VideoBar()
{
    delete ui;
}

int VideoBar::realWidth()
{
    return ui->widget_background->width();
}

void VideoBar::showVideoBar(int channel)
{
    m_channel = channel;
    //
    show();
}

void VideoBar::closeVideoBar()
{
    on_toolButton_close_clicked();
}

void VideoBar::updateState(LiveVideo::State state)
{
    m_state = state;
    setEmergencyRecord(m_state.isEmergencyRecording);
    switch (SubControl::instance()->currentScreen()) {
    case SCREEN_MAIN:
        setVideoRatio(m_state.ratioMain);
        break;
    case SCREEN_SUB:
        setVideoRatio(m_state.ratioSub);
        break;
    default:
        break;
    }
    //
    updateAudioState();
    //
    updateTalkState();

    updataStream();
}

void VideoBar::updateAudioState()
{
    if (qMsNvr->liveviewAudioChannel() == m_channel && m_channel >= 0) {
        setAudioState(true);
    } else {
        setAudioState(false);
    }
}

void VideoBar::updateTalkState()
{
    if (qMsNvr->talkbackChannel() == m_channel && m_channel >= 0) {
        setTalkbackButtonState(1);
    } else {
        setTalkbackButtonState(0);
    }
}

void VideoBar::updataStream()
{
    int type = LiveView::instance()->streamInfo();
    setStreamState(type);
}

void VideoBar::recordClicked()
{
    on_toolButton_record_clicked();
}

void VideoBar::audioClicked()
{
    on_toolButton_audio_clicked();
}

void VideoBar::snapshotClicked()
{
    on_toolButton_screenshot_clicked();
}

void VideoBar::setFisheyeEnable(bool enable)
{
    Q_UNUSED(enable)

#ifdef MS_FISHEYE_SOFT_DEWARP
#else
    if (enable) {
        ui->toolButton_fisheye->show();
    } else {
        ui->toolButton_fisheye->hide();
    }
#endif
}

NetworkResult VideoBar::dealNetworkCommond(const QString &commond)
{
    qDebug() << "====VideoBar::dealNetworkCommond====";
    qDebug() << "----commond:" << commond;
    qDebug() << "----visible" << isVisible();
    qDebug() << "----channel:" << m_channel;
    if (!isVisible() || m_channel < 0) {
        return NetworkReject;
    }

    NetworkResult result = NetworkReject;
    if (commond.startsWith("Video_Play")) {
        QMetaObject::invokeMethod(this, "on_toolButton_playback_clicked", Qt::QueuedConnection);
        result = NetworkAccept;
    }
    return result;
}

void VideoBar::focusPreviousChild()
{
    if (focusWidget() == ui->toolButton_record) {
        ui->toolButton_close->setFocus();
        return;
    }
    QWidget::focusPreviousChild();
}

void VideoBar::focusNextChild()
{
    if (focusWidget() == ui->toolButton_close) {
        ui->toolButton_record->setFocus();
        return;
    }
    QWidget::focusNextChild();
}

void VideoBar::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_STOP_AUDIO_TALK:
        ON_RESPONSE_FLAG_STOP_AUDIO_TALK(message);
        break;
    }
}

void VideoBar::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_SYSTEM_INFO:
        ON_RESPONSE_FLAG_GET_IPC_SYSTEM_INFO(message);
        break;
    case RESPONSE_FLAG_CHECK_AUDIOTALK:
        ON_RESPONSE_FLAG_CHECK_AUDIOTALK(message);
        break;
    default:
        break;
    }
}

void VideoBar::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void VideoBar::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void VideoBar::returnPressed()
{
    QToolButton *button = qobject_cast<QToolButton *>(focusWidget());
    if (button) {
        qDebug() << "VideoBar::returnPressed," << button;
        button->click();
    }
}

void VideoBar::ON_RESPONSE_FLAG_GET_IPC_SYSTEM_INFO(MessageReceive *message)
{
    ipc_system_info *audio_info = static_cast<ipc_system_info *>(message->data);
    memset(&m_audio_info, 0, sizeof(ipc_system_info));
    if (!audio_info) {
        qMsWarning() << "data is nullprt.";
    } else {
        memcpy(&m_audio_info, audio_info, sizeof(ipc_system_info));
    }
    m_eventLoop.exit();
}

void VideoBar::ON_RESPONSE_FLAG_CHECK_AUDIOTALK(MessageReceive *message)
{
    m_nvrTalkState = *(int *)(message->data);
    m_eventLoop.exit();
}

void VideoBar::ON_RESPONSE_FLAG_STOP_AUDIO_TALK(MessageReceive *message)
{
    Q_UNUSED(message)
    int tmp = m_channel;
    m_channel = m_audioChn;
    qMsNvr->closeTalkback();
    qMsNvr->closeLiveviewAudio();
    updataCurrentAudio();
    m_channel = tmp;
}

void VideoBar::setEmergencyRecord(bool enable)
{
    ui->toolButton_record->setChecked(enable);
    if (enable) {
        ui->toolButton_record->setToolTip(GET_TEXT("LIVEVIEW/20019", "Stop Emergency Record"));
    } else {
        ui->toolButton_record->setToolTip(GET_TEXT("LIVEVIEW/20018", "Start Emergency Record"));
    }

    if (enable) {
        if (!m_recordAnimationTimer->isActive()) {
            m_recordAnimationTimer->start();
            m_animationState = true;
            onRecordAnimation();
        }
    } else {
        m_recordAnimationTimer->stop();
        ui->toolButton_record->setIcon(QIcon(":/videobar/videobar/recordOff.png"));
    }
}

void VideoBar::setVideoRatio(RATIO_E ratio)
{
    if (ratio == RATIO_VO_FULL) {
        ui->toolButton_ratio->setToolTip(GET_TEXT("LIVEVIEW/20063", "Original"));
        ui->toolButton_ratio->setIcon(QIcon(":/videobar/videobar/imageconfigctl.png"));
    } else {
        ui->toolButton_ratio->setToolTip(GET_TEXT("LIVEVIEW/20064", "Resize"));
        ui->toolButton_ratio->setIcon(QIcon(":/videobar/videobar/imageconfigctlOn.png"));
    }
    ui->toolButton_ratio->setProperty("ratio", (int)ratio);
}

void VideoBar::setAudioState(bool open)
{
    if (open) {
        ui->toolButton_audio->setProperty("audio", true);
        ui->toolButton_audio->setIcon(QIcon(":/videobar/videobar/audioOn.png"));
        ui->toolButton_audio->setToolTip(GET_TEXT("LIVEVIEW/20023", "Audio Off"));
    } else {
        ui->toolButton_audio->setProperty("audio", false);
        ui->toolButton_audio->setIcon(QIcon(":/videobar/videobar/audioOff.png"));
        ui->toolButton_audio->setToolTip(GET_TEXT("LIVEVIEW/20022", "Audio On"));
    }
}

void VideoBar::setTalkbackButtonState(int state)
{
    if (state == 0) {
        ui->toolButton_talkback->setIcon(QIcon(":/liveview/liveview/talkback_off.png"));
        ui->toolButton_talkback->setToolTip(GET_TEXT("LIVEVIEW/20104", "Two-way Audio"));
    } else {
        ui->toolButton_talkback->setIcon(QIcon(":/liveview/liveview/talkback_on.png"));
        ui->toolButton_talkback->setToolTip(GET_TEXT("LIVEVIEW/20104", "Two-way Audio"));
    }
    ui->toolButton_talkback->setProperty("state", state);
}

void VideoBar::setStreamState(int type)
{
    switch (type) {
    case STREAM_TYPE_MAINSTREAM: {
        ui->toolButtonStream->setProperty("stream", STREAM_TYPE_SUBSTREAM);
        ui->toolButtonStream->setIcon(QIcon(":/videobar/videobar/mainStream.png"));
        ui->toolButtonStream->setToolTip(GET_TEXT("LIVEVIEW/20111", "Switch to Sub Stream"));
        break;
    }
    case STREAM_TYPE_SUBSTREAM: {
        ui->toolButtonStream->setProperty("stream", STREAM_TYPE_MAINSTREAM);
        ui->toolButtonStream->setIcon(QIcon(":/videobar/videobar/subStream.png"));
        ui->toolButtonStream->setToolTip(GET_TEXT("LIVEVIEW/20112", "Switch to Main Stream"));
        break;
    }
    default:
        break;
    }
}

void VideoBar::waitForCheckIpcAudioSupport(int channel)
{
    Q_UNUSED(channel)
}

void VideoBar::waitForCheckIpcTalkSupport(int channel)
{
    //考虑至Audio这块IPC经过多个版本变更，较为繁杂，故此次NVR仅从IPC 75版本开始兼容，即低于75版本的IPC提示This channel does not support this function.
    char version[50];
    MsCameraVersion cameraVersion(version);
    if (cameraVersion < MsCameraVersion(7, 75)) {
        memset(&m_audio_info, 0, sizeof(ipc_system_info));
        return;
    }

    Q_UNUSED(channel)
}

void VideoBar::waitForCheckNvrTalkSupport()
{
    m_nvrTalkState = 0;

}

void VideoBar::onLanguageChanged()
{
    if (ui->toolButton_record->isChecked()) {
        ui->toolButton_record->setToolTip(GET_TEXT("LIVEVIEW/20019", "Stop Emergency Record"));
    } else {
        ui->toolButton_record->setToolTip(GET_TEXT("LIVEVIEW/20018", "Start Emergency Record"));
    }
    if (ui->toolButton_audio->property("audio").toBool()) {
        ui->toolButton_audio->setToolTip(GET_TEXT("LIVEVIEW/20023", "Audio Off"));
    } else {
        ui->toolButton_audio->setToolTip(GET_TEXT("LIVEVIEW/20022", "Audio On"));
    }
    ui->toolButton_image->setToolTip(GET_TEXT("LIVEVIEW/20020", "Image Configuration"));
    ui->toolButton_ptz->setToolTip(GET_TEXT("LIVEVIEW/20021", "PTZ"));
    ui->toolButton_fisheye->setToolTip(GET_TEXT("LIVEVIEW/20083", "Fisheye Mode"));
    ui->toolButton_ratio->setToolTip(GET_TEXT("LIVEVIEW/20063", "Original"));
    ui->toolButton_zoom->setToolTip(GET_TEXT("LIVEVIEW/20024", "Electronic Amplification"));
    ui->toolButton_screenshot->setToolTip(GET_TEXT("LIVEVIEW/20025", "Snapshot"));
    ui->toolButton_playback->setToolTip(GET_TEXT("LIVEVIEW/20099", "Instant Playback"));
    ui->toolButton_close->setToolTip(GET_TEXT("LIVEVIEW/20026", "Close the Toolbar"));
    ui->toolButtonAlarm->setToolTip(GET_TEXT("ALARMOUT/53013", "Camera Alarm Output"));
    ui->toolButtonOptimal->setToolTip(GET_TEXT("STATUS/177013", "Optimal Splicing Distance"));
}

void VideoBar::onRecordAnimation()
{
    if (m_animationState) {
        ui->toolButton_record->setIcon(QIcon(":/videobar/videobar/recordOn.png"));
    } else {
        ui->toolButton_record->setIcon(QIcon(":/videobar/videobar/recordOn1.png"));
    }
    m_animationState = !m_animationState;
}

void VideoBar::on_toolButton_record_clicked()
{
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_RECORD)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        ui->toolButton_record->clearUnderMouse();
        return;
    }

    m_state.isEmergencyRecording = !m_state.isEmergencyRecording;
    setEmergencyRecord(m_state.isEmergencyRecording);

    emit sig_setEmergencyRecord(m_state.isEmergencyRecording);
}

void VideoBar::on_toolButton_image_clicked()
{
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_IMAGE)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    emit sig_imageConfiguration();
}

void VideoBar::on_toolButton_ptz_clicked()
{
    ui->toolButton_ptz->clearUnderMouse();
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }

    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL | PERM_LIVE_PTZSETTINGS)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }

    emit sig_ptz();
}

void VideoBar::on_toolButton_ratio_clicked()
{
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_ALL)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    int ratio = ui->toolButton_ratio->property("ratio").toInt();
    if (ratio == RATIO_VO_FULL) {
        ratio = RATIO_VO_AUTO;
        ui->toolButton_ratio->setToolTip(GET_TEXT("LIVEVIEW/20064", "Resize"));
        ui->toolButton_ratio->setIcon(QIcon(":/videobar/videobar/imageconfigctlOn.png"));
    } else {
        ratio = RATIO_VO_FULL;
        ui->toolButton_ratio->setToolTip(GET_TEXT("LIVEVIEW/20063", "Original"));
        ui->toolButton_ratio->setIcon(QIcon(":/videobar/videobar/imageconfigctl.png"));
    }
    ui->toolButton_ratio->setProperty("ratio", (int)ratio);

    emit sig_videoRatio(ratio);
}
void VideoBar::updataCurrentAudio()
{
    updateAudioState();
    updateTalkState();
    LiveView::instance()->updateAudioState(m_channel);
    LiveView::instance()->updateTalkState(m_channel);
    m_audioChn = m_channel;
}

void VideoBar::updatePreviousAudio(int previous_channel)
{
    LiveView::instance()->updateAudioState(previous_channel);
    LiveView::instance()->updateTalkState(previous_channel);
}

void VideoBar::on_toolButton_audio_clicked()
{
    ui->toolButton_audio->clearUnderMouse();
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_AUDIO)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    //if videobar hide, cancel
    if (!isVisible()) {
        return;
    }

    bool audio = ui->toolButton_audio->property("audio").toBool();
    if (audio) {
        //关闭
        qMsNvr->closeLiveviewAudio();
        ui->toolButton_audio->setProperty("audio", !audio);
        updataCurrentAudio();
        return;
    }

    //MsWaitting::showGlobalWait();
    waitForCheckIpcAudioSupport(m_channel);
    //MsWaitting::closeGlobalWait();
    //
    if (!m_audio_info.audioSupport) {
        ShowMessageBox(this, GET_TEXT("PTZCONFIG/36013", "This channel does not support this function."));
        return;
    }
    if (qMsNvr->isAudioOpen()) {
        int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20105", "The other audio will be disabled, continue?"));
        if (result == MessageBox::Cancel) {
            return;
        }
        int previousChannel = qMsNvr->liveviewAudioChannel();
        qMsNvr->closeAudio();
        updatePreviousAudio(previousChannel);
    }
    if (qMsNvr->isCamTalkbackOpen()) {
        int previousChannel = qMsNvr->talkbackChannel();
        int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20106", "The other Two-way audio will be disabled, continue?"));
        if (result == MessageBox::Yes) {
            qMsNvr->closeTalkback();
            qMsNvr->openLiveviewAudio(m_channel);
        }
        updataCurrentAudio();
        updatePreviousAudio(previousChannel);
        return;
    }

    //MsWaitting::showGlobalWait();
    waitForCheckNvrTalkSupport();
    //MsWaitting::closeGlobalWait();
    if (m_nvrTalkState == 1) {
        ShowMessageBox(this, GET_TEXT("LIVEVIEW/20108", "Device is busy."));
        updataCurrentAudio();
        return;
    }
    //打开
    qMsNvr->openLiveviewAudio(m_channel);
    ui->toolButton_audio->setProperty("audio", !audio);
    updataCurrentAudio();
}

void VideoBar::on_toolButton_talkback_clicked()
{
    ui->toolButton_talkback->clearUnderMouse();
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_TWOWAYAUDIO)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    //if videobar hide, cancel
    if (!isVisible()) {
        return;
    }

    int state = ui->toolButton_talkback->property("state").toInt();
    if (state) {
        //关闭
        qMsNvr->closeTalkback();
        updataCurrentAudio();
        return;
    }
    //MsWaitting::showGlobalWait();
    waitForCheckIpcTalkSupport(m_channel);
    waitForCheckNvrTalkSupport();
    //MsWaitting::closeGlobalWait();
    //
    if (!m_audio_info.systemSpeakerSupport) {
        ShowMessageBox(this, GET_TEXT("PTZCONFIG/36013", "This channel does not support this function."));
        return;
    }

    if (qMsNvr->isAudioOpen()) {
        int previousChannel = qMsNvr->liveviewAudioChannel();
        int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20105", "The other audio will be disabled, continue?"));
        if (result == MessageBox::Yes) {
            qMsNvr->closeAudio();
            qMsNvr->openTalkback(m_channel);
        }
        updataCurrentAudio();
        updatePreviousAudio(previousChannel);
        return;
    }
    if (qMsNvr->isCamTalkbackOpen()) {
        int previousChannel = qMsNvr->talkbackChannel();
        int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20106", "The other Two-way audio will be disabled, continue?"));
        if (result == MessageBox::Yes) {
            qMsNvr->openTalkback(m_channel);
        }
        updataCurrentAudio();
        updatePreviousAudio(previousChannel);
        return;
    }
    if (m_nvrTalkState == 1) {
        ShowMessageBox(this, GET_TEXT("LIVEVIEW/20108", "Device is busy."));
        updataCurrentAudio();
        return;
    }
    //打开
    qMsNvr->openTalkback(m_channel);

    updataCurrentAudio();
}

void VideoBar::on_toolButton_zoom_clicked()
{
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_ALL)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    emit sig_zoom(true);
}

void VideoBar::on_toolButton_screenshot_clicked()
{
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_SNAPSHOT)) {
        ui->toolButton_screenshot->clearUnderMouse();
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    emit sig_screenshot(m_channel);
}

void VideoBar::on_toolButton_playback_clicked()
{
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_ALL)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    emit sig_playback(m_channel);
}

void VideoBar::on_toolButton_fisheye_clicked()
{
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_ALL)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    emit sig_fisheye(m_channel);
}

void VideoBar::on_toolButton_close_clicked()
{
    emit sig_hide();
}

void VideoBar::on_toolButtonAlarm_clicked()
{
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_CAMERAALARMOUTPUT)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        ui->toolButtonAlarm->clearUnderMouse();
        return;
    }
    emit sig_alarmout(m_channel);
}

void VideoBar::on_toolButtonStream_clicked()
{
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        return;
    }
    int type = ui->toolButtonStream->property("stream").toInt();
    emit sig_stream(type);
}

void VideoBar::on_toolButtonOptimal_clicked()
{
  if (!LiveView::instance()->isChannelConnected(m_channel)) {
    return;
  }
  if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_IMAGE)) {
    ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
    return;
  }

  emit sig_optimalSplicing();
}
