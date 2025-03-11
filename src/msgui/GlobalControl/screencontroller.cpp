#include "screencontroller.h"
#include "SubControl.h"
#include "mainwindow.h"
#include "screencontroller_p.h"
#include "tde.h"
#include <QDebug>
#include <QScreen>
#include <QTimer>
#include <QWSServer>
#include <TestHardware.h>
#include <functional>
#include <sys/ioctl.h>

#if defined(_HI3536A_)
#include "gfbg.h"
#else
#include "hifb.h"
#endif

ScreenController::ScreenController()
{
    d = new ScreenControllerPrivate(this);
}

ScreenController::~ScreenController()
{
    tde_fb_uninit();
}

ScreenController *ScreenController::instance()
{
    static ScreenController screenController;
    return &screenController;
}

void ScreenController::blackScreen(int mili)
{
    QScreen *screen = nullptr;

    if (QScreen::instance()->subScreens().isEmpty() && QScreen::instance()->region().contains(QCursor::pos()))
        screen = QScreen::instance();

    for (auto i : QScreen::instance()->subScreens())
        if (i->region().contains(QCursor::pos()))
            screen = i;

    if (!screen)
        return;

    screen->setFbVisible(false);

    QTimer::singleShot(mili, d, SLOT(afterBlackScreen()));
}

void ScreenController::setMouseSpeed(qreal level)
{
    d->mouseSpeed = level;
    changeMouseSpeed();
}

void ScreenController::changeMouseSpeed()
{
    QMetaObject::invokeMethod(d, "onChangeMouseSpeed", Qt::QueuedConnection);
}

void ScreenController::setRefreshRate(DisplayDcMode_e res)
{
    d->refreshRate = 30;

    switch (res) {
    case OUTPUT_RESOLUTION_2160P30:
#if defined(_HI3536G_) || defined(_HI3798_)
        d->refreshRate = 15;
#elif defined(_HI3536C_)
        d->refreshRate = 10;
#else
        d->refreshRate = 15;
#endif
        break;
    case OUTPUT_RESOLUTION_2160P60:
        d->refreshRate = 30;
        break;
    default:
        d->refreshRate = 120;
    }

    qDebug() << "refresh rate: " << d->refreshRate;
    QScreen::instance()->setRefreshRate(d->refreshRate);
}

int ScreenController::refreshRate()
{
    return d->refreshRate;
}

void ScreenController::speedUp()
{
    qDebug() << "speed of refresh rate up";
    QScreen::instance()->setRefreshRate(qMax(d->refreshRate, 60));
}

void ScreenController::speedDown()
{
    qDebug() << "speed of refresh rate down";
    QScreen::instance()->setRefreshRate(d->refreshRate);
}

qreal ScreenController::mouseRealAccel()
{
    return d->mouseRealSpeed;
}

void ScreenController::prepare()
{
    d->setScreenCallback(QScreen::instance());
    for (auto i : QScreen::instance()->subScreens())
        d->setScreenCallback(i);

    blackScreen(1000);

    changeMouseSpeed();
}

void ScreenControllerPrivate::onChangeMouseSpeed()
{
    QString options = "auto:accel=%1:accel_limit=%2";
    int xres, yres;

    vapi_get_screen_res(SubControl::instance()->currentScreen(), &xres, &yres);

    mouseRealSpeed = mouseSpeed;
    if (xres == 3840 && yres == 2160) {
        if (qFuzzyCompare(mouseSpeed, qreal(0.2))) {
            mouseRealSpeed = 0.7;
            options = options.arg(0.7).arg(0);
        } else if (qFuzzyCompare(mouseSpeed, qreal(0.3))) {
            mouseRealSpeed = 0.8;
            options = options.arg(0.8).arg(0);
        } else if (qFuzzyCompare(mouseSpeed, qreal(0.5))) {
            mouseRealSpeed = 0.9;
            options = options.arg(0.9).arg(0);
        } else if (qFuzzyCompare(mouseSpeed, qreal(1))) {
            mouseRealSpeed = 1;
            options = options.arg(1).arg(0);
        } else if (qFuzzyCompare(mouseSpeed, qreal(2))) {
            mouseRealSpeed = 1.2;
            options = options.arg(1.2).arg(20);
        } else if (qFuzzyCompare(mouseSpeed, qreal(3))) {
            mouseRealSpeed = 1.3;
            options = options.arg(1.3).arg(20);
        } else if (qFuzzyCompare(mouseSpeed, qreal(4))) {
            mouseRealSpeed = 1.5;
            options = options.arg(1.5).arg(20);
        } else {
            qCritical() << "unknown mouse speed level!";
            mouseRealSpeed = 1.2;
            options = options.arg(1.2).arg(20);
        }
    } else {
        if (mouseSpeed < 2)
            options = options.arg(mouseSpeed).arg(0);
        else
            options = options.arg(mouseSpeed).arg(5);
    }

    QWSServer::instance()->closeMouse();
    qDebug() << options;
    qputenv("QWS_MOUSE_PROTO", options.toLocal8Bit());
    QWSServer::instance()->openMouse();
}

