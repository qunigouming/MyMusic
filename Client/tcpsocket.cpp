#include "tcpsocket.h"
#include <QDebug>
#include <QThread>

TcpSocket::TcpSocket(QObject *parent, const QString ip) : QTcpSocket(parent),
    m_port(16888),
    m_ip(ip)
{
    connectToHost(QHostAddress(m_ip),m_port);
    connect(this,&TcpSocket::readyRead,this,&TcpSocket::recvmsg);
}

TcpSocket::~TcpSocket(){
//    delete m_MusicData;
//    m_MusicData = nullptr;
}

//发送信息给服务器
void TcpSocket::sendmsg(TcpMSGType type,QString MusicName)
{
    QByteArray msg;         //发送给服务器的数据
    switch(type){
        case TcpMSGType::REGIST_REQUEST:{
            //qDebug() << "接收了一个注册请求";
            QDataStream in(&msg,QIODevice::WriteOnly);
            in << TcpMSGType::REGIST_REQUEST << m_name << m_password;
            write(msg);
            break;
        }
        //登录请求，客户端拿消息类型和用户名密码
        case TcpMSGType::LOGIN_REQUEST:{
            //qDebug() << "接收了应该登录请求";
            QDataStream in(&msg,QIODevice::WriteOnly);
            in << TcpMSGType::LOGIN_REQUEST << m_name << m_password;
            write(msg);
            break;
        }
        //音乐播放请求
        case TcpMSGType::MUSICOPEN_REQUEST: {
            //qDebug() << "接收了一个音乐播放请求";
            QDataStream in(&msg,QIODevice::WriteOnly);
            in << TcpMSGType::MUSICOPEN_REQUEST << MusicName;
            write(msg);
            break;
        }
        //音乐列表请求
        case TcpMSGType::MUSICLIST_REQUEST:{
            //qDebug() << "接收了一个音乐列表请求";
            QDataStream in(&msg,QIODevice::WriteOnly);
            in << TcpMSGType::MUSICLIST_REQUEST;
            write(msg);
            break;
        }
    default:
        break;
    }
}

//收到服务器信息给界面
void TcpSocket::recvmsg(){
    QByteArray array;
    static TcpMSGType type;
    static uint Msglen = 0;
    if (Msglen == 0){        //类型长度只读取一次
        array = read(sizeof(TcpMSGType)+sizeof(uint));
        QDataStream outType(array);
        outType >> type >> Msglen;
        qDebug() << "正在获取消息类型和长度";
        qDebug() << "总字节:" << Msglen;
    }
    switch(type){
        case TcpMSGType::REGIST_REPLY:{
            array = readAll();
            QDataStream out(array);
            REGISTTYPE registType;
            out >> registType;
            Msglen = 0;
            emit RegistJudge(registType);
            break;
        }
        case TcpMSGType::LOGIN_REPLY:{
            array = readAll();
            QDataStream out(array);
            LOGINTYPE loginType;
            out >> loginType;
            Msglen = 0;
            emit LoginJudge(loginType);
            break;
        }
        case TcpMSGType::MUSICOPEN_REPLY:{
            static uint filelen = 0;           //当前获取的资源大小
            filelen += this->bytesAvailable();
            if (filelen == 0 || m_file == nullptr){
                m_file = new QFile("./Config/Music.mp3");
                if (!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate))
                    qDebug() << "文件打开失败";
                else qDebug() << "打开文件";
                if (filelen == 0)       //不为0表示发送粘包现象，可以不用直接跳出
                    return;
            }
            m_file->write(this->readAll());
            if (filelen >= Msglen){
                Msglen = 0;
                filelen = 0;
                m_file->close();
                m_file->deleteLater();
                m_file = nullptr;
                qDebug() << "文件全部接收完毕";
                emit MusicOpenRequest();        //通知主窗口读取文件
            }
            break;
        }
//        case TcpMSGType::MUSICOPEN_REPLY: {
//            static uint filelen = 0;
//            filelen += this->bytesAvailable();
//            if (filelen == 0){      //读取暂存区数据长度为0表示此次为数据包开头
//                return;
//            }
//            m_DataMsg.append(this->readAll());
//            if (filelen == Msglen){
//                Msglen = 0;
//                filelen = 0;
//                qDebug() << "文件全部接收完毕";
//                emit MusicOpenRequest(m_DataMsg);        //通知主窗口读取文件
//                //m_DataMsg.clear();
//            }
//            break;
//        }
        case TcpMSGType::MUSICLIST_REPLY:{
            array = readAll();
            QDataStream out(array);
            QStringList list;
            out >> list;
            qDebug() << list.size();
            Msglen = 0;
            emit MusicListRequest(list);
            break;
        }
    default:
        qDebug() << "Tcp类型错误";
        break;
    }
}

//从登录界面获取用户名和密码
void TcpSocket::setUserPwd(QString name, QString pwd)
{
    m_name = name;
    m_password = pwd;
}
