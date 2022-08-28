#include "dialog.h"
#include <QThreadPool>
#include <QThread>
#include "workthread.h"
#include <QDebug>


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    m_server = new TcpServer(this);
    connect(m_server,&TcpServer::newDescriptor,this,[=](qintptr handle){
        WorkThread *thread = new WorkThread(handle);
        QThreadPool::globalInstance()->start(thread);
    });
    qDebug() << "mainID" << QThread::currentThreadId();
}

Dialog::~Dialog()
{

}
