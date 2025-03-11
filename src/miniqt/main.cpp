#include "MainWindow.h"

#include "BreakpadHandler.h"
#include "MsApplication.h"
#include "MyInputMethod.h"
#include "tde.h"
#include <QApplication>
#include <QWSServer>
#include <QtDebug>

void myMessageOutput(QtMsgType type, const char *msg)
{
    QString text(msg);

    //兼容qDebug
    text = QString("[GUI][%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
    switch (type) {
    case QtDebugMsg:
        text.append(QString("qDebug: %1").arg(msg));
        fprintf(stderr, "\033[32m%s\033[0m\n", text.toStdString().c_str());
        break;
    case QtWarningMsg:
        text.append(QString("qWarning: %1").arg(msg));
        fprintf(stderr, "\033[31m%s\033[0m\n", text.toStdString().c_str());
        break;
    case QtCriticalMsg:
        text.append(QString("qCritical: %1").arg(msg));
        fprintf(stderr, "\033[35m%s\033[0m\n", text.toStdString().c_str());
        break;
    case QtFatalMsg:
        text.append(QString("qFatal: %1").arg(msg));
        fprintf(stderr, "\033[35m%s\033[0m\n", text.toStdString().c_str());
        break;
    }
}

int main(int argc, char *argv[]) {
    qInstallMsgHandler(myMessageOutput);
    // if (argc < 3) {
    //     qDebug() << "using:\n"
    //              << " start with 4k_30Hz: miniqt -4k30 -qws\n"
    //              << " start with 4k_60Hz: miniqt -4k60 -qws\n";
    //     return 0;
    // }
    SCREEN_RES_E res = SCREEN_1920X1080_60;
    for (int i = 0; i < argc; i++) {
        if (QString(argv[i]) == "-4k30") {
            res = SCREEN_3840X2160_30;
            break;
        }
        if (QString(argv[i]) == "-4k60") {
            res = SCREEN_3840X2160_60;
            break;
        }
    }
    if (res == SCREEN_RES_NUM) {
        qDebug() << "using:\n"
                 << " start with 1080p_60Hz: miniqt -1080p -qws\n"
                 << " start with 4k_30Hz: miniqt -4k30 -qws\n";
                //  << " start with 4k_60Hz: miniqt -4k60 -qws\n";
        return 0;
    }

    BreakpadHandler::instance().setDumpPath("/opt");

    int ret = 0;
    qDebug() << "vapiInitialize start";
    ret = MainWindow::vapiInitialize(res);
    qDebug() << "vapiInitialize end, ret:" << ret;

    tde_fb_init();

    qDebug() << "QApplication initialize begin";
    MsApplication a(argc, argv);
    qDebug() << "QApplication initialize end";
    QWSServer::setBackground(Qt::transparent);

    MyInputMethod inputMethod;
    QWSServer::setCurrentInputMethod(&inputMethod);

    qDebug() << "MainWindow initialize begin";
    MainWindow w;
    qDebug() << "MainWindow initialize end";
    w.show();

    a.exec();

    tde_fb_uninit();

    return 0;
}
