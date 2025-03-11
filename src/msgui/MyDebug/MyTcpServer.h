#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include "MyTcpSocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);

protected:
    void incomingConnection(int socketDescriptor) override;

signals:

private slots:
    void onClientDisconnected(int socketDescriptor);

private:
    QMap<int, MyTcpSocket *> m_clientMap;
};

#endif // MYTCPSERVER_H
