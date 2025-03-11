#ifndef PLAYBACKZOOM_H
#define PLAYBACKZOOM_H

#include "ZoomControl.h"

#include <QLabel>
#include <QTimer>
#include <QWidget>

namespace Ui {
class PlaybackZoom;
}

class PlaybackZoom : public QWidget {
    Q_OBJECT

public:
    explicit PlaybackZoom(QWidget *parent = nullptr);
    ~PlaybackZoom();

    static PlaybackZoom *instance();

    void showZoom(int channel, int vapiWinId);
    void setZoomMode(bool zoom);
    bool isZoomMode() const;

    bool isZooming() const;
    void closeZoom();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void zoomStateChanged(int zoom);

private slots:
    void onZoomChanged(int zoom, QRect rc);
    void onTimerAutoHide();

private:
    Ui::PlaybackZoom *ui;

    static PlaybackZoom *s_playbackZoom;

    int m_channel = -1;
    bool m_isZoomMode = false;

    ZoomControl *m_control = nullptr;

    int m_winId;

    QLabel *m_labelScale = nullptr;
    QTimer *m_timerAutoHide = nullptr;
};

#endif // PLAYBACKZOOM_H
