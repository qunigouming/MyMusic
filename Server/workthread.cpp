#include "workthread.h"
#include <QThread>

WorkThread::WorkThread(qintptr handle,QObject *parent) : QObject(parent), QRunnable()
{
    setAutoDelete(true);        //让线程池自动删除资源
    m_tcpsocket = new TcpSocket;
    m_tcpsocket->setSocketDescriptor(handle);
    connect(m_tcpsocket,&TcpSocket::disconnected,this,&WorkThread::over);
}

WorkThread::~WorkThread()
{
    qDebug() << "WorkThread的析构函数调用";
}

void WorkThread::run()
{
    qDebug() << "WorkThreadID" << QThread::currentThreadId();
    while(flag);        //不进行循环run()函数会直接跳出
}

void WorkThread::over()
{
    flag = false;
    qDebug() << "socket发出离线信号";
    m_tcpsocket->close();
    m_tcpsocket->deleteLater();
}
