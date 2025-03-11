#include "PlaybackVideoBar.h"
#include "ui_PlaybackVideoBar.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackData.h"
#include "PlaybackWindow.h"
#include "SmartSearchControl.h"
#include "SubControl.h"

PlaybackVideoBar *PlaybackVideoBar::self = nullptr;

PlaybackVideoBar::PlaybackVideoBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlaybackVideoBar)
{
    ui->setupUi(this);
    self = this;

    connect(SmartSearchControl::instance(), SIGNAL(modeChanged(int)), this, SLOT(onSmartSearchModeChanged(int)));
    connect(&gPlaybackData, SIGNAL(fisheyeModeChanged(int)), this, SLOT(onFisheyeModeChanged(int)));
    connect(&gPlaybackData, SIGNAL(zoomModeChanged(int)), this, SLOT(onZoomModeChanged(int)));

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    ui->toolButtonLock->setVisible(!qMsNvr->isSlaveMode());
    ui->toolButtonQuickTag->setVisible(!qMsNvr->isSlaveMode());
    ui->toolButtonCustomTag->setVisible(!qMsNvr->isSlaveMode());
    ui->toolButtonSnapshot->setVisible(!qMsNvr->isSlaveMode());
    onLanguageChanged();
}

PlaybackVideoBar::~PlaybackVideoBar()
{
    self = nullptr;
    delete ui;
}

PlaybackVideoBar *PlaybackVideoBar::instance()
{
    return self;
}

void PlaybackVideoBar::show(int channel, const QRect &videoGeometry)
{
    m_channel = channel;
    const QRect &screenGeometry = gSubControl->currentScreenGeometry();
    const QRect &playbackBarGeometry = gPlayback->playbackBarGlobalGeometry();

    QRect rc = rect();
    rc.moveCenter(videoGeometry.center());
    rc.moveTop(videoGeometry.bottom());
    if (videoGeometry.bottom() + 5 > playbackBarGeometry.top()) {
        rc.moveBottom(videoGeometry.top());
    }
    if (rc.top() < screenGeometry.top()) {
        rc.moveTop(videoGeometry.top());
    }
    if (rc.left() < playbackBarGeometry.left()) {
        rc.setLeft(playbackBarGeometry.left());
    }
    if (rc.right() > playbackBarGeometry.right()) {
        rc.setRight(playbackBarGeometry.right());
    }
    setGeometry(rc);

    raise();
    QWidget::show();
}

void PlaybackVideoBar::setSmartSearchButtonState(bool enabled, bool checked)
{
    Q_UNUSED(enabled)
    //ui->toolButtonSmartSearch->setEnabled(enabled);
    ui->toolButtonSmartSearch->setChecked(checked);
}

void PlaybackVideoBar::setSmartSearchButtonVisible(bool visible)
{
    ui->toolButtonSmartSearch->setVisible(visible);
}

void PlaybackVideoBar::setFisheyeButtonState(bool enabled, bool checked)
{
    Q_UNUSED(enabled)
    //ui->toolButtonFisheye->setEnabled(enabled);
    ui->toolButtonFisheye->setChecked(checked);
}

void PlaybackVideoBar::setFisheyeButtonVisible(bool visible)
{
    ui->toolButtonFisheye->setVisible(visible);
}

QRect PlaybackVideoBar::fisheyeButtonGlobalGeometry()
{
    return QRect(ui->toolButtonFisheye->mapToGlobal(QPoint(0, 0)), ui->toolButtonFisheye->size());
}

void PlaybackVideoBar::setZoomButtonState(bool enabled, bool checked)
{
    Q_UNUSED(enabled)
    //ui->toolButtonZoom->setEnabled(enabled);
    ui->toolButtonZoom->setChecked(checked);
}

void PlaybackVideoBar::setSnapshotButtonEnabled(bool enabled)
{
    Q_UNUSED(enabled)
    //ui->toolButtonSnapshot->setEnabled(enabled);
}

void PlaybackVideoBar::setSnapshotButtonChecked(bool checked)
{
    ui->toolButtonSnapshot->setChecked(checked);
}

void PlaybackVideoBar::setAudioButtonEnabled(bool enabled)
{
    QIcon icon;
    if (enabled && ui->toolButtonAudio->isChecked()) {
        icon.addFile(":/PlaybackVideoBar/PlaybackVideoBar/audioOn.png", QSize(32, 32), QIcon::Normal);
    } else if (enabled) {
        icon.addFile(":/PlaybackVideoBar/PlaybackVideoBar/audioOff.png", QSize(32, 32), QIcon::Disabled);
    } else {
        icon.addFile(":/PlaybackVideoBar/PlaybackVideoBar/audioOff-gray.png", QSize(32, 32), QIcon::Disabled);
    }
    ui->toolButtonAudio->setIcon(icon);
    ui->toolButtonAudio->setEnabled(enabled);
}

