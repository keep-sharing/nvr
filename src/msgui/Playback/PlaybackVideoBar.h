#ifndef PLAYBACKVIDEOBAR_H
#define PLAYBACKVIDEOBAR_H

#include <QWidget>

namespace Ui {
class PlaybackVideoBar;
}

#define gPlaybackVideoBar PlaybackVideoBar::instance()

class PlaybackVideoBar : public QWidget
{
    Q_OBJECT

public:
    explicit PlaybackVideoBar(QWidget *parent = nullptr);
    ~PlaybackVideoBar();

    static PlaybackVideoBar *instance();

    void show(int channel, const QRect &videoGeometry);

    void setSmartSearchButtonState(bool enabled, bool checked);
    void setSmartSearchButtonVisible(bool visible);

    void setFisheyeButtonState(bool enabled, bool checked);
    void setFisheyeButtonVisible(bool visible);
    QRect fisheyeButtonGlobalGeometry();

    void setZoomButtonState(bool enabled, bool checked);

    void setSnapshotButtonEnabled(bool enabled);
    void setSnapshotButtonChecked(bool checked);

    void setAudioButtonEnabled(bool enabled);
    void setAudioButtonChecked(bool checked);
    void setAudioButtonToolTip(const QString &text);

    void setLockButtonEnabled(bool enabled);
    void setQuickTagButtonEnable(bool enabled);
    void setCustomTagButtonEnable(bool enabled);
    void setZoomButtonEnable(bool enabled);
    void setNVRDewarpingEnable(bool enabled);
    void setSmartButtonEnabled(bool enabled);

signals:
    void smartSearchClicked();
    void fisheyeClicked();
    void zoomClicked();
    void snapshotClicked();
    void audioClicked(bool checked);
    void lockClicked();
    void quickTagClicked();
    void customTagClicked();

private slots:
    void onLanguageChanged();

    void onSmartSearchModeChanged(int mode);
    void onFisheyeModeChanged(int mode);
    void onZoomModeChanged(int mode);

    void on_toolButtonSmartSearch_clicked();
    void on_toolButtonFisheye_clicked();
    void on_toolButtonZoom_clicked();
    void on_toolButtonSnapshot_clicked();
    void on_toolButtonAudio_clicked(bool checked);
    void on_toolButtonLock_clicked();
    void on_toolButtonQuickTag_clicked();
    void on_toolButtonCustomTag_clicked();
    void on_toolButtonClose_clicked();

private:
    static PlaybackVideoBar *self;

    Ui::PlaybackVideoBar *ui;

    int m_channel = -1;
};

#endif // PLAYBACKVIDEOBAR_H
