#include "SubControl.h"
#include "EventPopup.h"
#include "LiveVideo.h"
#include "LiveView.h"
#include "LiveViewOccupancyManager.h"
#include "LiveViewSub.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "PlaybackWindow.h"
#include "centralmessage.h"
#include "commonvideo.h"
#include "mainwindow.h"
#include "progressdialog.h"
#include "screencontroller.h"
#include "splashdialog.h"
#include "tde.h"
#include <QDesktopWidget>
#include <QFile>
#include <QScreen>
#include <QWSServer>

SubControl *SubControl::s_subControl = nullptr;
bool SubControl::s_multiSupported = false;
bool SubControl::s_isSubEnable = false;
QRect SubControl::s_mainScreenGeometry;
QRect SubControl::s_subScreenGeometry;

SubControl::SubControl(QObject *parent)
    : MsObject(parent)
{
    s_subControl = this;

    //
    connect(qMsApp, SIGNAL(middleButtonDoubleClicked()), this, SLOT(onMiddleButtonDoubleClicked()), Qt::QueuedConnection);

    readDisplayInfo();
    qDebug() << "main screen resolution: " << m_db_display.main_resolution;
    qDebug() << "sub screen resolution: " << m_db_display.sub_resolution;

    if (m_db_display.sub_enable) {
        setSubEnable(m_db_display.sub_enable);

        m_startScreen = static_cast<SCREEN_E>(m_db_display.start_screen);
        if (m_startScreen == SCREEN_SUB) {
            setSubControl(true);
        }
    }

    m_switchTimer = new QTimer(this);
    m_switchTimer->setSingleShot(true);
    m_switchTimer->setInterval(3000);

    /**一开始就切换FrameBuffer可能会有问题，这里放在事件循环里去切换**/
    //MSHN-8849
    //QT-主辅屏：控制屏在辅屏时，重启NVR，辅屏显示黑屏；此时操作鼠标滚轮双击切至主屏，辅屏会进入预览；再切回辅屏，辅屏在Wirzard首页【重启后，辅屏应在wirzard首页，主屏显示Milesight Logo】
    /**2020-09-17改由mainwindow主动调用**/
    //QTimer::singleShot(0, this, SLOT(initializeLater()));
    //QMetaObject::invokeMethod(this, "initializeLater", Qt::QueuedConnection);
}

SubControl::~SubControl()
{
    qDebug() << "SubControl::~SubControl()";
    s_subControl = nullptr;
}

SubControl *SubControl::instance()
{
    //qDebug() << QString("SubControl::instance");
    return s_subControl;
}

