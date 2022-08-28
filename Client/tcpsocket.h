#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include <QDataStream>
#include <QHostAddress>
#include <QFile>

class TcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpSocket(QObject *parent = nullptr);
    ~TcpSocket();
    void sendmsg(TcpMSGType type,QString MusicName = "");          //发送消息(向服务器写消息)
    void recvmsg();                         //读服务器消息
    void getUserPwd(QString &name, QString &pwd);

signals:
    void RegistJudge(REGISTTYPE);           //从服务器接收到注册回复时发送注册信号
    void LoginJudge(LOGINTYPE);             //登录信号
    void MusicOpenRequest();                //音乐播放请求
    void MusicListRequest(QStringList);       //音乐列表请求

public slots:

private:
    unsigned short m_port;        //服务器端口
    QString m_ip;         //服务器ip
    QString m_name;
    QString m_password;
    //QByteArray m_DataMsg;        //数据消息(通常是该Tcp类读来的消息)
    QFile *m_file;      //音乐资源文件指针
};

#endif // TCPSOCKET_H
