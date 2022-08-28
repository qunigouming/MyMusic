#include "dbope.h"
#include <QDebug>

dbope::dbope(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName("localhost");
    m_db.setUserName("root");
    m_db.setPassword("123");
    m_db.setDatabaseName("music");
    if (!m_db.open()){
        qDebug() << "数据库打开失败" << m_db.lastError().text();
    }
}

dbope::~dbope()
{
    m_db.close();
}

dbope &dbope::getInstance()
{
    static dbope db;
    return db;
}

//查询指定的用户名和密码,有则返回true，无返回false
bool dbope::isuserpwd(const QString name, const QString pwd)
{
    QSqlQuery query;
    bool flag = query.exec(QString("select username, password from user where username='%1' && password='%2'").arg(name).arg(pwd));
    if (!flag)  qDebug() << "function isuserpwd: 用户名和密码有效性查询错误" << query.lastError().text();
    flag = query.next();        //查询到了记录说明有该用户，返回true
    //用户名和密码一致说明已有该用户
    if (flag){
        return true;
    }
    return false;
}

//注册判断
/*
 * 返回0  已有该用户
 * 返回-1  用户名相同
 * 返回1  创建成功
*/
REGISTTYPE dbope::isregister(const QString name, const QString pwd)
{
    bool flag;
    QSqlQuery query;
    flag = isuserpwd(name,pwd);
    if (flag) return REGISTTYPE::HAVEUSER;
    //查询是否有相同的用户名
    flag = query.exec(QString("select * from user where username='%1'").arg(name));
    if (!flag)  qDebug() << "function isregister: 用户名查询错误" << query.lastError().text();
    flag = query.next();
    //有相同的用户名，返回-1
    if (flag)   return REGISTTYPE::EQUAL_USERNAME;
    flag = QSqlDatabase::database().transaction();
    if (!flag)  qDebug() << "function isregister: 事务开启失败";
    flag = query.exec(QString("insert into user(username,password,status) value('%1','%2',1)").arg(name,pwd));
    if (!flag) qDebug() << "function isregister: 用户数据插入失败" << query.lastError().text();
    //插入失败不会影响到数据库数据，因此可以直接提交
    flag = QSqlDatabase::database().commit();
    if (!flag)  qDebug() << "function isregister: 事务提交失败";
    return REGISTTYPE::OK;
}

//登录时查看是否在线
/*
 * 返回0  无该用户
 * 返回-1 在线
 * 返回1  成功
 */
LOGINTYPE dbope::isOnline(const QString name, const QString pwd)
{
    bool flag;
    QSqlQuery query;
    flag = isuserpwd(name,pwd);
    //无该用户，返回0
    if (!flag)  return LOGINTYPE::NOTUSER;
    //查询是否在线
    flag = query.exec(QString("select * from user where username='%1' && password='%2' && status=0").arg(name).arg(pwd));
    if (!flag)  qDebug() << "function isOnline: 在线状态查询失败" << query.lastError();
    flag = query.next();
    //没记录说明在线
    if (!flag)  return LOGINTYPE::LOGIN;
    flag = QSqlDatabase::database().transaction();
    if (!flag)  qDebug() << "function isOnline: 事务开启失败";
    flag = query.exec(QString("update user set status=1 where username='%1' && password='%2'").arg(name).arg(pwd));
    if (!flag)  qDebug() << "function isOnline: 状态更新失败";
    flag = QSqlDatabase::database().commit();
    if (!flag)  qDebug() << "function isOnline: 事务提交失败";
    return LOGINTYPE::OK;
}

//获取指定音乐路径
QString dbope::isMusicExist(const QString name)
{
    bool flag;
    QSqlQuery query;
    flag = query.exec(QString("select music_path from music_list where music_name='%1'").arg(name));
    if (!flag)  qDebug() << "function isMusicExist: 音乐路径查询错误" << query.lastError().text();
    QString path;
    while(query.next())
        path = query.value(0).toString();
    qDebug() << path;
    return path;
}

QStringList dbope::MusicList()
{
    bool flag;
    QSqlQuery query;
    flag = query.exec(QString("select music_name from music_list"));
    if (!flag)  qDebug() << "function MusicList: 音乐列表查询失败" << query.lastError().text();
    QStringList musiclist;
    while(query.next())
        musiclist.append(query.value(0).toString());
    return musiclist;
}

//离线
void dbope::isOffline(const QString name)
{
    bool flag;
    QSqlQuery query;
    flag = QSqlDatabase::database().transaction();
    if (!flag)  qDebug() << "function isOffline: 事务开启失败";
    flag = query.exec(QString("update user set status=0 where username='%1'").arg(name));
    if (!flag)  qDebug() << "function isOffline: 状态更新失败" << query.lastError().text();
    flag = QSqlDatabase::database().commit();
    if (!flag)  qDebug() << "function isOffline: 事务提交失败";
}
