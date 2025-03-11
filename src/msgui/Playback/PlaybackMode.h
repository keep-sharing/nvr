#ifndef PLAYBACKMODE_H
#define PLAYBACKMODE_H

#include "BasePlayback.h"
#include "PlaybackGeneral.h"

namespace Ui {
class PlaybackMode;
}

class PlaybackMode : public BasePlayback {
    Q_OBJECT

public:
    explicit PlaybackMode(QWidget *parent = 0);
    ~PlaybackMode();

    void initializeData();

    NetworkResult dealNetworkCommond(const QString &commond);

    void dealMessage(MessageReceive *message) override;

signals:
    void playbackModeChanged(MsPlaybackType mode);

public slots:
    void closePlayback();

private slots:
    void onLanguageChanged();

    void on_comboBox_playType_activated(int index);
    void on_comboBox_stream_activated(int index);
    void onToolButtonCloseClicked();

private:
    Ui::PlaybackMode *ui;
};

#endif // PLAYBACKMODE_H
