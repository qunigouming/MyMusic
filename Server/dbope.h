#ifndef DBOPE_H
#define DBOPE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <protocol.h>

class dbope : public QObject
{
    Q_OBJECT
public:
    explicit dbope(QObject *parent = nullptr);
    ~dbope();
    static dbope &getInstance();

    //密码判断入口时密码必须转为MD5加密
    bool isuserpwd(const QString name, const QString pwd);      //对用户名和密码的有效性判断
    REGISTTYPE isregister(const QString name, const QString pwd);     //注册时查看是否存在于数据库，并对数据进行校验
    LOGINTYPE isOnline(const QString name, const QString pwd);   //登入时查看是否在线
    QString isMusicExist(const QString name);                  //选择音乐时返回该音乐的本地地址
    QStringList MusicList();                                 //客户端展示音乐列表时返回列表中的所有音乐名
    void isOffline(const QString name);                     //离线时修改数据库

signals:

public slots:

private:
    QSqlDatabase m_db;
};

#endif // DBOPE_H
