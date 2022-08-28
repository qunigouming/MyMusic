#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QTcpSocket>
#include <QList>
#include "dbope.h"
#include "protocol.h"
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

class TcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpSocket();
    ~TcpSocket();
    void clientoffline();       //处理客户端下线


public slots:
    void recvmsg();

private:
    QString m_name;         //该套接字连接的用户名
};

#endif // TCPSOCKET_H