void SubControl::initializeMultiScreen()
{
    qDebug() << "====SubControl::initializeMultiScreen====";

    struct display display_info;
    struct device_info device;
    memset(&display_info, 0, sizeof(struct display));
    memset(&device, 0, sizeof(struct device_info));
    db_get_device(SQLITE_FILE_NAME, &device);
    read_display(SQLITE_FILE_NAME, &display_info);

    s_isSubEnable = display_info.sub_enable;

    qDebug() << "----model:" << device.model;
    qDebug() << "----prefix:" << device.prefix;
    qDebug() << "----main_resolution:" << display_info.main_resolution;
    qDebug() << "----sub_resolution:" << display_info.sub_resolution;

    QRect mainRect(0, 0, 1920, 1080);
    //    switch(display_info.main_resolution)
    //    {
    //    case OUTPUT_RESOLUTION_1080P:
    //        mainRect = QRect(0, 0, 1920, 1080);
    //        break;
    //    case OUTPUT_RESOLUTION_720P:
    //        mainRect = QRect(0, 0, 1280, 720);
    //        break;
    //    case OUTPUT_RESOLUTION_SXGA:
    //        mainRect = QRect(0, 0, 1280, 1024);
    //        break;
    //    case OUTPUT_RESOLUTION_XGA:
    //        mainRect = QRect(0, 0, 1024, 768);
    //        break;
    //    case OUTPUT_RESOLUTION_1080P50:
    //        mainRect = QRect(0, 0, 1920, 1080);
    //        break;
    //    case OUTPUT_RESOLUTION_2160P30:
    //        mainRect = QRect(0, 0, 3840, 2160);
    //        break;
    //    case OUTPUT_RESOLUTION_2160P60:
    //        mainRect = QRect(0, 0, 3840, 2160);
    //        break;
    //    default:
    //        mainRect = QRect(0, 0, 1920, 1080);
    //        break;
    //    }

    bool multiSupport = false;
    QString strModel(device.model);
    QString strPrefix(device.prefix);
    if (strModel == QString("MS-N7016-UH") || strModel == QString("MS-N7032-UH") || strModel == QString("MS-N7016-UPH") || strModel == QString("MS-N7032-UPH") || strModel == QString("MS-N7048-UPH") || strModel == QString("MS-N5032-UH") || 
        strModel == QString("MS-N7016-G") || strModel == QString("MS-N7016-PG") || strModel == QString("MS-N7032-G") || strModel == QString("MS-N7048-PG")) {
        //这个功能仅3536平台，一个HDMI+一个VGA的型号支持（MS-N7016-UH，MS-N7032-UH，MS-N7016-UPH，MS-N7032-UPH, MS-N7048-UPH）
        int homologous = get_param_int(SQLITE_FILE_NAME, PARAM_HOMOLOGOUS, 1);
        if (homologous == 0) {
            multiSupport = true;
        }
    } else if (strPrefix == QString("8")) {
        multiSupport = true;
    }
    qDebug() << "----multiSupport:" << multiSupport;
    setMultiSupported(multiSupport);

    if (isMultiSupported()) {
        qputenv("QWS_DISPLAY", QString("multi: LinuxFb:/dev/fb0:0 LinuxFb:/dev/fb1:offset=%1,0:1 :0").arg(mainRect.width()).toLatin1());

        QScreenCursor::setMainScreenRect(mainRect);
    }
}

void SubControl::setMultiSupported(bool support)
{
    s_multiSupported = support;
}

bool SubControl::isMultiSupported()
{
    return s_multiSupported;
}

void SubControl::setMainScreenGeometry(const QRect &rc)
{
    s_mainScreenGeometry = rc;
}

void SubControl::setSubScreenGeometry(const QRect &rc)
{
    s_subScreenGeometry = rc;
}

/**
 * @brief SubControl::mainScreenGeometry
 * 当前鼠标操作的屏幕
 * @return
 */
QRect SubControl::mainScreenGeometry()
{
    return s_mainScreenGeometry;
}

/**
 * @brief SubControl::subScreenGeometry
 * @return
 */
QRect SubControl::subScreenGeometry()
{
    return s_subScreenGeometry;
}

QRect SubControl::currentScreenGeometry()
{
    if (gSubControl->isSubControl()) {
        return subScreenGeometry();
    } else {
        return mainScreenGeometry();
    }
}

QString SubControl::screenString(int screen)
{
    QString text;
    switch (screen) {
    case SCREEN_MAIN:
        text = QString("SCREEN_MAIN(%1)").arg(screen);
        break;
    case SCREEN_SUB:
        text = QString("SCREEN_SUB(%1)").arg(screen);
        break;
    default:
        text = QString("UnknowScreen(%1)").arg(screen);
        break;
    }
    return text;
}

void SubControl::initializeLater()
{
    qMsDebug() << QString("begin");

    if (m_db_display.sub_enable) {
        m_startScreen = static_cast<SCREEN_E>(m_db_display.start_screen);
        if (m_startScreen == SCREEN_SUB)
            switchFrameBuffer(QString("/dev/fb1"), false);
        else if (isMultiSupported())
            switchFrameBuffer(QString("/dev/fb0"), false);
    } else {
        ScreenController::instance()->setRefreshRate(DisplayDcMode_e(m_db_display.main_resolution));
        //
        MainWindow::instance()->resetGeometry();
    }

    qMsDebug() << QString("end");
}

void SubControl::onMiddleButtonDoubleClicked()
{
    if (!qMsNvr->isQuickSwitchScreenEnable()) {
        return;
    }
    if (LiveVideo::isDraging()) {
        return;
    }
    onSwitchScreen();
}

