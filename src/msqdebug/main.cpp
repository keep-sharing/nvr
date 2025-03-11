#include <QApplication>
#include <QFile>
#include <QLocalSocket>
#include <QStringList>
#include <QTextStream>
#include <QtDebug>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStringList arguments = a.arguments();
    if (arguments.size() > 1) {
        QString commond = arguments.at(1);
        if (commond.startsWith("-h")) {
            QFile file(QString("%1/msqdebug.help").arg(a.applicationDirPath()));
            if (file.open(QFile::ReadOnly)) {
                QString text = file.readAll();
                text.replace(R"(\033)", "\033");
                qDebug() << qPrintable(text);
                //fprintf(stderr, "\033[32m%s\033[0m\n", QString(file.readAll()).toStdString().c_str());
                return 0;
            }
        }
    }

    QLocalSocket socket;
    socket.connectToServer("msqdebug");
    if (socket.waitForConnected()) {
        QByteArray ba;
        QDataStream stream(&ba, QIODevice::WriteOnly);
        for (int i = 1; i < arguments.size(); ++i) {
            stream << arguments.at(i);
        }
        socket.write(ba);
        if (socket.waitForBytesWritten()) {
        }
    }

#if 1
    return 0;
#else
    MainWindow w;
    w.show();
    return a.exec();
#endif
}
