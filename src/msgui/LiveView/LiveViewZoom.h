#ifndef LIVEVIEWZOOM_H
#define LIVEVIEWZOOM_H

#include "BaseDialog.h"
#include "ZoomControl.h"
#include <QTimer>
#include <QLabel>

namespace Ui {
class LiveViewZoom;
}

class LiveViewZoom : public BaseDialog
{
    Q_OBJECT

public:
    enum ZoomMode
    {
        Normal,
        Anpr
    };

    explicit LiveViewZoom(int winId, QWidget *parent = 0);
    ~LiveViewZoom();

    static LiveViewZoom *instance();

    void showZoom(const LiveViewZoom::ZoomMode &mode, const QRect &rc);
    void closeZoom();

protected:
    void mousePressEvent(QMouseEvent *) override;
    void closeEvent(QCloseEvent *event) override;

    bool isMoveToCenter() override;
    bool isAddToVisibleList() override;

    void escapePressed() override;

private:


private slots:
    void onZoomChanged(int zoom, QRect rc);
    void onTimerAutoHide();

private:
    static LiveViewZoom *s_liveViewZoom;

    Ui::LiveViewZoom *ui;

    ZoomControl *m_control = nullptr;
    ZoomMode m_mode = Normal;

    int m_winId;

    QLabel *m_labelScale = nullptr;
    QTimer *m_timerAutoHide = nullptr;
};

#endif // LIVEVIEWZOOM_H
