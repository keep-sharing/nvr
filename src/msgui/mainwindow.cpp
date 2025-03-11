#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "AutoLogout.h"
#include "BottomBar.h"
#include "CameraData.h"
#include "ContentLiveView.h"
#include "CoreThread.h"
#include "DownloadPanel.h"
#include "DynamicDisplayData.h"
#include "EventDetectionRegionManager.h"
#include "EventPopup.h"
#include "LiveView.h"
#include "LiveViewOccupancyManager.h"
#include "LiveViewSub.h"
#include "LogoutChannel.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MessageHelper.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsDisk.h"
#include "MsGlobal.h"
#include "MsGraphicsScene.h"
#include "MsLanguage.h"
#include "MsObject.h"
#include "MsWaitting.h"
#include "MsWidget.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"
#include "PacketCaptureData.h"
#include "PeopleCountingAutoBackup.h"
#include "PlaybackWindow.h"
#include "Script.h"
#include "SubControl.h"
#include "TargetInfoManager.h"
#include "autotest.h"
#include "centralmessage.h"
#include "logout.h"
#include "messagepool.h"
#include "mslogin.h"
#include "msqtvca.h"
#include "msuser.h"
#include "mswizard.h"
#include "ptzdatamanager.h"
#include "resetpassword.h"
#include "screencontroller.h"
#include "screensetting.h"
#include "settingcontent.h"
#include "splashdialog.h"
#include "userlogin.h"
#include <QDesktopWidget>
#include <QFileInfo>
#include <QMouseDriverFactory>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QWSMouseHandler>
#include <QWSServer>
#include <TestHardware.h>

MainWindow *MainWindow::s_mainWindow = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
#ifndef _NT98323_
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    s_mainWindow = this;

    qsrand(QDateTime::currentMSecsSinceEpoch());

    qRegisterMetaType<MsQtVca::ObjectSizeType>("MsQtVca::ObjectSizeType");

    connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)), this, SLOT(onFocusWidgetChanged(QWidget *, QWidget *)));

    //
    //    m_fileWatcher = new QFileSystemWatcher(this);
    //    m_fileWatcher->addPath("/dev/input");
    //    connect(m_fileWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
    //    m_mouseConnected = QFileInfo("/dev/input/mice").exists();

    //
    qDebug() << "arguments:" << qApp->arguments();
    if (qApp->arguments().contains("-test")) {
        AutoTest *autoTest = new AutoTest(this);
        autoTest->show();

        autoTest->move(0, 200);
    }

    //
    MessageBox::initialize(this);

    //
    m_msDevice = new MsDevice(this);

    //
    m_messagePool = new MessagePool(this);
    connect(&gMsMessage, SIGNAL(sig_message()), this, SLOT(dealMessage()), Qt::QueuedConnection);

    //
    m_subControl = new SubControl(this);

    //content liveview
    ContentLiveView::instance();
    //
    m_liveView = new LiveView(ContentLiveView::instance());
    connect(m_liveView, SIGNAL(menuItemClicked(MainMenu::MenuItem)), this, SLOT(menuItemClicked(MainMenu::MenuItem)));

    //
    m_eventPopup = new EventPopup(this);
    m_eventPopup->hide();

    //
    m_login = new MsLogin(this);
    connect(m_login, SIGNAL(loginFinished()), this, SLOT(onLoginFinished()));

    //
    m_wizard = new MsWizard(this);
    connect(m_wizard, SIGNAL(sig_finished()), this, SLOT(onWizardFinished()));

    //
    initPlayback();

    //
    initSettingContent();

    //
    connect(&gMsDisk, SIGNAL(detectedUninitializedDisk()), this, SLOT(onDetectedUninitializedDisk()), Qt::QueuedConnection);

    //
    m_fileSystemDialog = new MyFileSystemDialog(this);
    m_fileSystemDialog->hide();

    //
    m_networkCommond = new NetworkCommond(this);

    m_testHardware = new TestHardware();
    connect(m_testHardware, SIGNAL(closed()), this, SLOT(show()));
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow::~MainWindow()";
    delete m_testHardware;
    s_mainWindow = nullptr;
    delete ui;
}

MainWindow *MainWindow::instance()
{
    return s_mainWindow;
}

/**
 * @brief MainWindow::initializeData
 */
