#ifndef FISHEYEDEWARPCONTROL_H
#define FISHEYEDEWARPCONTROL_H

#include "FisheyePanel.h"
#include "maindialog.h"
#include <QEventLoop>

extern "C" {
#include "fisheye/common.h"
}

namespace Ui {
class FisheyeDewarpControl;
}

QDebug operator<<(QDebug debug, const MSPointi &p);

class FisheyeDewarpControl : public MainDialog {
    Q_OBJECT

public:
    enum Mode {
        ModeNone,
        ModeLiveview,
        ModePlayback,
        ModePlaybackSplit
    };

    explicit FisheyeDewarpControl(QWidget *parent = nullptr);
    ~FisheyeDewarpControl();

    static void setCurrentChannel(FisheyeKey key);
    static FisheyeKey currentChannel();
    static void setVapiWinId(int id);
    static FisheyeKey fisheyeChannel();
    static FisheyeKey fisheyeChannel(FisheyeDewarpControl::Mode mode);
    static void clearFisheyeChannel();
    static void clearFisheyeChannel(FisheyeDewarpControl::Mode mode);
    static void setFisheyeChannel(FisheyeKey key);
    static void setFisheyeChannel(FisheyeDewarpControl::Mode mode, FisheyeKey key);
    static int fisheyeDisplayMode(FisheyeKey key);

    static void callbackWireFrameTouch(void *, int, char *);

    int execFisheyeDewarpControl(FisheyeKey key);
    void showFisheyeDewarpControl(FisheyeKey key);
    void hideFisheyeDewarpControl();

protected:
    void showEvent(QShowEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void setWireFrameEnable(bool enable);
    MSPointi makeMsPoint(const QPoint &p);

signals:
    void dewarpStateChanged(int state);
    void controlClosed();

public slots:
    void onLiveViewFisheyePanelButtonClicked(int x, int y);
    void onPlaybackFisheyePanelButtonClicked(int x, int y);
    void closeFisheyePanel();
    void closeFisheyeDewarp();

    void setFisheyeCursor(int type);

private slots:
    void onFisheyeHandleChanged();
    void onDewarpStateChanged(int state);

private:
    Ui::FisheyeDewarpControl *ui;

    static int s_fisheyeControlMode;
    int m_mode = ModeNone;
    QEventLoop m_eventLoop;
    bool m_pressed = false;
    QPoint m_pressedPoint;
    FisheyePanel *m_fishPanel = nullptr;

    QSize m_nvrScreenSize;
    QSize m_qtScreenSize;

    int m_cursorType = -1;
};
#endif // FISHEYEDEWARPCONTROL_H
