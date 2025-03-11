#include "MultiScreenControl.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "CoreThread.h"
#include "mainwindow.h"
#include "MsApplication.h"

#include "MsDevice.h"
#include "MsGlobal.h"
#include "MyInputMethod.h"
#include "screencontroller.h"
#include "signal.h"
#include "splashdialog.h"
#include "SubControl.h"
#include <QDesktopWidget>
#include <QFile>
#include <QFontDatabase>
#include <QResource>
#include <QScreenCursor>
#include <QSettings>
#include <QTextCodec>
#include <QWSServer>
#include <sys/mman.h>
#include "BreakpadHandler.h"

extern "C" {
#include "malloc_extension.h"
#include "msoem.h"
}

#ifndef _HI3536C_
#include <malloc.h>
#include <mcheck.h>
#endif

int gui_proc_exist(char *proc_path);
bool copyFile(QString fileName, QString newName, bool replace);

#define MS_HI_NVR_VERSION  "9.0.19-opensource"

#if 0
static void handler_SIGSEGV(int value)
{
    fprintf(stderr, "\n\n----SIGSEGV_Handler, signal: %d----\n\n", value);
}

static void set_SIGSEGV()
{
    stack_t sigstk;
    sigstk.ss_size = 0;
    sigstk.ss_flags = 0;
    sigstk.ss_sp = mmap (NULL, SIGSTKSZ, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (sigstk.ss_sp != MAP_FAILED) {
        sigstk.ss_size = SIGSTKSZ;
        if (sigaltstack (&sigstk, 0) < 0) {
            sigstk.ss_size = 0;
            fprintf (stderr, "\n\nsigaltstack errno=%d\n\n", errno);
        }
    } else {
        fprintf (stderr, "\n\nmalloc (SIGSTKSZ) failed!\n\n");
    }

    struct sigaction sigaction_SIGSEGV;
    sigaction_SIGSEGV.sa_handler = handler_SIGSEGV;
    sigaction_SIGSEGV.sa_flags = SA_ONSTACK;
    sigaction(SIGSEGV, &sigaction_SIGSEGV, 0);//2
}
#endif

#if 0
static void set_SIGINT()
{
    struct sigaction sigaction_int;
    sigaction_int.sa_handler = handler_SIGINT;
    sigemptyset(&sigaction_int.sa_mask);
    sigaction_int.sa_flags = 0;
    sigaction_int.sa_flags |= SA_RESTART;
    sigaction(SIGINT, &sigaction_int, 0);//2
}
#endif

void onNewHandler()
{
    qCritical() << "operator new failed, out of memeory!";
}

int main(int argc, char *argv[])
{
    fprintf(stderr, "\n\n%s\n\n", "----mscore start----");

    nice(-10);

    std::set_new_handler(onNewHandler);

    QDateTime dateTime = QLocale("en_US").toDateTime(QString("%1 %2").arg(__DATE__).arg(__TIME__).simplified(), "MMM d yyyy HH:mm:ss");
    QString version = QString("firmware datetime: %1, version: %2").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(MS_HI_NVR_VERSION);
    msprintf("%s", version.toStdString().c_str());
    ms_system(QString("echo %1 > /tmp/firmware").arg(version).toStdString().c_str());

#if 0
    // 禁止malloc调用mmap分配内存
    mallopt(M_MMAP_MAX, 0);
    // 禁止内存紧缩
    mallopt(M_TRIM_THRESHOLD, -1);
#endif

    BreakpadHandler::instance().setDumpPath("/mnt/nand", QString("%1_%2").arg(MS_HI_NVR_VERSION).arg(dateTime.toString("yyyyMMddHHmmss")));

#if 0
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) < 0) {
        fprintf(stderr, "\n\n%s\n\n", "----set thread affinity failed----");
    }
