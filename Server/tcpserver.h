/*
 * tcp服务器，用来检查新套接字的连接
*/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include "tcpsocket.h"

#define CONNECT_MAX 1000        //最大连接数

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    static TcpServer &getInstance();          //获取实例对象

protected:
    virtual void incomingConnection(qintptr handle);


signals:
    void newDescriptor(qintptr);

private:
    unsigned short m_port;        //端口
};

#endif // TCPSERVER_H
