#include "RunnableDiskRawDataImport.h"
#include "MyDebug.h"
#include <QDir>

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

RunnableDiskRawDataImport::RunnableDiskRawDataImport(int port, const QString &srcPath) :
    QRunnable(),
    m_port(port),
    m_srcPath(srcPath)
{

}

RunnableDiskRawDataImport::~RunnableDiskRawDataImport()
{

}

void RunnableDiskRawDataImport::run()
{
    qMsDebug() << QString("import disk raw data begin, port:%1, file:%2").arg(m_port).arg(m_srcPath);

    QFileInfo fileInfo(m_srcPath);
    if (!fileInfo.exists()) {
        qMsWarning() << "file not exists:" << fileInfo.filePath();
        return;
    }

    QString fileName = fileInfo.baseName();
    QStringList fileNameList = fileName.split("_", QString::SkipEmptyParts);
    if (fileNameList.size() < 3) {
        qMsWarning() << "error file name:" << fileInfo.fileName();
        return;
    }

    quint64 diskOffset = fileNameList.at(2).toULongLong();
    if (diskOffset == 0) {
        qMsWarning() << "error disk offset:" << fileNameList << diskOffset;
        return;
    }

    QString diskFile;
    QString importFile;

    QDir dir("/dev");
    QFileInfoList fileInfoList = dir.entryInfoList(QStringList("MF_local_*"), QDir::Files);
    for (int i = 0; i < fileInfoList.size(); ++i) {
        const auto &info = fileInfoList.at(i);
        if (info.fileName().startsWith(QString("MF_local_%1").arg(m_port))) {
            diskFile = QString("/dev/%1").arg(info.fileName());
        }
    }
    if (diskFile.isEmpty()) {
        qMsCritical() << "disk not found with port:" << m_port;
    } else {
        int diskfd = open(diskFile.toStdString().c_str(), O_RDWR | O_DIRECT | O_LARGEFILE | O_CREAT | O_CLOEXEC, 0644);
        if (diskfd < 0) {
            qMsCritical() << "error to open:" << diskFile;
        }
        //
        importFile = m_srcPath;
        int importfd = open(importFile.toStdString().c_str(), O_RDWR | O_DIRECT | O_LARGEFILE | O_CREAT | O_CLOEXEC, 0644);
        if (importfd < 0) {
            qMsCritical() << "error to open:" << importFile;
        }
        //
        if (diskfd >= 0 && importfd >= 0) {
            const int ALGIN = 512;
            ssize_t bufferSize = (1024 << 10);
            char *buffer = (char *)malloc(bufferSize + ALGIN);
            char *alginBuffer = (char *)(((uintptr_t)buffer + (ALGIN - 1))&~((uintptr_t)(ALGIN - 1)));
            for (int i = 0; i < 1024; ++i) {
                quint64 offset = (quint64)bufferSize * i;
                ssize_t retRead = pread64(importfd, alginBuffer, bufferSize, offset);
                //qMsDebug() << QString("pread64, index:%1, offset:0x%2, ret:%3").arg(i).arg(offset, 0, 16).arg(retRead);
                if (retRead != bufferSize) {
                    qMsCritical() << QString("pread64 error, index:%1, offset:0x%2, error:%3").arg(i).arg(offset, 0, 16).arg(strerror(errno));
                    break;
                } else {
                    ssize_t retWrite = pwrite64(diskfd, alginBuffer, bufferSize, diskOffset + offset);
                    qMsDebug() << QString("pwrite64, index:%1, offset:0x%2, ret:%3").arg(i).arg(diskOffset + offset, 0, 16).arg(retRead);
                    if (retWrite != bufferSize) {
                        qMsCritical() << QString("pwrite64 error, index:%1, offset:0x%2, error:%3").arg(i).arg(diskOffset + offset, 0, 16).arg(strerror(errno));
                        break;
                    }
                }
            }
            free(buffer);
        }
        if (diskfd >= 0) {
            close(diskfd);
        }
        if (importfd >= 0) {
            close(importfd);
        }
    }
    qMsDebug() << QString("backup disk raw data end, path:%1").arg(importFile);
}
