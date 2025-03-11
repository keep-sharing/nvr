#include "MyTcpSocket.h"
#include "MyDebug.h"

#include <QRegExp>
#include <QStringList>
#include <QTextStream>

#include <QDir>
#include <QFileInfoList>

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

const int ALGIN = 512;

MyTcpSocket::MyTcpSocket(int socketDescriptor, QObject *parent) :
    QTcpSocket(parent)
{
    setSocketDescriptor(socketDescriptor);
    connect(this, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(this, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(this, SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten(qint64)));

    m_bufferSize = (1024 << 10);
    m_diskbuffer = (char *)malloc(m_bufferSize + ALGIN);
    m_alginBuffer = (char *)(((uintptr_t)m_diskbuffer + (ALGIN - 1))&~((uintptr_t)(ALGIN - 1)));

    moveToThread(&m_thread);
    m_thread.start();
}

MyTcpSocket::~MyTcpSocket()
{
    m_thread.quit();
    m_thread.wait();

    free(m_diskbuffer);
}

void MyTcpSocket::sendFile()
{
    if (m_diskfd < 0) {
        return;
    }

    ssize_t retRead = pread64(m_diskfd, m_alginBuffer, m_bufferSize, m_baseOffset + m_offset);
    qMsDebug() << QString("pread, index:%1, offset:0x%2, ret:%3").arg(m_sendIndex).arg(m_baseOffset + m_offset, 0, 16).arg(retRead);
    if (retRead != m_bufferSize) {
        qMsWarning() << QString("pread error, index:%1, offset:0x%2, error:%3").arg(m_sendIndex).arg(m_baseOffset + m_offset, 0, 16).arg(strerror(errno));
    }
    qint64 retWrite = write(m_alginBuffer, m_bufferSize);
    if (retWrite != m_bufferSize) {
        qMsWarning() << QString("write error, index:%1, offset:0x%2, error:%3").arg(m_sendIndex).arg(m_baseOffset + m_offset, 0, 16).arg(strerror(errno));
    }

    m_offset += m_bufferSize;
    m_sendIndex++;

    if (m_offset >= m_fileSize) {
        if (m_diskfd >= 0) {
            ::close(m_diskfd);
            m_diskfd = -1;
        }
        close();
        emit clientDisconnected(socketDescriptor());
    }
}

void MyTcpSocket::onDisconnected()
{
    qMsWarning() << "disconnected";
    emit clientDisconnected(socketDescriptor());
}

void MyTcpSocket::onReadyRead()
{
    QByteArray data = readAll();

    static QRegExp rx(R"(GET /(.*)\?(.*) HTTP)");
    if (rx.indexIn(QString(data)) != -1) {
        QString type = rx.cap(1);
        QString params = rx.cap(2);
        QStringList paramsList = params.split("&");
        QMap<QString, QString> paramsMap;
        for (int i = 0; i < paramsList.size(); ++i) {
            QStringList list = paramsList.at(i).split("=");
            if (list.size() == 2) {
                QString key = list.at(0);
                QString value = list.at(1);
                paramsMap.insert(key, value);
            }
        }
        if (type == QString("backup_disk_raw_data")) {
            QString port = paramsMap.value("port");
            m_baseOffset = paramsMap.value("offset").toULongLong();

            QString fileName = QString("disk_%1_%2").arg(port).arg(m_baseOffset);
            QTextStream stream(this);
            stream << "HTTP/1.0 200 Ok\r\n";
            stream << "Content-Description: File Transfer\r\n";
            stream << "Content-Type: application/octet-stream\r\n";
            stream << QString("Content-Disposition: attachment; filename=%1\r\n").arg(fileName);
            stream << QString("Content-Length: %1\r\n").arg(m_fileSize);
            stream << "\r\n";
            stream.flush();

            QDir dir("/dev");
            QString diskFile;
            QFileInfoList fileInfoList = dir.entryInfoList(QStringList("MF_local_*"), QDir::Files);
            for (int i = 0; i < fileInfoList.size(); ++i) {
                const auto &info = fileInfoList.at(i);
                if (info.fileName().startsWith(QString("MF_local_%1").arg(port))) {
                    diskFile = QString("/dev/%1").arg(info.fileName());
                }
            }
            if (diskFile.isEmpty()) {
                qMsWarning() << QString("disk not found with port:%1").arg(port);
            } else {
                m_diskfd = ::open(diskFile.toStdString().c_str(), O_RDWR | O_DIRECT | O_LARGEFILE | O_CREAT | O_CLOEXEC, 0644);
                if (m_diskfd < 0) {
                    qMsWarning() << QString("error to open:%1").arg(diskFile);
                }
            }
        }
    }
}

void MyTcpSocket::onBytesWritten(qint64)
{
    qint64 bytes = bytesToWrite();
    if (bytes <= 0) {
        sendFile();
    }
}