void MainWindow::initializeData()
{
    qMsDebug() << "begin";
    do {
        //读上一次登录的用户
        qMsDebug() << "1";
        reboot_conf conf;
        read_autoreboot_conf(SQLITE_FILE_NAME, &conf);
        if (QString(conf.username).isEmpty()) {
            qMsWarning() << "table auto_reboot.username is empty, use admin default!!!";
            snprintf(conf.username, sizeof(conf.username), "%s", "admin");
        }
        gMsUser.initializeData(conf.username);

        //
        qMsDebug() << "2";
        if (conf.enable && conf.reboot) {
            conf.reboot = 0;
            write_autoreboot_conf(SQLITE_FILE_NAME, &conf);
            MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_AUTO_REBOOT, NULL, 0);
        }

        //是否开启了启动向导
        qMsDebug() << "3";
        char wizard[20] = { 0 };
        get_param_value(SQLITE_FILE_NAME, PARAM_GUI_WIZARD_ENABLE, wizard, sizeof(wizard), "");
        bool isWizardEnable = QString(wizard).toInt();
        //热备模式默认跳过开机向导
        if (qMsNvr->isSlaveMode()) {
            isWizardEnable = false;
        }
        if (isWizardEnable) {
            showWizard();
            break;
        }

        //是否开启了登录认证
        qMsDebug() << "4";
        char auth[20] = { 0 };
        get_param_value(SQLITE_FILE_NAME, PARAM_GUI_AUTH, auth, sizeof(auth), "");
        bool localAuthEnable = QString(auth).toInt();
        if (localAuthEnable) {
            showLogin();
            break;
        }

        //啥都没有，直接进入预览
        qMsDebug() << "5";
        showLiveView();
        break;
    } while (0);

    //
    m_subControl->initializeLater();
    qMsDebug() << "end";
}

void MainWindow::showLiveView(LiveView::ShowReason reason)
{
    //
    if (g_splashMain) {
        g_splashMain->hide();
        delete g_splashMain;
        g_splashMain = nullptr;
    }
    if (g_splashSub) {
        g_splashSub->hide();
        delete g_splashSub;
        g_splashSub = nullptr;
    }

    //
    if (DownloadPanel::instance()) {
        DownloadPanel::instance()->hide();
    }

    //
    if (m_liveView) {
        m_liveView->showLiveView(reason);
        ScreenController::instance()->blackScreen();
    }

    //
    if (SubControl::instance()->isSubEnable() && LiveViewSub::instance()) {
        LiveViewSub::instance()->show();
    }

    QMetaObject::invokeMethod(this, "onPermissionUpdateCheck", Qt::QueuedConnection);

    emit liveViewVisibilityChanged(true);
}

void MainWindow::hideLiveView()
{
    if (m_liveView) {
        m_liveView->stopLiveView();
        m_liveView->hideMainMenu();
        m_liveView->hide();
        ScreenController::instance()->blackScreen();
    }

    emit liveViewVisibilityChanged(false);
}

void MainWindow::showLogin()
{
    qDebug() << QString("MainWindow::showLogin");

    //
    if (SubControl::instance()->isSubEnable()) {
        if (!g_splashSub) {
            g_splashSub = new SplashDialog(this);
        }
        g_splashSub->setGeometry(SubControl::subScreenGeometry());
        g_splashSub->show();
    }
    //
    m_login->showLogin();
}

void MainWindow::showWizard()
{
    qDebug() << QString("MainWindow::showWizard 1");

    m_wizard->showWizard();

    //
    qDebug() << QString("MainWindow::showWizard 2");
    if (g_splashMain) {
        g_splashMain->hide();
        delete g_splashMain;
        g_splashMain = nullptr;
    }
    qDebug() << QString("MainWindow::showWizard 3");
    if (SubControl::instance()->isSubEnable()) {
        if (!g_splashSub) {
            g_splashSub = new SplashDialog(this);
        }
        g_splashSub->setGeometry(SubControl::subScreenGeometry());
        g_splashSub->show();
    }
    //
    //qApp->processEvents();
    qDebug() << QString("MainWindow::showWizard end");
}

void MainWindow::logout()
{
    qMsNvr->closeAudio();
    qMsNvr->closeTalkback();
    LogoutChannel::instance()->logout();
    gAutoLogout.logout();
    if (LiveView::instance()) {
        LiveView::instance()->closeAllPopupWindow();
    }
    showLiveView(LiveView::LogoutShowReason);
}

void MainWindow::logoutToLogin()
{
    logout();
    LogoutChannel::instance()->setTempLogin(true);
    showLogin();
}

