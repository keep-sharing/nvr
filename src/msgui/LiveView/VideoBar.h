#ifndef VIDEOBAR_H
#define VIDEOBAR_H

#include "BaseWidget.h"
#include <QTimer>
#include <QEventLoop>
#include "LiveVideo.h"

namespace Ui {
class VideoBar;
}

class VideoBar : public BaseWidget
{
    Q_OBJECT

public:
    explicit VideoBar(QWidget *parent = 0);
    ~VideoBar();

    int realWidth();

    void showVideoBar(int channel);
    void closeVideoBar();
    void updateState(LiveVideo::State state);

    void updateAudioState();
    void updateTalkState();
    void updataStream();

    void recordClicked();
    void audioClicked();
    void snapshotClicked();

    void setFisheyeEnable(bool enable);

    NetworkResult dealNetworkCommond(const QString &commond) override;

    void focusPreviousChild();
    void focusNextChild();

    void dealMessage(MessageReceive *message);
    void processMessage(MessageReceive *message) override;

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    void returnPressed() override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_SYSTEM_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_CHECK_AUDIOTALK(MessageReceive *message);
    void ON_RESPONSE_FLAG_STOP_AUDIO_TALK(MessageReceive *message);

    void setEmergencyRecord(bool enable);
    void setVideoRatio(RATIO_E ratio);
    void setAudioState(bool open);
    void setTalkbackButtonState(int state);
    void setStreamState(int type);

    void waitForCheckIpcAudioSupport(int channel);
    void waitForCheckIpcTalkSupport(int channel);
    void waitForCheckNvrTalkSupport();

    void updataCurrentAudio();
    void updatePreviousAudio(int previous_channel);

signals:
    void sig_setEmergencyRecord(bool enable);
    void sig_imageConfiguration();
    void sig_ptz();
    void sig_videoRatio(int type);
    void sig_audio(int channel, bool enable);
    void sig_zoom(bool enable);
    void sig_screenshot(int channel);
    void sig_playback(int channel);
    void sig_fisheye(int channel);
    void sig_alarmout(int channel);
    void sig_hide();
    void sig_stream(int type);
    void sig_optimalSplicing();

private slots:
    void onLanguageChanged();

    void onRecordAnimation();

    void on_toolButton_record_clicked();
    void on_toolButton_image_clicked();
    void on_toolButton_ptz_clicked();
    void on_toolButton_ratio_clicked();
    void on_toolButton_audio_clicked();
    void on_toolButton_talkback_clicked();
    void on_toolButton_zoom_clicked();
    void on_toolButton_screenshot_clicked();
    void on_toolButton_playback_clicked();
    void on_toolButton_fisheye_clicked();
    void on_toolButton_close_clicked();
    void on_toolButtonAlarm_clicked();
    void on_toolButtonStream_clicked();
    void on_toolButtonOptimal_clicked();

  private:
    Ui::VideoBar *ui;

    QEventLoop m_eventLoop;
    LiveVideo::State m_state;
    QTimer *m_recordAnimationTimer;
    bool m_animationState = false;
    int m_channel = -1;
    int m_audioChn = -1;
    //
    ipc_system_info m_audio_info;
    int m_nvrTalkState = 0;
};

#endif // VIDEOBAR_H
