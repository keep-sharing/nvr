#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include "LiveviewBottomBar.h"
#include "BaseWidget.h"
#include "ptzbottombar.h"
#include <QPropertyAnimation>

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "FisheyeDewarpBottomBar.h"
#else
#include "FisheyeBottomBar.h"
#endif

class MyToolButton;
class MessageReceive;

namespace Ui {
class BottomBar;
}

class BottomBar : public BaseWidget {
    Q_OBJECT

public:
    enum BottomMode {
        ModeNormal,
        ModePTZ3D,
        ModeFisheye
    };

    explicit BottomBar(QWidget *parent = 0);
    ~BottomBar();

    static BottomBar *instance();

    void setBottomMode(BottomMode mode, int channel = -1);

    void initializeState();
    void setLocked(bool lock);
    bool isLocked();

    void showOrHide(const QPoint &point);

    void animateShow();
    void animateHide();

    void tempHide();
    void resume();

    MyToolButton *lockButton();

    LiveviewBottomBar *liveviewBottomBar();

    //
    void dealMessage(MessageReceive *message);

    int mode() const;

protected:
    bool eventFilter(QObject *obj, QEvent *evt) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool isAddToVisibleList() override;
    void returnPressed() override;

private:
    bool mouseEnter(const QPoint &mousePos);
    bool mouseLeave(const QPoint &mousePos);

signals:
    void visibleChanged(bool visible);

private slots:
    void onLanguageChanged();
    void animationFinished();
    void updateCurrentTime();
    void on_toolButton_lock_clicked(bool checked);

private:
    Ui::BottomBar *ui;
    static BottomBar *s_bottomBar;

    BottomMode m_mode;
    bool m_locked = false;

    QTimer *m_timerTime = nullptr;

    LiveviewBottomBar *m_liveviewBar = nullptr;
    PtzBottomBar *m_ptzBar = nullptr;
#ifdef MS_FISHEYE_SOFT_DEWARP
    FisheyeDewarpBottomBar *m_fisheyeDewarpBar = nullptr;
#else
    FisheyeBottomBar *m_fisheyeBar = nullptr;
#endif

    int m_height = 70;
    QRect m_enterRect;
    QRect m_leaveRect;
    QPropertyAnimation *m_animation;
    bool m_readyHide = false;

    bool m_isTempHide = false;
};

#endif // BOTTOMBAR_H