void MainWindow::checkDisk()
{
    gMsDisk.getDiskInfo();
}

void MainWindow::dealMouseMove(const QPoint &pos)
{
    if (LogoutChannel::instance()->isLogout()) {
        return;
    }
    if (LiveViewOccupancyManager::instance() && LiveViewOccupancyManager::instance()->isCurrentScreenOccupancy()) {
        return;
    }
    //
    static QPoint point;
    if (point == pos) {
        return;
    }
    point = pos;
    if (m_liveView) {
        m_liveView->showOrHideWidget(pos);
    }
}

void MainWindow::readyToQuit()
{
    qDebug() << "MainWindow::readyToQuit";

    QMetaObject::invokeMethod(this, "onReadyToQuit", Qt::QueuedConnection);
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (m_testHardware->isVisible()) {
        return;
    }
    qMsDebug() << "";
    QMainWindow::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    qMsDebug() << "";
    QMainWindow::hideEvent(event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
#ifdef _NT98323_
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(rect(), Qt::SolidPattern);
#else
    QMainWindow::paintEvent(event);
#endif
}

void MainWindow::initPlayback()
{
    if (m_playback) {
        return;
    }
    m_playback = new PlaybackWindow(this);
    connect(m_playback, SIGNAL(playbackClosed()), this, SLOT(onPlaybackClosed()), Qt::QueuedConnection);
    m_playback->hide();
}

void MainWindow::releasePlayback()
{
}

void MainWindow::initSettingContent()
{
    if (m_settingContent) {
        return;
    }
    m_settingContent = new SettingContent(this);
    connect(m_settingContent, SIGNAL(closed()), this, SLOT(onSettingsClosed()), Qt::QueuedConnection);
    m_settingContent->hide();
}

void MainWindow::ON_RESPONSE_FLAG_POWER_SHORT(MessageReceive *message)
{
    Q_UNUSED(message)

    if (!isVisible()) {
        return;
    }

    qDebug() << QString("MainWindow::ON_RESPONSE_FLAG_POWER_SHORT");

    if (!m_shutdownMessageBox) {
        m_shutdownMessageBox = new MessageBox(this);
        connect(m_shutdownMessageBox, SIGNAL(yesButtonClicked()), this, SLOT(onShutdownClicked()));
    }
    if (m_shutdownMessageBox->isVisible()) {
        onShutdownClicked();
    } else {
        m_shutdownMessageBox->showQuestion(GET_TEXT("LIVEVIEW/20050", "You can press power key again to shutdown. Do you want to shutdown? "), QString());
        m_shutdownMessageBox->show();
    }
}

void MainWindow::ON_RESPONSE_FLAG_POWER_LONG(MessageReceive *message)
{
    Q_UNUSED(message)

    if (!isVisible()) {
        return;
    }

    qDebug() << QString("MainWindow::ON_RESPONSE_FLAG_POWER_LONG");

    onShutdownClicked();
}

void MainWindow::ON_RESPONSE_FLAG_SET_DISPARAM(MessageReceive *message)
{
    Q_UNUSED(message)
    int smallVideoChannel = -1;
    if (CommonVideo::instance()) {
        smallVideoChannel = CommonVideo::instance()->stopAllVideo();
    }
    if (smallVideoChannel > -1 && CommonVideo::instance()) {
        CommonVideo::instance()->adjustVideoRegion();
        CommonVideo::instance()->playVideo(smallVideoChannel);
        qDebug() << QString("########################################################  MainWindow::ON_RESPONSE_FLAG_SET_DISPARAM");
    }

    //刷新布局
    if (SubControl::instance()->isSubEnable()) {
        SCREEN_E currentScreen = SubControl::instance()->currentScreen();
        switch (currentScreen) {
        case SCREEN_MAIN:
            if (LiveView::instance() && LiveView::instance()->isVisible()) {
                LiveView::instance()->resetMainScreenLayout();
            }
            if (LiveViewSub::instance() && LiveViewSub::instance()->isVisible()) {
                LiveView::instance()->resetSubScreenLayout();
            }
            break;
        case SCREEN_SUB:
            if (LiveViewSub::instance() && LiveViewSub::instance()->isVisible()) {
                LiveView::instance()->resetMainScreenLayout();
            }
            if (LiveView::instance() && LiveView::instance()->isVisible()) {
                LiveView::instance()->resetSubScreenLayout();
            }
            break;
        default:
            break;
        }
    } else {
        if (LiveView::instance() && LiveView::instance()->isVisible()) {
            LiveView::instance()->resetMainScreenLayout();
        }
    }

    update();
}

void MainWindow::ON_RESPONSE_FLAG_USER_UPDATE(MessageReceive *message)
{
    Q_UNUSED(message)

    db_user currentUser = gMsUser.currentUserInfo();
    db_user newUser;
    memset(&newUser, 0, sizeof(db_user));
    read_user(SQLITE_FILE_NAME, &newUser, currentUser.id);
    if (gMsUser.isUserChanged(currentUser, newUser)) {
        qMsDebug() << QString("user '%1' info changed.").arg(currentUser.username);
        if (MsWizard::instance() && MsWizard::instance()->isVisible()) {
            if (MsWizard::instance()->isActivatePage()) {
                MsWizard::instance()->next();
                return;
            } else {
                MsWizard::instance()->finishWizard();
            }
        }
        if (SettingContent::instance() && SettingContent::instance()->isVisible()) {
            SettingContent::instance()->closeSetting();
        }
        if (m_playback && m_playback->isVisible()) {
            m_playback->closePlayback();
        }
        QTimer::singleShot(100, this, SLOT(logoutToLogin()));
    }
}

void MainWindow::resetGeometry()
{
    QRect rc1 = SubControl::instance()->physicalMainScreenGeometry();
    QRect rc2 = SubControl::instance()->physicalSubScreenGeometry();
    QRect rc(0, 0, rc1.width() + rc2.width(), qMax(rc1.height(), rc2.height()));
    setGeometry(rc);
    //
    ContentLiveView::instance()->setGeometry(rc);
    //
    MsLogin::instance()->setGeometry(SubControl::instance()->logicalMainScreenGeometry());
}

void MainWindow::ON_RESPONSE_FLAG_ENABLE_SCREEN_WEB(MessageReceive *message)
{
    Q_UNUSED(message)
    display db_display = qMsNvr->displayInfo();
    bool isSub = SubControl::instance()->isSubControl();
    int sub_enable = !(db_display.sub_enable);
    qMsNvr->readDisplayInfo();
    db_display = qMsNvr->displayInfo();

    EventPopup::instance()->setPopupInfo(db_display.eventPop_screen, db_display.eventPop_time);

    REFRESH_E enRefresh;
    if (sub_enable == 0 && !isSub) //辅屏状态下关不了
    {
        db_display.sub_enable = 0;
        enRefresh = REFRESH_CLEAR_VDEC;
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_ENABLE_SCREEN, (void *)&enRefresh, sizeof(REFRESH_E));

        SubControl::instance()->setSubEnable(false);
    } else {
        db_display.sub_enable = 1;
        SubControl::instance()->setSubEnable(true);
    }

    qMsNvr->writeDisplayInfo(&db_display);
    qMsNvr->setQuickSwitchScreenEnable(db_display.sub_enable);
    qDebug() << QString("######################################################## MainWindow::dealMessage() isSubControl():[%1], db_display.sub_enable:[%2], sub_enable_cmd:[%3]").arg(isSub).arg(db_display.sub_enable).arg(sub_enable);
}

