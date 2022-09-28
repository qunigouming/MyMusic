#include "tcpworkthread.h"
#include <QDebug>

TcpWorkThread::TcpWorkThread(QString ip,QObject *parent) : QObject(parent)
{
    workThread = new QThread();
    m_tcp = new TcpSocket(nullptr,ip);
    m_tcp->moveToThread(workThread);
    connect(workThread,&QThread::finished,m_tcp,&TcpSocket::deleteLater);           //线程退出资源释放
    connect(workThread,&QThread::finished,workThread,&QThread::deleteLater);
    connect(m_tcp,&TcpSocket::RegistJudge,this,[this](REGISTTYPE type) {
        emit RegistJudge(type);
    });
    connect(m_tcp,&TcpSocket::LoginJudge,this,[this](LOGINTYPE type) {
        emit LoginJudge(type);
    });
    connect(m_tcp,&TcpSocket::MusicOpenRequest,this,[this] {
        emit MusicOpenRequest();
    });
    connect(m_tcp,&TcpSocket::MusicListRequest,this,[this](QStringList list) {
        emit MusicListRequest(list);
    });
    connect(this,&TcpWorkThread::on_TcpWorkThread_setUserPwd,m_tcp,&TcpSocket::setUserPwd);
    connect(this,&TcpWorkThread::on_TcpWorkThread_sendmsg,m_tcp,&TcpSocket::sendmsg);
    workThread->start();            //启动子线程
}

//封装设置用户密码
void TcpWorkThread::setUserPwd(QString &name, QString &pwd)
{
    emit on_TcpWorkThread_setUserPwd(name,pwd);
}

//封装发送消息
void TcpWorkThread::sendmsg(TcpMSGType type, QString MusicName)
{
    emit on_TcpWorkThread_sendmsg(type,MusicName);
}
