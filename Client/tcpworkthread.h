#ifndef TCPWORKTHREAD_H
#define TCPWORKTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "tcpsocket.h"

class TcpWorkThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpWorkThread(QString ip = "", QObject *parent = nullptr);
    void sendmsg(TcpMSGType type,QString MusicName = "");       //发送信息给服务器
    void setUserPwd(QString &name, QString &pwd);

signals:
    void RegistJudge(REGISTTYPE);           //从服务器接收到注册回复时发送注册信号
    void LoginJudge(LOGINTYPE);             //登录信号
    void MusicOpenRequest();                //音乐播放请求
    void MusicListRequest(QStringList);       //音乐列表请求

    void on_TcpWorkThread_sendmsg(TcpMSGType type, QString MusicName);
    void on_TcpWorkThread_setUserPwd(QString name, QString pwd);

public slots:

private:
    QThread* workThread;
    QMutex mutex;               //给竞争资源加的互斥锁
    TcpSocket *m_tcp;           //通信套接字
};

#endif // TCPWORKTHREAD_H
