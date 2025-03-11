#ifndef PLAYBACKLAYOUTSPLIT_H
#define PLAYBACKLAYOUTSPLIT_H

#include "BasePlayback.h"
#include "PlaybackVideo.h"
#include <QWidget>

namespace Ui {
class PlaybackLayout_Split;
}

class PlaybackLayoutSplit : public BasePlayback {
    Q_OBJECT

public:
    explicit PlaybackLayoutSplit(QWidget *parent = nullptr);
    ~PlaybackLayoutSplit();

    static PlaybackLayoutSplit *instance();

    void setSplitLayout(int mode, int selectedSid);
    void resetSplitLayout();

    void showFullPlayback();
    void closeFullPlayback();

    void dealZoom();

    int currentWinId();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void clearGridLayout();

private slots:
    void onVideoClicked(int channel);
    void onVideoDoubleClicked(int channel);

    void dealVideoBar();

private:
    static PlaybackLayoutSplit *s_splitLayout;

    Ui::PlaybackLayout_Split *ui;

    QMap<int, PlaybackVideo *> m_videoMap;
    PlaybackVideo *m_currentVideo = nullptr;
    bool m_isFullPlayback = false;
};

#endif // PLAYBACKLAYOUTSPLIT_H
