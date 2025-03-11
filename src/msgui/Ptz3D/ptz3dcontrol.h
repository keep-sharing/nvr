#ifndef PTZ3DCONTROL_H
#define PTZ3DCONTROL_H

#include "BaseDialog.h"
extern "C" {
#include "msg.h"
}


class PtzControl;

namespace Ui {
class PTZ3DControl;
}

class PTZ3DControl : public BaseDialog {
    Q_OBJECT

public:
    enum ExitReason {
        ExitForRightButton,
        ExitForPopup,
        ExitForSwitchScreen
    };

    explicit PTZ3DControl(QWidget *parent = nullptr);
    ~PTZ3DControl();

    static PTZ3DControl *instance();

    static bool isSupportPTZ3D(const QString &model);

    void setChannel(int channel);

    void setPtzWidget(PtzControl *ptz);

    void set3DEnable(bool enable);
    void setPtzEnable(bool enable);
    void setManualTrackEnabled(bool enable);

    void closePtz3DForPopup();
    void closePtz3DForSwitchScreen();

    void closePtz3D(ExitReason reason);

    void ptzPanelControl(IPC_PTZ_CONTORL_TYPE cmd, int value);
protected:
    bool eventFilter(QObject *, QEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *) override;

    bool isAddToVisibleList() override;
    void escapePressed() override;

    bool isMoveToCenter() override;


private:
    void ptzDone(int result);

    /****ptz 3d****/
    void set3DPosition(int channel, const QPoint &point, int zoom);
    //单击左键移动区域
    void set3DPosition(const QPoint &point);
    //拖拽区域控制PTZ
    void set3DPosition(const QRect &rc);
    //鼠标滚轮控制zoom
    void set3DPosition(int zoom);

    //
    void setManualTrack(const QRect &rc);

private slots:
    void onSendZoomTimer();

private:
    static PTZ3DControl *s_ptz3dControl;
    Ui::PTZ3DControl *ui;

    int m_channel;

    bool m_pressed = false;
    QPoint m_pressedPoint;

    bool m_showRect = false;
    QRect m_ptzRubberBand;

    QTimer *m_sendZoomTimer = nullptr;
    int m_zoomSteps = 0;

    bool m_is3DEnable = true;
    bool m_isPtzEnable = true;
    bool m_isManualTrackEnable = false;

    PtzControl *m_ptzWidget = nullptr;
};

#endif // PTZ3DCONTROL_H