ScreenControllerPrivate::ScreenControllerPrivate(QObject *parent)
    : QObject(parent)
{
    q = qobject_cast<ScreenController *>(parent);

    QTimer::singleShot(0, this, SLOT(initializeTde()));
}

void ScreenControllerPrivate::setScreenCallback(QScreen *screen)
{
    screen->setFbVisibleCallback(std::bind(&ScreenControllerPrivate::setFbVisible, this, std::placeholders::_1, std::placeholders::_2));
    screen->setTdeCallback(std::bind(&ScreenControllerPrivate::tdeScaling, this,
                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    screen->setWaitForVSyncCallback(std::bind(&ScreenControllerPrivate::waitForVSync, this, std::placeholders::_1));
}

void ScreenControllerPrivate::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != screenUpdater.timerId())
        return;

    if (SubControl::instance()->isMultiSupported()) {
        auto screen = SubControl::instance()->currentScreen() == SCREEN_MAIN ? SCREEN_SUB : SCREEN_MAIN;
        tde_fb_scale(screen, 0, 0, 1920, 1080, NULL);
    }

    if (refreshRate > 60)
        tde_fb_scale(SubControl::instance()->currentScreen(), 0, 0, 1920, 1080, NULL);
}

void ScreenControllerPrivate::afterBlackScreen()
{
    QScreen *screen = nullptr;

    if (QScreen::instance()->subScreens().isEmpty() && QScreen::instance()->region().contains(QCursor::pos()))
        screen = QScreen::instance();

    for (auto i : QScreen::instance()->subScreens())
        if (i->region().contains(QCursor::pos()))
            screen = i;

    if (!screen)
        return;

    screen->invokeTdeCallback(0, 0, 1920, 1080);
    screen->setFbVisible(true);
    if (!TestHardware::instance()->isVisible()) {
        MainWindow::instance()->show();
    }
}

void ScreenControllerPrivate::setFbVisible(int fb, bool visible)
{
#if defined(_HI3536A_)
    td_bool isVisible = visible ? TD_TRUE : TD_FALSE;
    ioctl(fb, FBIOPUT_SHOW_GFBG, &isVisible);
#else
    HI_BOOL isVisible = visible ? HI_TRUE : HI_FALSE;
    ioctl(fb, FBIOPUT_SHOW_HIFB, &isVisible);
#endif
}

void ScreenControllerPrivate::tdeScaling(int x, int y, int width, int height)
{
    if (!isTdeInitialized)
        return;

    x = qMax(0, x - 2);
    y = qMax(0, y - 2);
    width = width + 4;
    height = height + 4;

    if (refreshRate > 60)
        return;

    auto screen = SubControl::instance()->currentScreen();
    tde_fb_scale(screen, x, y, width, height, NULL);
}

void ScreenControllerPrivate::waitForVSync(int fb)
{
    if (refreshRate <= 60) {
#if defined(_HI3536A_)
        ioctl(fb, FBIOGET_VER_BLANK_GFBG);
#else
        ioctl(fb, FBIOGET_VBLANK_HIFB);
#endif
    }
}

void ScreenControllerPrivate::initializeTde()
{
    if (isTdeInitialized) {
        qWarning() << "Tde has been initialized";
        return;
    }

    tde_fb_init();

    screenUpdater.start(1000 / 20, this);

    isTdeInitialized = true;
}
