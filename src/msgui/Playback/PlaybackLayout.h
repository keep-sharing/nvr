#ifndef PLAYBACKLAYOUT_H
#define PLAYBACKLAYOUT_H

#include "BasePlayback.h"
#include <QMap>

class PlaybackVideo;

namespace Ui {
class PlaybackLayout;
}

#define gPlaybackLayout PlaybackLayout::instance()

class PlaybackLayout : public BasePlayback {
    Q_OBJECT

public:
    explicit PlaybackLayout(QWidget *parent = 0);
    ~PlaybackLayout();

    static PlaybackLayout *instance();

    void setCurrentVideo(int channel);
    void adjustLayout(int channel);
    void resetLayout();
    void refreshLayout();

    int vapiWinId(int channel);

    void dealZoom(int channel);

    bool isSingleLayout() const;
    void enterSingleLayout(int channel);
    void leaveSingleLayout();

    void showNoResource(const QMap<int, bool> &map);
    void clearNoResource();

    void dealMessage(MessageReceive *message) override;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message) override;

    void setPlaybackLayout(const QList<int> &channelList);

signals:
    void videoClicked(int channel);

private slots:
    void onVideoClicked(int channel);
    void onVideoDoubleClicked(int channel);

    void dealVideoBar();

private:
    Ui::PlaybackLayout *ui;

    QList<int> m_channelList;
    int m_currentChannel = -1;

    int m_rowCount = 0;
    int m_columnCount = 0;

    LAYOUT_E m_layout_e = LAYOUT_1;
    bool m_isSingleLayout = false;
#ifdef VAPI2_0
    req_layout2_s m_layout_s;
#else
    req_layout_s m_layout_s;
#endif

    QMap<int, PlaybackVideo *> m_videoMap; //key: channel
};

#endif // PLAYBACKLAYOUT_H