void SubControl::onSwitchScreen()
{
    if (m_switchTimer->isActive()) {
        return;
    }
    if (MsWaitting::hasWaitting()) {
        qDebug() << QString("SubControl::onSwitchScreen, has waitting.");
        return;
    }
    if (ProgressDialog::instance() && ProgressDialog::instance()->isVisible()) {
        qDebug() << QString("SubControl::onSwitchScreen, has progress.");
        return;
    }
    if (EventPopup::instance()) {
        EventPopup::instance()->closePopup();
    }

    qMsNvr->clearNoResource();
    //如果有小窗口在播放，先停止
    int smallVideoChannel = -1;
    if (CommonVideo::instance()) {
        smallVideoChannel = CommonVideo::instance()->stopAllVideo();
    }
    //
    if (PlaybackWindow::instance() && PlaybackWindow::instance()->isVisible()) {
        PlaybackWindow::instance()->switchScreen();
    }

    //切换屏幕
    LiveView::instance()->switchScreen();
    //恢复之前停止的小窗口
    if (smallVideoChannel > -1 && CommonVideo::instance()) {
        CommonVideo::instance()->playVideo(smallVideoChannel);
    }
    //
    if (LiveViewOccupancyManager::instance()) {
        LiveViewOccupancyManager::instance()->resetOccupancy(LiveViewOccupancyManager::NormalReason);
    }
    m_switchTimer->start();

    emit screenSwitched();
}

/**
 * @brief SubControl::setSubEnable
 * @param enable: 是否启用辅屏
 */
void SubControl::setSubEnable(bool enable)
{
    qMsDebug() << QString("enable: %1").arg(enable);

    m_isSubEnable = enable;
    s_isSubEnable = enable;

    if (enable) {
        m_startScreen = static_cast<SCREEN_E>(m_db_display.start_screen);
        if (m_startScreen == SCREEN_MAIN) {

        } else {
        }

        if (LiveView::instance()) {
            LiveView::instance()->initializeSubScreenLayout();
            if (LiveViewSub::instance()) {
                LiveViewSub::instance()->setGeometry(logicalSubScreenGeometry());
                LiveViewSub::instance()->show();
                LiveViewSub::instance()->updateDisplayInfo();
            }
        }
    } else {
        if (isMultiSupported()) {
            LiveViewSub::instance()->hide();
        }
    }
}

bool SubControl::isSubEnable()
{
    return s_isSubEnable;
}

/**
 * @brief SubControl::setSubControl
 * @param enable: true(当前操作辅屏), false(当前操作主屏)
 */
void SubControl::setSubControl(bool enable)
{
    m_isSubControl = enable;

    if (enable) {
        qDebug() << QString("----Control Sub Screen----");
    } else {
        qDebug() << QString("----Control Main Screen----");
    }
}

bool SubControl::isSubControl()
{
    return m_isSubControl;
}

SCREEN_E SubControl::startScreen()
{
    return m_startScreen;
}

SCREEN_E SubControl::currentScreen()
{
    return isSubControl() ? SCREEN_SUB : SCREEN_MAIN;
}

/**
 * @brief SubControl::mainLiveViewScreen
 * 当前鼠标在操作的屏幕
 * @return
 */
SCREEN_E SubControl::mainLiveViewScreen()
{
    return isSubControl() ? SCREEN_SUB : SCREEN_MAIN;
}

/**
 * @brief SubControl::subLiveViewScreen
 * @return
 */
SCREEN_E SubControl::subLiveViewScreen()
{
    return isSubControl() ? SCREEN_MAIN : SCREEN_SUB;
}

QRect SubControl::logicalMainScreenGeometry()
{
    return mainScreenGeometry();
}

QRect SubControl::logicalSubScreenGeometry()
{
    return subScreenGeometry();
}

QRect SubControl::physicalMainScreenGeometry()
{
    if (isSubControl()) {
        return subScreenGeometry();
    } else {
        return mainScreenGeometry();
    }
}

QRect SubControl::physicalSubScreenGeometry()
{
    if (isSubControl()) {
        return mainScreenGeometry();
    } else {
        return subScreenGeometry();
    }
}