/**
 * @brief MainWindow::dealMessage
 * 处理中心消息
 */
void MainWindow::dealMessage()
{
    MessageReceive *message = MsMessagePool->usedMessageReceive();
    if (message) {
        //
        //yCDebug("qt_message") << "recv:" << gMessageHelper.messageTypeString(message->type()).toLocal8Bit().constData() << "begin";
        if (message->obj) {
            if (message->obj->isValid()) {
                message->obj->processMessage(message);
            } else {
                //发送的obj已被释放
                qMsDebug() << "obj has been destroyed, type:" << message->type();
            }
        } else {
            switch (message->type()) {
            case RESPONSE_FLAG_POWER_SHORT:
                ON_RESPONSE_FLAG_POWER_SHORT(message);
                break;
            case RESPONSE_FLAG_POWER_LONG:
                ON_RESPONSE_FLAG_POWER_LONG(message);
                break;
            case RESPONSE_FLAG_SENDTO_QT_CMD:
                NetworkCommond::instance()->ON_RESPONSE_FLAG_SENDTO_QT_CMD(message);
                break;
            case RESPONSE_FLAG_SET_DISPARAM:
                ON_RESPONSE_FLAG_SET_DISPARAM(message);
                break;
            case RESPONSE_FLAG_ENABLE_SCREEN_WEB:
                ON_RESPONSE_FLAG_ENABLE_SCREEN_WEB(message);
                break;
            case RESPONSE_FLAG_UDISK_OFFLINE:
                if (PacketCaptureData::instance()) {
                    PacketCaptureData::instance()->dealMessage(message);
                }
                break;
            case RESPONSE_FLAG_GUI_AUTO_BACKUP_INFO: {
                //web或者中心要获取qt的后台下载情况
                RespAutoBackupInfo info;
                info.from = *((int *)message->data);
                if (DownloadPanel::instance()) {
                    info.status = DownloadPanel::instance()->isDownloading();
                } else {
                    info.status = 0;
                }
                MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_GUI_AUTO_BACKUP_INFO, &info, sizeof(info));
                break;
            }
            case RESPONSE_FLAG_STOP_AUTO_BACKUP_GUI: {
                //web或者中心要取消qt的后台下载
                //修改为不取消
#if 0
            if (DownloadList::instance()) {
                DownloadList::instance()->stopDownload();
            }
#endif
                break;
            }
            case RESPONSE_FLAG_GUI_PEOPLECNT_AUTO_BACKUP: {
                //人数统计自动备份
                gPeopleCountingAutoBackup.processMessage(message);
                break;
            }
            case RESPONSE_FLAG_USER_UPDATE: {
                //user信息改变，web端改变等
                ON_RESPONSE_FLAG_USER_UPDATE(message);
                break;
            }
            default:
                break;
            }
            //
            if (gDownload) {
                gDownload->dealMessage(message);
            }
            //
            if (MyFileSystemDialog::instance()) {
                MyFileSystemDialog::instance()->dealMessage(message);
            }
            //
            gMsDisk.dealMessage(message);

            if (m_liveView) {
                m_liveView->dealMessage(message);
            }

            if (m_playback && m_playback->isVisible()) {
                m_playback->dealMessage(message);
            }

            if (m_settingContent) {
                m_settingContent->dealMessage(message);
            }
        }
        //
        gMessageFilter.processEventFilter(message);
        //yCDebug("qt_message") << "recv:" << gMessageHelper.messageTypeString(message->type()).toLocal8Bit().constData() << "end";

        //
        MsMessagePool->setMessageUsed(message, false);
    }
    //
    if (MsMessagePool->hasUnprocessedMessageReceive()) {
        QMetaObject::invokeMethod(this, "dealMessage", Qt::QueuedConnection);
    }
}

