#include "RunnableDiskRawDataBackup.h"
#include "MyDebug.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define writeLog(text) \
    do {\
        qMsDebug() << text;\
        stream << QDateTime::currentDateTime().toString("[yyyy-MM-dd HH:mm:ss.zzz]");\
        stream << text;\
        stream << "\r\n";\
    } while (0)

RunnableDiskRawDataBackup::RunnableDiskRawDataBackup(int port, quint64 fileOffset, const QString &dstPath, QObject *originatingObject, const QString &callBack) :
    QRunnable(),
    m_port(port),
    m_fileOffset(fileOffset),
    m_dstPath(dstPath),
    m_originatingObject(originatingObject),
    m_callBack(callBack)
{

}

RunnableDiskRawDataBackup::~RunnableDiskRawDataBackup()
{

}

void RunnableDiskRawDataBackup::run()
{
    do {
        QString logText;
        QString logFile = QString("%1/disk_raw.log").arg(m_dstPath);
        QFile file(logFile);
        if (!file.open(QFile::Append)) {
            qMsCritical() << "open log file error:" << file.errorString();
            break;
        }
        QTextStream stream(&file);

        logText = QString("backup disk raw data begin, port:%1, 0x%2").arg(m_port).arg(m_fileOffset, 0, 16);
        writeLog(logText);

        QString diskFile;
        QString dstFile;

        QDir dir("/dev");
        QFileInfoList fileInfoList = dir.entryInfoList(QStringList("MF_local_*"), QDir::Files);
        for (int i = 0; i < fileInfoList.size(); ++i) {
            const auto &info = fileInfoList.at(i);
            if (info.fileName().startsWith(QString("MF_local_%1").arg(m_port))) {
                diskFile = QString("/dev/%1").arg(info.fileName());
            }
        }
        if (diskFile.isEmpty()) {
            logText = QString("disk not found with port:%1").arg(m_port);
            writeLog(logText);
        } else {
            int srcfd = open(diskFile.toStdString().c_str(), O_RDWR | O_DIRECT | O_LARGEFILE | O_CREAT | O_CLOEXEC, 0644);
            if (srcfd < 0) {
                logText = QString("error to open:%1").arg(diskFile);
                writeLog(logText);
            }
            //
            QFileInfo info(QString("%1/disk_%2_%3.raw").arg(m_dstPath).arg(m_port).arg(m_fileOffset));
            int fileIndex = 2;
            while (info.exists()) {
                info = QString("%1/disk_%2_%3_%4.raw").arg(m_dstPath).arg(m_port).arg(m_fileOffset).arg(fileIndex);
                fileIndex++;
            }
            dstFile = info.filePath();
            int dstfd = open(dstFile.toStdString().c_str(), O_RDWR | O_DIRECT | O_LARGEFILE | O_CREAT | O_CLOEXEC, 0644);
            if (dstfd < 0) {
                logText = QString("error to open:%1").arg(m_dstPath);
                writeLog(logText);
            }
            //
            if (srcfd >= 0 && dstfd >= 0) {
                const int ALGIN = 512;
                ssize_t bufferSize = (1024 << 10);
                char *buffer = (char *)malloc(bufferSize + ALGIN);
                char *alginBuffer = (char *)(((uintptr_t)buffer + (ALGIN - 1))&~((uintptr_t)(ALGIN - 1)));
                for (int i = 0; i < 1024; ++i) {
                    quint64 offset = (quint64)bufferSize * i;
                    ssize_t retRead = pread64(srcfd, alginBuffer, bufferSize, m_fileOffset + offset);
                    logText = QString("pread, index:%1, offset:0x%2, ret:%3").arg(i).arg(m_fileOffset + offset, 0, 16).arg(retRead);
                    writeLog(logText);
                    if (retRead != bufferSize) {
                        logText = QString("pread error, index:%1, offset:0x%2, error:%3").arg(i).arg(m_fileOffset + offset, 0, 16).arg(strerror(errno));
                        writeLog(logText);
                        break;
                    } else {
                        ssize_t retWrite = pwrite64(dstfd, alginBuffer, bufferSize, offset);
                        if (retWrite != bufferSize) {
                            logText = QString("pwrite error, index:%1, offset:0x%2, error:%3").arg(i).arg(offset, 0, 16).arg(strerror(errno));
                            writeLog(logText);
                            break;
                        }
                    }
                }
                free(buffer);
            }
            if (srcfd >= 0) {
                close(srcfd);
            }
            if (dstfd >= 0) {
                close(dstfd);
            }
        }
        logText = QString("backup disk raw data end, path:%1").arg(dstFile);
        writeLog(logText);
    } while (0);

    //
    QMetaObject::invokeMethod(m_originatingObject, m_callBack.toLocal8Bit().data(), Qt::QueuedConnection);
}
