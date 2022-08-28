#include "tcpsocket.h"

TcpSocket::TcpSocket()
{
    connect(this,&TcpSocket::readyRead,this,&TcpSocket::recvmsg);        //读取信号连接
    connect(this,&TcpSocket::disconnected,this,&TcpSocket::clientoffline);   //关闭信号连接
}

//关闭窗口时，可能会对数据库进行操作，因此进行析构
TcpSocket::~TcpSocket()
{
    clientoffline();
}

void TcpSocket::clientoffline()
{
    dbope::getInstance().isOffline(m_name);
}

void TcpSocket::recvmsg()
{
    QByteArray array = readAll();
    TcpMSGType msgtype;
    QDataStream in(array);
    in.setVersion(QDataStream::Qt_5_15);
    in >> msgtype;
    qDebug() << "收到了客户端发来的消息";
    switch(msgtype){
    //注册请求
        case TcpMSGType::REGIST_REQUEST:{
            //注册客户端拿用户名和密码，服务器返回应答消息和注册状态
            //该消息需要类型： 消息类型 用户名 密码
            //该消息类型：消息类型 消息长度 注册状态
            QString username, password;
            in >> username >> password;
            REGISTTYPE status = dbope::getInstance().isregister(username, password);
            if (status == REGISTTYPE::OK)   m_name = username;
            QByteArray sendmsg;         //发送给客户端的消息
            QDataStream out(&sendmsg,QIODevice::WriteOnly);
            out << TcpMSGType::REGIST_REPLY << uint(0) << status;
            write(sendmsg);
            break;
        }
        //登录请求
        case TcpMSGType::LOGIN_REQUEST:{
            //客户端拿用户名和密码，服务器返回应答消息和登录状态
            //该消息需要格式：消息类型 用户名 密码
            //该消息格式： 消息类型 消息长度 登录状态
            QString username, password;
            in >> username >> password;
            LOGINTYPE status = dbope::getInstance().isOnline(username,password);
            if (status == LOGINTYPE::OK)    m_name = username;      //获取用户名
            QByteArray sendmsg;     //发送给客户端的消息
            QDataStream out(&sendmsg,QIODevice::WriteOnly);
            out << TcpMSGType::LOGIN_REPLY << uint(0) << status;
            write(sendmsg);
            break;
        }
        //音乐播放请求
        case TcpMSGType::MUSICOPEN_REQUEST:{
            //客户端拿音乐名称，服务器返回音乐数据
            //该消息需要格式： 消息类型 音乐名称
            //该消息格式： 先发送 消息类型，消息大小  再发送消息(文件数据)
            //获取音乐名称
            QString MusicName;
            in >> MusicName;
            QString MusicPath = dbope::getInstance().isMusicExist(MusicName);
            qDebug() << MusicName << MusicPath;
            QFile file(MusicPath);
            if (file.open(QIODevice::ReadOnly)){
                QByteArray msg;
                QDataStream se(&msg,QIODevice::WriteOnly);
                uint filelen = file.size();     //获取文件总大小
                se << TcpMSGType::MUSICOPEN_REPLY << filelen;
                write(msg);
                while(!file.atEnd()){
                    QByteArray line;
                    QDataStream out(&line,QIODevice::WriteOnly);
                    line.append(file.read(65534));          //疑点，65535 QMediaPlayer不能播放,65534可以
                    uint Msglen = line.size();  //获取当前块消息长度
                    //qDebug() << Msglen;
                    out.writeRawData(line,Msglen);
                    write(line);
                    //waitForBytesWritten();
                }
                file.close();
                qDebug() << "音乐播放消息,发送给客户端的文件大小:" << filelen;
//                uint filelen = file.size();
//                out << filelen;     //写入文件大小
//                QByteArray byteMusic = file.readAll();
//                out.writeRawData(byteMusic,byteMusic.size());
//                file.close();
//                int Binlen = write(sendmsg);
//                qDebug() << "音乐播放消息,发送给客户端的字节数:" << Binlen;
//                qDebug() << "音乐播放消息,总字节数:" << filelen;
            }else qDebug() << "打开音乐文件失败";
            break;
        }
        //音乐列表请求
        case TcpMSGType::MUSICLIST_REQUEST:{
            //该消息需要格式：消息类型
            //该消息格式 消息类型 消息长度 音乐列表
            QStringList musicList = dbope::getInstance().MusicList();     //以','分隔列表中的数据,并组成字符串
            qDebug() << "音乐列表请求消息下的列表数据大小" << musicList.size();
            QByteArray sendmsg;     //发送的数据
            QDataStream out(&sendmsg,QIODevice::WriteOnly);
            uint musicListSize = 0;
            //获取字节大小
            for (QString i : musicList){
                musicListSize += i.toLocal8Bit().size();
            }
            qDebug() << musicListSize;
            out << TcpMSGType::MUSICLIST_REPLY << musicListSize << musicList;
            int len = write(sendmsg);
            qDebug() << "音乐列表请求消息下的已发送消息的大小" << len;
            break;
        }
        default:
            break;
    }
}
