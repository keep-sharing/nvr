#include "BreakpadHandler.h"
#include "MsApplication.h"
#include "MyDebug.h"
#include "client/linux/handler/exception_handler.h"
#include <QDir>

extern "C" {
#include "msstd.h"
}
#define MS_HI_NVR_VERSION  "9.0.19-opensource"

google_breakpad::ExceptionHandler *pExptHandler = nullptr;

bool DumpCallback(const google_breakpad::MinidumpDescriptor &descriptor, void *context, bool succeeded)
{
    Q_UNUSED(context);

    /*
        NO STACK USE, NO HEAP USE THERE !!!
        Creating QString's, using qDebug, etc. - everything is crash-unfriendly.
    */

    fprintf(stderr, "\033[31m--------\nwrite minidump, succeeded: %d, path: %s\n--------\n\n\033[0m",
            succeeded, descriptor.path());

    char cmd[128] = { 0 };
    time_t t      = time(NULL);
    printf("mscore received signal: %d, backtrace stack at time: %s...\n", 11, ctime(&t));
    snprintf(cmd, sizeof(cmd), "%s", "free");
    ms_system(cmd);
    pid_t pId = getpid();
    snprintf(cmd, sizeof(cmd), "cat /proc/%d/status", pId);
    ms_system(cmd);
    msdebug(DEBUG_ERR, "mscore received signal: 11, backtrace stack at time: %s version:%s\n", ctime(&t), MS_HI_NVR_VERSION);

    return succeeded;
}

BreakpadHandler::BreakpadHandler(QObject *parent)
    : QObject(parent)
{
    QDir dir("/mnt/nand");
    QList<QFileInfo> infoList = dir.entryInfoList(QStringList("*.dmp"), QDir::Files, QDir::Time);
    while (infoList.size() > 1) {
        QFileInfo info = infoList.takeLast();
        ms_system(QString("rm %1").arg(info.absoluteFilePath()).toStdString().c_str());
        qDebug() << "remove dmp file:" << info.absoluteFilePath();
    }
}

BreakpadHandler::~BreakpadHandler()
{
}

BreakpadHandler &BreakpadHandler::instance()
{
    static BreakpadHandler self;
    return self;
}

void BreakpadHandler::setDumpPath(const QString &path, const QString &newPrefix)
{
    QString absPath = path;
    if (!QDir::isAbsolutePath(absPath)) {
        absPath = QDir::cleanPath(qApp->applicationDirPath() + "/" + path);
    }
    Q_ASSERT(QDir::isAbsolutePath(absPath));

    QDir().mkpath(absPath);
    if (!QDir().exists(absPath)) {
        qMsCritical() << "Failed to set dump path which not exists:" << absPath;
        return;
    }

    QString prefix;
#if defined(_HI3536G_)
    prefix += "71.";
#elif defined(_HI3798_)
    prefix += "72.";
#elif defined(_HI3536C_)
    prefix += "73.";
#elif defined(_NT98323_)
    prefix += "75.";
#elif defined(_HI3536A_)
    prefix += "77.";
#elif defined(_NT98633_)
    prefix += "78.";
#else
    prefix += "7x.";
#endif
    prefix += newPrefix;

    google_breakpad::MinidumpDescriptor descriptor(absPath.toStdString(), prefix.toStdString());
    descriptor.set_size_limit(1 * 1024 * 1024);

    pExptHandler = new google_breakpad::ExceptionHandler(descriptor,
                                                         /*FilterCallback*/ 0,
                                                         DumpCallback,
                                                         /*context*/ 0,
                                                         true,
                                                         -1);
}

bool BreakpadHandler::writeDump()
{
    qCritical("manual write minidump");
    bool result = pExptHandler->WriteMinidump();
    return result;
}