void MainWindow::onDirectoryChanged(const QString &path)
{
    QFileInfo fileInfo("/dev/input/mice");
    bool tempConnect = fileInfo.exists();
    qMsDebug() << path << ", mice exists:" << tempConnect;

    if (tempConnect) {
        QWSServer::instance()->openMouse();
    }
}

void MainWindow::onPermissionUpdateCheck()
{
    //升级到14版本需要提示
    if (get_param_int(SQLITE_FILE_NAME, PARAM_PERMISSION_UPDATE, 0) == 1) {
        set_param_int(SQLITE_FILE_NAME, PARAM_PERMISSION_UPDATE, 0);
        MessageBox::information(this, GET_TEXT("USER/74080", "User permissions except user with 'Admin' level have been changed to default, please set again if necessary."));
    }

    //
    static bool isFirst = true;
    if (isFirst) {
        isFirst = false;
        //
        checkDisk();
        //
        if (LiveviewBottomBar::instance()) {
            LiveviewBottomBar::instance()->getExceptionStatus();
            if (qMsNvr->isMilesight()) {
                MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_ONLINE_UPGRADE_INFO, nullptr, 0);
            }
        }
    }
}

void MainWindow::onWizardFinished()
{
    char tmp[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_AUTH, tmp, sizeof(tmp), "");
    bool localAuthEnable = QString(tmp).toInt();
    if (localAuthEnable) {
        showLogin();
    } else {
        showLiveView();
    }
}

void MainWindow::onLoginFinished()
{
    if (EventPopup::instance()->isVisible()) {
        EventPopup::instance()->closePopup();
    }
    //
    LogoutChannel::instance()->clearLogout();
    LogoutChannel::instance()->setTempLogin(false);
    showLiveView(LiveView::LoginShowReason);
    //
    gAutoLogout.resume();
}

