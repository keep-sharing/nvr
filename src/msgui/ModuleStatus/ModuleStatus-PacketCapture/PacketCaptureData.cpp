#include "PacketCaptureData.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "mainwindow.h"
#include "MessageBox.h"
#include "MsApplication.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include <QDir>


PacketCaptureData *PacketCaptureData::self = nullptr;

PacketCaptureData::PacketCaptureData(QObject *parent)
    : QObject(parent)
{
    self = this;

    moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(started()), this, SLOT(onThreadStarted()));
    m_thread.start();
}

PacketCaptureData::~PacketCaptureData()
{
    self = nullptr;
}

PacketCaptureData *PacketCaptureData::instance()
{
    return self;
}

void PacketCaptureData::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

PacketCaptureData::State PacketCaptureData::state()
{
    QMutexLocker locker(&m_mutex);
    return m_state;
}

void PacketCaptureData::clear()
{
    m_ip.clear();
    m_port.clear();
    m_nic.clear();
    m_path.clear();
}

void PacketCaptureData::startCapture(int diskPort)
{
    QMutexLocker locker(&m_mutex);

    if (m_state == StateWorking) {
        return;
    }

    m_diskPort = diskPort;
    m_state = StateWorking;

    QMetaObject::invokeMethod(this, "onStartCapture", Qt::QueuedConnection);
}

void PacketCaptureData::stopCapture()
{
    QMutexLocker locker(&m_mutex);

    if (m_state == StateStoped) {
        return;
    }
    m_state = StateStoped;

    QMetaObject::invokeMethod(this, "onStopCapture", Qt::QueuedConnection);
}

QString PacketCaptureData::ip()
{
    QMutexLocker locker(&m_mutex);
    return m_ip;
}

void PacketCaptureData::setIp(const QString &ip)
{
    QMutexLocker locker(&m_mutex);
    m_ip = ip;
}

QString PacketCaptureData::port()
{
    QMutexLocker locker(&m_mutex);
    return m_port;
}

void PacketCaptureData::setPort(const QString &port)
{
    QMutexLocker locker(&m_mutex);
    m_port = port;
}

QString PacketCaptureData::nic()
{
    QMutexLocker locker(&m_mutex);
    return m_nic;
}

void PacketCaptureData::setNic(const QString &nic)
{
    QMutexLocker locker(&m_mutex);
    m_nic = nic;
}

QString PacketCaptureData::path()
{
    QMutexLocker locker(&m_mutex);
    return m_path;
}

void PacketCaptureData::setPath(const QString &path)
{
    QMutexLocker locker(&m_mutex);
    m_path = path;
}

void PacketCaptureData::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_UDISK_OFFLINE:
        ON_RESPONSE_FLAG_UDISK_OFFLINE(message);
        break;
    }
}

void PacketCaptureData::ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message)
{
    QMutexLocker locker(&m_mutex);
    if (!message->data) {
        qMsWarning() << "data is null.";
        return;
    }
    int port = *((int *)message->data);
    if (StateWorking == m_state) {
        if (port == m_diskPort) {
            m_state = StateStoped;
            QMetaObject::invokeMethod(this, "onStopCapture", Qt::QueuedConnection);

            QString text = GET_TEXT("PACKETCAPTURE/109006", "The storage device does not exist, please check and try again.");
            QMetaObject::invokeMethod(this, "onShowMessage", Qt::QueuedConnection, Q_ARG(QString, text));
        }
    }
}

QString PacketCaptureData::currentLanName()
{
    QString text;
    if (nic() == "eth0") {
        text = "LAN1";
    } else if (nic() == "eth1") {
        text = "LAN2";
    } else {
        text = "LAN";
    }
    return text;
}

void PacketCaptureData::onThreadStarted()
{
    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    }
    m_timer->setInterval(1000 * 180);

    emit threadStarted();
}

void PacketCaptureData::onStartCapture()
{
    
}

void PacketCaptureData::onStopCapture()
{
    m_state = StateStoped;

    m_timer->stop();

    //
    QString cmd = QString("killall tcpdump > /dev/null");
    qMsDebug() << "PacketCaptureData:" << cmd;
    ms_system(cmd.toStdString().c_str());

    //延时1s，不然会出现The capture file appears to have been cut short in the middle of a packet.
    sleep(1);

    QFileInfoList fileInfoList = QDir(m_tempPath).entryInfoList(QDir::Files);
    for (const QFileInfo &info : fileInfoList) {
        ms_system(QString("cd %1;mv %2 %2.pcap > /dev/null").arg(m_tempPath).arg(info.fileName()).toStdString().c_str());
    }

    //
    cmd = QString("cd %1;tar -cvf packet.tar * > /dev/null").arg(m_tempPath);
    qMsDebug() << "PacketCaptureData:" << cmd;
    ms_system(cmd.toStdString().c_str());

    //
    int index = 2;
    QFileInfo info(QString("%1/packet.tar").arg(path()));
    while (info.exists()) {
        info.setFile(QString("%1/packet%2.tar").arg(path()).arg(index));
        index++;
    }

    //拷贝到U盘
    cmd = QString("cd %1;cp packet.tar \"%2\" > /dev/null").arg(m_tempPath).arg(info.absoluteFilePath());
    qMsDebug() << "PacketCaptureData:" << cmd;
    ms_system(cmd.toStdString().c_str());

    //删除临时文件
    cmd = QString("rm -r %1 > /dev/null").arg(m_tempPath);
    qMsDebug() << "PacketCaptureData:" << cmd;
    ms_system(cmd.toStdString().c_str());

    if (m_isTimeout) {
        QString text = GET_TEXT("PACKETCAPTURE/109010", "Packet capture is completed.");
        QMetaObject::invokeMethod(MessageBox::instance(), "showInformation", Qt::QueuedConnection, Q_ARG(QString, text));
    }

    sync();

    emit finished();
}

void PacketCaptureData::onTimeout()
{
    m_isTimeout = true;
    stopCapture();
}
