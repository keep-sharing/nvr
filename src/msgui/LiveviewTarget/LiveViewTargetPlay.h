#ifndef LIVEVIEWTARGETPLAY_H
#define LIVEVIEWTARGETPLAY_H

#include "BaseDialog.h"
#include <QDateTime>
#include <QEventLoop>

extern "C" {
#include "msg.h"
}

class MsWaitting;

namespace Ui {
class LiveViewTargetPlay;
}

class LiveViewTargetPlay : public BaseDialog {
    Q_OBJECT

public:
    explicit LiveViewTargetPlay(QWidget *parent = nullptr);
    ~LiveViewTargetPlay();

    static LiveViewTargetPlay *instance();

    int waitForSearchAnprPlayback(int channel, const QDateTime &dateTime, const QRect &rc);
    void execAnprPlayback(const QRect &rc);
    void closeAnprPlayback();

    void showNoResource(const QMap<int, bool> &map);

    void networkPlay();

    void processMessage(MessageReceive *message) override;

    NetworkResult dealNetworkCommond(const QString &commond) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;

    bool isAddToVisibleList() override;

private:
    void ON_RESPONSE_FLAG_SET_ANPR_LIVE_PLAYBACK(MessageReceive *message);

    void stopAnprPlayback();
    void seekAnprPlayback(const QDateTime &dateTime);
    void pauseAnprPlayback();
    void resumeAnprPlayback();

    void immediatelyUpdatePlayTime();

    void setAudioEnable(bool enable);

    void showNoResource(bool show);

private slots:
    void onLanguageChanged();

    void onSliderValueChanged(int value);
    //
    void onRealPlaybackTime();

    void on_pushButton_play_clicked();
    void on_pushButton_pause_clicked();
    void on_pushButton_close_clicked();

    //
    void onNoResourceChanged(int winid, int bNoResource);

private:
    static LiveViewTargetPlay *s_targetPlay;

    Ui::LiveViewTargetPlay *ui;

    MsWaitting *m_waitting = nullptr;
    QEventLoop m_eventLoop;

    int m_channel;
    int m_sid = -1;
    QList<resp_search_common_backup> m_listCommonBackup;
    anpr_live_pb_info2 m_anpr_pb_info;

    QTimer *m_timer;
    QDateTime m_currentDateTime;
    QDateTime m_startDateTime;
    QDateTime m_endDateTime;

    QRect m_videoRect;
};

#endif // LIVEVIEWTARGETPLAY_H