void MainWindow::onDetectedUninitializedDisk()
{
    if (!m_liveView->isVisible()) {
        return;
    }

    MessageBox box(m_settingContent);
    box.showQuestion(GET_TEXT("DISK/92030", "Detected uninitialized disks, please init the disk."), QString());
    box.move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, box.size(), qApp->desktop()->geometry()).topLeft());
    int result = box.exec();
    if (result == MessageBox::Yes) {
        hideLiveView();
        m_settingContent->showDiskSetting();
    }
}

void MainWindow::onPlaybackClosed()
{
    if (gAutoLogout.isReadyLogout()) {
        logout();
    } else {
        resumeLiveView();
    }
}

void MainWindow::onSettingsClosed()
{
    if (gAutoLogout.isReadyLogout()) {
        logout();
    } else {
        resumeLiveView();
    }
}

void MainWindow::menuItemClicked(const MainMenu::MenuItem &item)
{
    //
    if (item == MainMenu::ItemLogout) {
        Logout logout(this);
        const int &result = logout.exec();
        switch (result) {
        case Logout::TypeLogout: {
            // qMsNvr->closeAudio();
            // qMsNvr->closeTalkback();
            // LogoutChannel::instance()->logout();
            // showLiveView(LiveView::LogoutShowReason);
            break;
        }
        case Logout::TypeReboot: {
            //qMsNvr->reboot();
            //qMsApp->setAboutToReboot(true);
            break;
        }
        case Logout::TypeShutdown: {
            // if (qMsNvr->isSupportPhysicalShutdown()) {
            //     onShutdownClicked();
            // } else {
            //     m_isFakerShutdown = true;
            //     CoreThread::instance().stopCore();
            // }
            break;
        }
        default:
            break;
        }
        return;
    }

    //

    //
    hideLiveView();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    m_liveView->resetAnotherScreenLayout();

    switch (item) {
    case MainMenu::ItemPlayback: {
        m_playback->initializeData();
        m_playback->showPlayback();
        break;
    }
    case MainMenu::ItemRetrieve:
    case MainMenu::ItemCamera:
    case MainMenu::ItemSmart:
    case MainMenu::ItemStorage:
    case MainMenu::ItemEvent:
    case MainMenu::ItemSettings:
    case MainMenu::ItemStatus: {
        if (!m_settingContent) {
            initSettingContent();
        }
        m_settingContent->goSetting(item);
        break;
    }
    default:
        break;
    }
}

void MainWindow::enterMenuLayout()
{
    hideLiveView();
    m_settingContent->showLayoutSetting();
}

void MainWindow::onFocusWidgetChanged(QWidget *old, QWidget *now)
{
    qMsCDebug("qt_focus") << "====MainWindow::onFocusWidgetChanged====";
    qMsCDebug("qt_focus") << "----old:" << old;
    qMsCDebug("qt_focus") << "----now:" << now;
}

void MainWindow::resumeLiveView()
{
    showLiveView();
}

void MainWindow::onShutdownClicked()
{
    if (!m_shutdownMessageBox) {
        m_shutdownMessageBox = new MessageBox(this);
        connect(m_shutdownMessageBox, SIGNAL(yesButtonClicked()), this, SLOT(onShutdownClicked()));
    }

    m_shutdownMessageBox->showMessage(GET_TEXT("LIVEVIEW/20051", "The system is shutting down..."), QString());
    m_shutdownMessageBox->show();

    //qMsNvr->shutdownLater(500);
}

void MainWindow::onReadyToQuit()
{
    qMsDebug() << "onReadyToQuit, begin";

    MsWaitting *waitting = MsWaitting::instance();
    waitting->setCustomPos(true);
    waitting->moveToCenter(SubControl::instance()->mainScreenGeometry());
    //waitting->//showWait();

    if (Script::instance()) {
        Script::instance()->deleteLater();
    }

    gTargetInfoManager.readyToQuit();
    gDynamicDisplayData.readyToQuit();
    gCameraData.readyToQuit();
    LiveViewOccupancyManager::instance()->readyToQuit();
    qMsNvr->readyToQuit();
    gMsMessage.stopThread();

    qMsDebug() << "onReadyToQuit, end";
}

void MainWindow::onCoreStoped()
{
    if (LiveView::instance()) {
        LiveView::instance()->hide();
    }
    if (LiveViewSub::instance()) {
        LiveViewSub::instance()->hide();
    }

    if (m_isFakerShutdown) {
        MessageBox::message(this, GET_TEXT("SYSTEMGENERAL/70073", "Please power off NVR now."));
        return;
    }

    qApp->exit();
}
