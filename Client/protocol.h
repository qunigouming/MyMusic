#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QCryptographicHash>           //包含MD5算法库
#include <QSqlQuery>
#include <QObject>

#define PASSWORDMINSIZE    4           //密码最短长度

//数据传输枚举
enum class TcpMSGType{
    REGIST_REQUEST,             //注册请求
    REGIST_REPLY,               //注册回复
    LOGIN_REQUEST,              //登录请求
    LOGIN_REPLY,                //登录回复
    MUSICOPEN_REQUEST,          //音乐播放请求
    MUSICOPEN_REPLY,            //音乐播放回复
    MUSICLIST_REQUEST,          //音乐列表请求
    MUSICLIST_REPLY             //音乐列表回复
};

//注册判断枚举
enum class REGISTTYPE{
    HAVEUSER,                   //拥有用户
    EQUAL_USERNAME,             //用户名相同
    OK                          //完毕
};

//登录判断枚举
enum class LOGINTYPE{
    NOTUSER,                    //没有该用户
    LOGIN,                      //用户在线
    OK                          //完毕
};

//列表播放模式
enum class LISTPLAYER{
    LOOP,                       //列表循环
    SINGLELOOP,                 //单曲循环
    RANDOM                      //随机播放
};

//解析配置文件
class ConfigFile : public QObject{
    Q_OBJECT
public:
    ConfigFile(QObject* parent = nullptr);
    void WriteFile(QString Name, QString pwd, bool status);
    void ReadFile();

signals:
    void ReadSignal();          //发送读取完毕信号

public:
    bool m_status = false;
    QString m_userName;
    QString m_pwd;

private:
    QString toXOREncryptUncrypt(QString, const QChar);  //加解密算法
    void initDB();
    bool IsTableExist(QSqlQuery& query, QString table); //查询是否有指定表
    bool IsFieldExist(const QString Name);          //查询用户信息表内是否有指定用户名
    void InsertData(QString Name, const QString Pwd, const bool status);      //插入数据
    void QueryTable();
    void ModifyValue(const QString Name, const QString Pwd, const bool status);     //更新数据
    QString encrypt(QString str);
    QString decipher(QString str);
};

#endif // PROTOCOL_H
