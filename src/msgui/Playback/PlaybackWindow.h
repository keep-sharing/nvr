#ifndef PLAYBACKWINDOW_H
#define PLAYBACKWINDOW_H

#include "BasePlayback.h"
#include "BaseWidget.h"
#include "PlaybackLayoutSplit.h"
#include "ThumbWidget.h"

class PlaybackVideoBar;
class SmartSpeedPanel;
class SmartSearchControl;
class FilterEventPanel;

namespace Ui {
class Playback;
}

#define gPlayback PlaybackWindow::instance()

class PlaybackWindow : public BaseWidget {
    Q_OBJECT

public:
    explicit PlaybackWindow(QWidget *parent = 0);
    ~PlaybackWindow();

    static PlaybackWindow *instance();

    void initializeData();

    bool canAutoLogout();
    void showPlayback();
    void closePlayback();
    void switchScreen();

    void closeFisheyeDewarp();
    void closeFisheyePanel();

    QRect playbackBarGlobalGeometry();

    void showNoResource(const QMap<int, bool> &map);

    NetworkResult dealNetworkCommond(const QString &commond) override;

    void dealMessage(MessageReceive *message);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void escapePressed() override;

    bool isAddToVisibleList() override;

private:
    void ON_RESPONSE_FLAG_GET_INDENTATION_DIAGRAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message);

signals:
    void playbackClosed();

private slots:
    void onPlaybackModeChanged(MsPlaybackType mode);
    void onClosePlayback();
    void onToolButtionFilterEventSearch();

private:
    Ui::Playback *ui;

    static PlaybackWindow *s_playback;

    QPoint m_pressedPoint;
    bool m_isPressed = false;
    bool m_isAboutToClose = false;

    ThumbWidget *m_thumbWidget = nullptr;

    QRect m_screenGeometry;
    QRect m_leftGeometry;
    QRect m_layoutGeometry;
    QRect m_playbackBarGeometry;

    PlaybackLayoutSplit *m_layoutSplit = nullptr;

    SmartSpeedPanel *m_smartSpeedPanel = nullptr;

    PlaybackVideoBar *m_videoBar = nullptr;

    FilterEventPanel *m_filterEventPanel = nullptr;

    int m_playMode = -1;
};

#endif // PLAYBACKWINDOW_H