#endif

    sqlite_open(SQLITE_FILE_NAME, MS_MSCORE);
    // if (ms_sys_start(1)) {
    //     fprintf(stderr, "\n\n%s\n\n", "----mscore failed----");
    //     return -1;
    // }

    //显示器初始化
    //ms_vapi_init();
    VAPI_ATTR_S stVapiAttr;
    struct display display_info;
    struct device_info device;


    memset(&stVapiAttr, 0, sizeof(VAPI_ATTR_S));
    memset(&display_info, 0, sizeof(struct display));
    memset(&device, 0, sizeof(struct device_info));


    db_get_device(SQLITE_FILE_NAME, &device);
    read_display(SQLITE_FILE_NAME, &display_info);

    stVapiAttr.enRes[SCREEN_MAIN] = SCREEN_1920X1080_60;
    stVapiAttr.enRes[SCREEN_SUB] = SCREEN_1920X1080_60;

    stVapiAttr.stCallback.start_stream_cb   = NULL;
    stVapiAttr.stCallback.stop_stream_cb    = NULL;
    stVapiAttr.stCallback.update_chn_cb     = NULL;
    stVapiAttr.stCallback.update_screen_res_cb = NULL;
    stVapiAttr.stCallback.venc_stream_cb    = NULL;
    stVapiAttr.isHotplug = get_param_int(SQLITE_FILE_NAME, PARAM_DEVICE_HOTPLUG, DEF_HTMI_HOTPLUG);
    stVapiAttr.isDualScreen = get_param_int(SQLITE_FILE_NAME, PARAM_DOUBLE_SCREAN, 1);
#if defined(_HI3536C_)
    stVapiAttr.isDualScreen = 0;
#endif
    memcpy(stVapiAttr.prefix, device.prefix, sizeof (stVapiAttr.prefix));

    stVapiAttr.isogeny = get_param_int(SQLITE_FILE_NAME, PARAM_HOMOLOGOUS, 1);
    stVapiAttr.isBlock = get_param_int(SQLITE_FILE_NAME, PARAM_STREAM_PLAY_MODE, 0);

    vapi_init(&stVapiAttr);

    //多屏支持
    SubControl::initializeMultiScreen();

    //
    fprintf(stderr, "\n\n%s\n\n", "----MsApplication start----");

    //
    gDebug.installMessageHandler();
    gDebug.installMemoryHandler();
    MsApplication *a = new MsApplication(argc, argv);
    a->setQuitOnLastWindowClosed(false);

    qRegisterMetaType<camera>("camera");

    //
    const int screenCount = a->desktop()->numScreens();
    qDebug() << QString("----screen info, count: %1----").arg(screenCount);
    for (int i = 0; i < screenCount; ++i) {
        const QRect &rc = qApp->desktop()->availableGeometry(i);
        switch (i) {
        case 0:
            SubControl::setMainScreenGeometry(rc);
            break;
        case 1:
            SubControl::setSubScreenGeometry(rc);
            break;
        default:
            break;
        }

        qDebug() << QString("screen%1").arg(i) << rc;
    }

    //set_gui_signal();
    QWSServer::setBackground(Qt::transparent);

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);

    int demotion_limit = get_param_int(SQLITE_FILE_NAME, PARAM_DEMOTION_LIMIT, 0);
    if (!demotion_limit) {
        set_param_int(SQLITE_FILE_NAME, PARAM_DEMOTION_LIMIT, 1);
    }

#ifdef MANUAL_RESOURCE
    //rcc文件
    device_info sys_info;
    memset(&sys_info, 0, sizeof(device_info));
    db_get_device(SQLITE_FILE_NAME, &sys_info);
    if (sys_info.oem_type != OEM_TYPE_STANDARD) {
        copyFile("/oem/guifile/gui.rcc", "/opt/app/bin/gui.rcc", true);
    }

    if (!QResource::registerResource("/opt/app/bin/gui.rcc")) {
        qWarning() << "registerResource false";
    }
#endif

    //