void PlaybackVideoBar::setAudioButtonChecked(bool checked)
{
    QIcon icon;
    if (checked && ui->toolButtonAudio->isEnabled()) {
        icon.addFile(":/PlaybackVideoBar/PlaybackVideoBar/audioOn.png", QSize(32, 32), QIcon::Normal);
    } else if (ui->toolButtonAudio->isEnabled()) {
        icon.addFile(":/PlaybackVideoBar/PlaybackVideoBar/audioOff.png", QSize(32, 32), QIcon::Normal);
        ui->toolButtonAudio->setChecked(false);
    } else {
        icon.addFile(":/PlaybackVideoBar/PlaybackVideoBar/audioOff-gray.png", QSize(32, 32), QIcon::Disabled);
        ui->toolButtonAudio->setChecked(false);
    }
    ui->toolButtonAudio->setIcon(icon);
    ui->toolButtonAudio->setChecked(checked);
}

void PlaybackVideoBar::setAudioButtonToolTip(const QString &text)
{
    ui->toolButtonAudio->setToolTip(text);
}

void PlaybackVideoBar::setLockButtonEnabled(bool enabled)
{
    Q_UNUSED(enabled)
    //ui->toolButtonLock->setEnabled(enabled);
}

void PlaybackVideoBar::setQuickTagButtonEnable(bool enabled)
{
    Q_UNUSED(enabled)
    //ui->toolButtonQuickTag->setEnabled(enabled);
}

void PlaybackVideoBar::setCustomTagButtonEnable(bool enabled)
{
    Q_UNUSED(enabled)
    //ui->toolButtonCustomTag->setEnabled(enabled);
}

void PlaybackVideoBar::setZoomButtonEnable(bool enabled)
{
    ui->toolButtonZoom->setEnabled(enabled);
}

void PlaybackVideoBar::setNVRDewarpingEnable(bool enabled)
{
    ui->toolButtonFisheye->setEnabled(enabled);
}

void PlaybackVideoBar::setSmartButtonEnabled(bool enabled)
{
    ui->toolButtonSmartSearch->setEnabled(enabled);
}

void PlaybackVideoBar::onLanguageChanged()
{
    ui->toolButtonSmartSearch->setToolTip(GET_TEXT("PLAYBACK/80132", "Smart Search"));
    ui->toolButtonFisheye->setToolTip(GET_TEXT("FISHEYE/12007", "NVR Dewarping"));
    ui->toolButtonZoom->setToolTip(GET_TEXT("LIVEVIEW/20024", "Digital Zoom"));
    ui->toolButtonSnapshot->setToolTip(GET_TEXT("LIVEVIEW/20025", "Snapshot"));
    ui->toolButtonAudio->setToolTip(GET_TEXT("LIVEVIEW/20023", "Audio off"));
    ui->toolButtonLock->setToolTip(GET_TEXT("LIVEVIEW/20060", "Lock"));
    ui->toolButtonQuickTag->setToolTip(GET_TEXT("PLAYBACK/80068", "Quick Tag"));
    ui->toolButtonCustomTag->setToolTip(GET_TEXT("PLAYBACK/80069", "Custom Tag"));
    ui->toolButtonClose->setToolTip(GET_TEXT("LIVEVIEW/20026", "Close the Toolbar"));
}

void PlaybackVideoBar::onSmartSearchModeChanged(int mode)
{
    if (mode) {
        close();
    }
}

void PlaybackVideoBar::onFisheyeModeChanged(int mode)
{
    if (mode) {
        close();
    }
}

void PlaybackVideoBar::onZoomModeChanged(int mode)
{
    if (mode) {
        close();
    }
}

void PlaybackVideoBar::on_toolButtonSmartSearch_clicked()
{
    emit smartSearchClicked();
}

void PlaybackVideoBar::on_toolButtonFisheye_clicked()
{
    emit fisheyeClicked();
}

void PlaybackVideoBar::on_toolButtonZoom_clicked()
{
    emit zoomClicked();
    close();
}

void PlaybackVideoBar::on_toolButtonSnapshot_clicked()
{
    ui->toolButtonSnapshot->clearUnderMouse();
    emit snapshotClicked();
}

void PlaybackVideoBar::on_toolButtonAudio_clicked(bool checked)
{
    ui->toolButtonAudio->clearUnderMouse();
    emit audioClicked(checked);
}

void PlaybackVideoBar::on_toolButtonLock_clicked()
{
    ui->toolButtonLock->clearUnderMouse();
    emit lockClicked();
}

void PlaybackVideoBar::on_toolButtonQuickTag_clicked()
{
    ui->toolButtonQuickTag->clearUnderMouse();
    emit quickTagClicked();
}

void PlaybackVideoBar::on_toolButtonCustomTag_clicked()
{
    ui->toolButtonCustomTag->clearUnderMouse();
    emit customTagClicked();
}

void PlaybackVideoBar::on_toolButtonClose_clicked()
{
    close();
}
