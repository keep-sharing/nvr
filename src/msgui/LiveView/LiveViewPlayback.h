#ifndef LIVEVIEWPLAYBACK_H
#define LIVEVIEWPLAYBACK_H

#include "BaseDialog.h"
#include <QDateTime>
#include <QTimer>

extern "C" {
#include "msg.h"
}

namespace Ui {
class LiveViewPlayback;
}

class LiveViewPlayback : public BaseDialog {
    Q_OBJECT

public:
    explicit LiveViewPlayback(QWidget *parent = 0);
    ~LiveViewPlayback();

    static LiveViewPlayback *instance();

    int currentChannel() const;

    void closePlaybackForPopup();
    void closePlaybackForScreen();
    int execPlayback(int channel, const QRect &videoRect, int winId);

    void showNoResource(const QMap<int, bool> &map);

    NetworkResult dealNetworkCommond(const QString &commond) override;

    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_SET_LIVE_PLAYBACK(MessageReceive *message);

    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    bool isAddToVisibleList() override;

    void escapePressed() override;

private:
    void showNoResource(bool show);

    void setLivePlayback();
    void stopLivePlayback();
    void seekLivePlayback(const QDateTime &dateTime);
    void pauseLivePlayback();
    void resumeLivePlayback();

    void immediatelyUpdatePlayTime();

    void setAudioEnable(int sid);

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
    Ui::LiveViewPlayback *ui;
    static LiveViewPlayback *s_liveViewPlayback;

    QRect m_videoRect;
    QRect m_nvrRect;
    int m_channel;
    int m_sid = -1;
    QList<resp_search_common_backup> m_listCommonBackup;
    int m_winId;

    QTimer *m_timer;
    QDateTime m_currentDateTime;
    QDateTime m_startDateTime;
    QDateTime m_endDateTime;

    int m_result = 0;
};

#endif // LIVEVIEWPLAYBACK_H
