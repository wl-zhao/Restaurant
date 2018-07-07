#include "server.h"

Server::Server(QObject*parent,quint16 port):QTcpServer(parent),m_port(port)
{
    listen(QHostAddress::Any,m_port);
    connect(this,SIGNAL(newConnection()),this,SLOT(acceptConnection()));
}
void Server::onSend()
{
    for(int i=0;i<m_sockets.count();++i)
    {
        m_sockets[i]->write(m_data.toLatin1());
        connect(m_sockets[i],SIGNAL(disconnected()),m_sockets[i],SLOT(deleteLater()));
    }
}
void Server::acceptConnection()
{     //当有客户来访时将tcpSocket接受tcpServer建立的socket
    while(hasPendingConnections())
    {
        QTcpSocket*socket=nextPendingConnection();
        connect(socket,&QTcpSocket::readyRead,[=]()
        {
            onSend();
        });
        connect(socket,&QTcpSocket::disconnected,[=]()
        {
            m_sockets.removeOne(socket);
            socket->deleteLater();
        });
        m_sockets.append(socket);
    }
}
