#include "PlaybackData.h"
#include "MyDebug.h"

PlaybackData::PlaybackData(QObject *parent)
    : MsObject(parent)
{
}

PlaybackData &PlaybackData::instance()
{
    static PlaybackData self;
    return self;
}

int PlaybackData::playbackSpeed() const
{
    return m_playbackSpeed;
}

void PlaybackData::setPlaybackSpeed(int newPlaybackSpeed)
{
    if (m_playbackSpeed == newPlaybackSpeed) {
        return;
    }
    m_playbackSpeed = newPlaybackSpeed;
    emit playbackSpeedChanged(m_playbackSpeed);
}

void PlaybackData::resetPlaybackSpeed()
{
    setPlaybackSpeed(PLAY_SPEED_1X);
}

int PlaybackData::fisheyeMode() const
{
    return m_fisheyeMode;
}

void PlaybackData::setFisheyeMode(int newFisheyeMode)
{
    if (m_fisheyeMode == newFisheyeMode) {
        return;
    }
    m_fisheyeMode = newFisheyeMode;
    emit fisheyeModeChanged(m_fisheyeMode);
}

void PlaybackData::resetFisheyeMode()
{
    setFisheyeMode(false);
}

int PlaybackData::zoomMode() const
{
    return m_zoomMode;
}

void PlaybackData::setZoomMode(int newZoomMode)
{
    if (m_zoomMode == newZoomMode) {
        return;
    }
    m_zoomMode = newZoomMode;
    emit zoomModeChanged(m_zoomMode);
}

void PlaybackData::resetZoomMode()
{
    setZoomMode(false);
}

void PlaybackData::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}
