#include "MyTcpServer.h"

MyTcpServer::MyTcpServer(QObject *parent) :
    QTcpServer(parent)
{

}

void MyTcpServer::incomingConnection(int socketDescriptor)
{
    MyTcpSocket *socket = new MyTcpSocket(socketDescriptor);
    connect(socket, SIGNAL(clientDisconnected(int)), this, SLOT(onClientDisconnected(int)));
    m_clientMap.insert(socketDescriptor, socket);
}

void MyTcpServer::onClientDisconnected(int socketDescriptor)
{
    MyTcpSocket *socket = m_clientMap.value(socketDescriptor);
    if (socket) {
        delete socket;
        m_clientMap.remove(socketDescriptor);
    }
}
