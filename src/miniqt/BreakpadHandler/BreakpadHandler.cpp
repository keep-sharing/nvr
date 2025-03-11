#include "BreakpadHandler.h"
#include "MsApplication.h"
#include "client/linux/handler/exception_handler.h"
#include <QDir>
#include <QtDebug>

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

    return succeeded;
}

BreakpadHandler::BreakpadHandler(QObject *parent)
    : QObject(parent)
{

}

BreakpadHandler::~BreakpadHandler()
{
}

BreakpadHandler &BreakpadHandler::instance()
{
    static BreakpadHandler self;
    return self;
}

void BreakpadHandler::setDumpPath(const QString &path)
{
    QString absPath = path;
    if (!QDir::isAbsolutePath(absPath)) {
        absPath = QDir::cleanPath(qApp->applicationDirPath() + "/" + path);
    }
    Q_ASSERT(QDir::isAbsolutePath(absPath));

    QDir().mkpath(absPath);
    if (!QDir().exists(absPath)) {
        qCritical() << "Failed to set dump path which not exists:" << absPath;
        return;
    }

    QString prefix = "77.9.0.16-hi3536a";

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
