#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    m_port = 16888;
    listen(QHostAddress::Any,m_port);       //监听端口
}

TcpServer &TcpServer::getInstance()
{
    static TcpServer instance;
    return instance;
}

//有连接时触发此函数
void TcpServer::incomingConnection(qintptr handle)
{
    emit newDescriptor(handle);
}
