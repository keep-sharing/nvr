#ifndef FISHEYECONTROL_H
#define FISHEYECONTROL_H

#include "BaseDialog.h"

extern "C" {
#include "msg.h"
}

class PtzControl;
class MessageReceive;

namespace Ui {
class FisheyeControl;
}

class FisheyeControl : public BaseDialog {
    Q_OBJECT

public:
    explicit FisheyeControl(QWidget *parent = nullptr);
    ~FisheyeControl();

    static FisheyeControl *instance();

    void showFisheyeBar();
    void closePtz3DForSwitchScreen();

    int currentFisheyeStream();
    int currentFisheyeMode();

    int currentFisheyeMount() const;
    int currentFisheyeDisplay() const;
    bool isTrackEnable() const;
    bool isTrackShow() const;

    void setPtzControl(PtzControl *ptz);
    void showFisheyeControl(int channel);
    int currentChannel() const;

    void setFisheyeMode(int mount, int display);
    void setFisheyeAutoTrack(bool enable);

    void processMessage(MessageReceive *message) override;

protected:
    bool eventFilter(QObject *obj, QEvent *evt) override;
    void showEvent(QShowEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message);

signals:
    void fisheyeChanged();

public slots:

private slots:
    void onFisheyeButtonClicked(int mode);

private:
    static FisheyeControl *s_fisheyeControl;
    Ui::FisheyeControl *ui;

    int m_channel;
    PtzControl *m_ptzControl = nullptr;

    bool m_pressed = false;
    QPoint m_pressedPoint;

    resp_fishmode_param m_fishmode_param;
    ms_auto_tracking m_auto_tracking;
};

#endif // FISHEYECONTROL_H
