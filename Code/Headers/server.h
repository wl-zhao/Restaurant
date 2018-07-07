#ifndef SERVER_H
#define SERVER_H
#include<QtNetwork>
#include<QtNetwork/QTcpSocket>
#include<QtNetwork/QTcpServer>
#include <QList>

class Server:public QTcpServer
{
    Q_OBJECT
public:
    Server(QObject*parent=0,quint16 port=0);
    void setData(QString data){m_data=data;}
    void Listen();
public slots:
    void onSend();
protected slots:
    void acceptConnection(); //接受客户端连接
private:
    QList<QTcpSocket*> m_sockets;
    QString m_data;
    quint16 m_port;
signals:
    void newListen();
};

#endif // SERVER_H
