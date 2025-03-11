#ifndef PTZCONTROL_H
#define PTZCONTROL_H

#include "BasePopup.h"
#include "networkcommond.h"
#include <QEventLoop>

extern "C" {
#include "msg.h"
}

class QButtonGroup;
class MsWaitting;
class PathWidget;
class PatternWidget;

namespace Ui {
class PtzControl;
}

class PtzControl : public BasePopup {
    Q_OBJECT

public:
    enum PTZ_MODE {
        MODE_PTZ,
        MODE_PTZ3D,
        MODE_FISHEYE
    };

    enum PTZ_STATE {
        STATE_NONSUPPORT, //不支持PTZ
        STATE_PTZ_NORMAL, //常规PTZ
        STATE_PTZ_BUNDLE, //bundle鱼眼ptz
        STATE_PTZ_3D
    };

    enum PtzExitReason {
        ExitNormal,
        ExitForPopup
    };

    explicit PtzControl(QWidget *parent = 0);
    ~PtzControl();

    static PtzControl *instance();

    QPoint calculatePos() override;
    void closePopup(CloseType type) override;

    void escapePressed() override;

    void showWait(const QRect &videoRect, const PTZ_MODE &mode);
    void closeWait();

    int waitForGetPTZSupport(int channel, const QRect &videoRect, const PTZ_MODE &mode);
    int execPtz();
    void exitPtz(PtzExitReason reason);

    void setPtzMode(PTZ_MODE mode);
    PTZ_MODE ptzMode();
    bool isFisheye() const;

    void setChannel(int channel);
    int currentChannel();

    //bundle fisheye
    void setBundleFisheyeStream(int stream);

    PathWidget *pathWidget();
    PatternWidget *patternWidget();

    //
    void sendPtzControl(int action, int Hrate = 0, int Vrate = 0);
    void sendPresetControl(int action, int index);
    void sendPatrolControl(int action, int index);
    void sendPatternControl(int action, int index);

    void sendManualControl(int channel, PTZ_CONTROL_CMD action);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    int dealPtzInfo();
    void setPtzEnable(bool enable);

signals:

private slots:
    void onLanguageChanged();
    void onButtonGroupClicked(int index);

    void onTimerCloseWhiteLight();

    void on_comboBoxBundleStream_activated(int index);

    void on_pushButton_light_clicked(bool checked);
    void on_pushButton_lensInitialize_clicked();
    void on_pushButton_autoFocus_clicked();

    void on_pushButton_close_clicked();

private:
    Ui::PtzControl *ui;

    static PtzControl *s_ptzWidget;

    QButtonGroup *m_buttonGroup;

    bool m_titlePressed = false;
    QPoint m_titlePressedDistance;

    QEventLoop m_popupEventLoop;
    MsWaitting *m_waitting = nullptr;
    QRect m_videoRect;

    PTZ_MODE m_mode;
    int m_channel = -1;

    CAM_MODEL_INFO m_model_info;

    QMap<int, QTimer *> m_ledTimerMap;
};

#endif // PTZCONTROL_H