QRect SubControl::mapToNvrRect(const QRect qtRect) const
{
    int nvrWidth = 0;
    int nvrHeight = 0;
    vapi_get_screen_res(SubControl::instance()->currentScreen(), &nvrWidth, &nvrHeight);

    QRect screenRect = qApp->desktop()->geometry();

    qreal rx = (qreal)nvrWidth / screenRect.width();
    qreal ry = (qreal)nvrHeight / screenRect.height();

    return QRect(qtRect.left() * rx, qtRect.top() * ry, qtRect.width() * rx, qtRect.height() * ry);
}

void SubControl::switchScreen()
{
    QMetaObject::invokeMethod(this, "onSwitchScreen");
}

/**
 * @brief SubControl::switchFrameBuffer
 * @param displaySpec: "/dev/fb1"
 */
void SubControl::switchFrameBuffer(const QString &displaySpec, bool clearFB)
{
    Q_UNUSED(clearFB)
    qDebug() << "SubControl::switchFrameBuffer, displaySpec:" << displaySpec << ", begin.";
    if (!isMultiSupported()) {
        qDebug() << QString("SubControl::switchFrameBuffer, displaySpec: %1").arg(displaySpec);
        QWSServer *server = QWSServer::instance();
        QScreen *screen = QScreen::instance();
        tde_fb_scale(currentScreen(), 0, 0, 1920, 1080, NULL);

        QWSServer::setCursorVisible(false);
        server->enablePainting(false);
        screen->shutdownDevice();
        screen->disconnect();
        qApp->processEvents();
        screen->connect(displaySpec);
        screen->initDevice();
        server->enablePainting(true);
        server->refresh();
        qDebug() << QString("Switch to frame buffer: %1").arg(displaySpec);

        QWSServer::setCursorVisible(true);

        ScreenController::instance()->setRefreshRate(DisplayDcMode_e(m_db_display.main_resolution));
    } else {
        QWSServer *server = QWSServer::instance();
        QScreen *screen = QScreen::instance();
        tde_fb_scale(currentScreen(), 0, 0, 1920, 1080, NULL);

        //
        QWSServer::setCursorVisible(false);
        server->enablePainting(false);
        qDebug() << "----shutdownDevice";
        screen->shutdownDevice();
        qDebug() << "----disconnect";
        screen->disconnect();

        qDebug() << "----connect";
        int fb = displaySpec.right(1).toInt();

        if (fb == 0) {
            QString text = QString("multi: LinuxFb:/dev/fb0:0 LinuxFb:/dev/fb1:offset=%1,0:1 :0").arg(mainScreenGeometry().width());
            qDebug() << text;
            screen->connect(text);
            ScreenController::instance()->setRefreshRate(DisplayDcMode_e(m_db_display.main_resolution));
        } else {
            QString text = QString("multi: LinuxFb:/dev/fb1:0 LinuxFb:/dev/fb0:offset=%1,0:1 :0").arg(subScreenGeometry().width());
            qDebug() << text;
            screen->connect(text);
            ScreenController::instance()->setRefreshRate(DisplayDcMode_e(m_db_display.sub_resolution));
        }

        qDebug() << "----initDevice";
        screen->initDevice();
        qDebug() << "----enablePainting";
        server->enablePainting(true);
        qDebug() << "----refresh";
        server->refresh();
        QWSServer::setCursorVisible(false);

        qDebug() << "SubControl::switchFrameBuffer, displaySpec:" << displaySpec << ", end.";


        QList<QScreen *> screens = screen->subScreens();
        if (screens.size() > 0) {
            QScreenCursor::setMainScreenRect(screens.at(0)->region().boundingRect());
        }

        if (screens.size() > 1) {
            screen->disableSubScreen(screens.at(1));
        }
    }

    ScreenController::instance()->prepare();

    //
    MainWindow::instance()->resetGeometry();
}

void SubControl::readDisplayInfo()
{
    memset(&m_db_display, 0, sizeof(struct display));
    read_display(SQLITE_FILE_NAME, &m_db_display);
}
