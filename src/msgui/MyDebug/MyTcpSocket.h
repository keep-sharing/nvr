#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QThread>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT

public:
    explicit MyTcpSocket(int socketDescriptor, QObject *parent = nullptr);
    ~MyTcpSocket() override;

private:
    void sendFile();

signals:
    void clientDisconnected(int socketDescriptor);

private slots:
    void onDisconnected();
    void onReadyRead();
    void onBytesWritten(qint64 size);

private:
    QThread m_thread;

    int m_diskfd = -1;
    quint64 m_baseOffset = 0;
    quint64 m_offset = 0;
    int m_sendIndex = 0;
    quint64 m_fileSize = (1 << 30);

    qint64 m_bufferSize = 0;
    char *m_diskbuffer = nullptr;
    char *m_alginBuffer = nullptr;
};

#endif // MYTCPSOCKET_H
