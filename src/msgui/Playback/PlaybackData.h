#ifndef PLAYBACKDATA_H
#define PLAYBACKDATA_H

#include "MsObject.h"

extern "C" {
#include "msdefs.h"
}

#define gPlaybackData PlaybackData::instance()

class PlaybackData : public MsObject
{
    Q_OBJECT
public:
    explicit PlaybackData(QObject *parent = nullptr);

    static PlaybackData &instance();

    int playbackSpeed() const;
    void setPlaybackSpeed(int newPlaybackSpeed);
    void resetPlaybackSpeed();

    int fisheyeMode() const;
    void setFisheyeMode(int newFisheyeMode);
    void resetFisheyeMode();

    int zoomMode() const;
    void setZoomMode(int newZoomMode);
    void resetZoomMode();

    void processMessage(MessageReceive *message) override;

signals:
    void playbackSpeedChanged(int speed);

    void fisheyeModeChanged(int mode);

    void zoomModeChanged(int mode);

private:
    int m_playbackSpeed = PLAY_SPEED_1X;

    int m_fisheyeMode = 0;
    int m_zoomMode = 0;
};

#endif // PLAYBACKDATA_H