#if 1
    MsLanguage::instance()->initialize();
#endif

    //
    MyInputMethod *inputMethod = new MyInputMethod;
    QWSServer::setCurrentInputMethod(inputMethod);

    //
    QScreen *screen = QScreen::instance();
    QList<QScreen *> screens = screen->subScreens();
    if (screens.size() > 1) {
        screen->disableSubScreen(screens.at(1));
    }

    //启动界面
    g_splashMain = new SplashDialog();
    g_splashMain->setGeometry(SubControl::mainScreenGeometry());
    g_splashMain->show();

    bool isMultiSupported = SubControl::isMultiSupported();
    bool isSubEnable = SubControl::isSubEnable();
    qDebug() << "MultiSupported:" << isMultiSupported;
    qDebug() << "SubEnable:" << isSubEnable;
    if (isMultiSupported && isSubEnable) {
        g_splashSub = new SplashDialog();
        g_splashSub->setGeometry(SubControl::subScreenGeometry());
        g_splashSub->show();
    }
    {
        //优化3798平台不显示启动界面问题
        QEventLoop eventLoop;
        QTimer timer;
        QObject::connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
        timer.start(2000);
        eventLoop.exec();
    }

    //保证在主线程初始化
    gCameraData;

    //中心初始化
    fprintf(stderr, "\n\n%s\n\n", "----core initialize begin----");
#ifdef THREAD_CORE
    CoreThread::instance().startCore();
    QEventLoop eventLoop;
    QObject::connect(&CoreThread::instance(), SIGNAL(coreStarted()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
#else
    ms_core_init(0, (char **)0);
#endif
    fprintf(stderr, "\n\n%s\n\n", "----core initialize end----");

    //
    fprintf(stderr, "\n\n%s\n\n", "----MainWindow initialize begin----");
    MainWindow *w = new MainWindow;
    a->setMainWindow(w);
    w->showMaximized();
    w->initializeData();
    QWSServer::showCursor();
    a->setInitializeFinished(true);
    QObject::connect(&CoreThread::instance(), SIGNAL(coreStoped()), w, SLOT(onCoreStoped()));
    fprintf(stderr, "\n\n%s\n\n", "----MainWindow initialize end----");

    ScreenController::instance()->prepare();

    qDebug() << "stack size:" << QThread::currentThread()->stackSize();
    int result = a->exec();

    CoreThread::instance().stopThread();
    vapi_uninit();

    delete inputMethod;
    inputMethod = nullptr;

    if (g_splashMain) {
        delete g_splashMain;
        g_splashMain = nullptr;
    }
    if (g_splashSub) {
        delete g_splashSub;
        g_splashSub = nullptr;
    }

    delete w;
    w = nullptr;

#ifdef MANUAL_RESOURCE
    QResource::unregisterResource("/opt/app/bin/gui.rcc");
#endif

    delete a;
    a = nullptr;

    fprintf(stderr, "\n\n%s\n\n", "----mscore stoped----");
    sqlite_close(SQLITE_FILE_NAME, MS_MSCORE);
    return result;
}

int gui_proc_exist(char *proc_path)
{
    pid_t pId;
    char sBuf[32] = { 0 };
    FILE *fp = fopen(proc_path, "rb");
    if (!fp)
        return 0;
    fgets(sBuf, sizeof(sBuf), fp);
    fclose(fp);
    if (sBuf[0] == '\0')
        return 0;
    pId = atoi(sBuf);
    snprintf(sBuf, sizeof(sBuf), "/proc/%d", pId);

    return QFile::exists(sBuf);
}

bool copyFile(QString fileName, QString newName, bool replace)
{
    if (!QFile::exists(fileName)) {
        return false;
    }
    if (QFile::exists(newName)) {
        if (replace) {
            QFile::remove(newName);
        } else {
            return false;
        }
    }
    return QFile::copy(fileName, newName);
}
