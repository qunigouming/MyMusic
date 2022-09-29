#include "protocol.h"
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

ConfigFile::ConfigFile(QObject* parent) : QObject(parent)
{
    initDB();
}

//写指定数据
//状态为true 插入数据      如果数据库有数据更新数据
//状态为false 更新状态     如果数据库有数据更新数据
void ConfigFile::WriteFile(QString Name, QString pwd, bool status)
{
    //加密数据
    //Name = encrypt(Name);
    pwd = encrypt(pwd);
    if (status) {
        if (IsFieldExist(Name))         //数据库有数据更新数据
            ModifyValue(Name, pwd, status);
        else                            //数据库没有数据，插入数据
            InsertData(Name,pwd,status);
    }
    else {
        qDebug() << "状态为:" << status << Name << pwd;
        if (IsFieldExist(Name)){         //数据库有数据更新数据
            qDebug() << "修改数据";
            ModifyValue(Name, pwd, status);
        }
    }
}

void ConfigFile::ReadFile()
{
    QueryTable();
    if (m_status) {
        QTimer::singleShot(200,this,[this]() { emit ReadSignal(); }); //延时触发信号,等待该对象创建完毕再发送信号
    }
}

QString ConfigFile::toXOREncryptUncrypt(QString str, const QChar key)
{
    for (int i = 0; i < str.size(); ++i)
        str[i] = str.at(i).toLatin1() ^ key.toLatin1();
    return str;
}

void ConfigFile::initDB()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./Config/config.db");
    if (!db.open())
        qDebug() << "Error: databases is not open" << db.lastError();
    QSqlQuery query;
    //没有表创建表并退出
    if (!IsTableExist(query,"userMsg")) {
        qDebug() << "创建表";
        bool flag = query.exec(QString("create table userMsg(UserId int primary key, auto_increment, UserName varchar(255), Password varchar(255), status boolean)"));
        if (!flag)  qDebug() << "ConfigFile使用initDB创建表失败" << query.lastError();
        return;
    }
    ReadFile();
}

//有指定表返回true
bool ConfigFile::IsTableExist(QSqlQuery &query, QString table)
{
    QString sql = QString("select * from sqlite_master where type='table' and name='%1'").arg(table);
    query.exec(sql);
    return query.next();
}

//有指定用户名数据返回true
bool ConfigFile::IsFieldExist(const QString name)
{
    QSqlQuery query;
    query.exec(QString("select * from userMsg where UserName='%1'").arg(name));
    return query.next();
}

//插入用户相关信息
void ConfigFile::InsertData(QString name, const QString pwd, const bool status)
{
    QSqlQuery query;
    bool flag = QSqlDatabase::database().transaction();
    if (!flag)
        qDebug() << "ConfigFile使用InsertData开启事务失败";
    query.prepare("insert into userMsg(UserId,UserName,Password, status) values(?,?,?,?)");
    query.bindValue(0,1);
    query.bindValue(1,name);
    query.bindValue(2,pwd);
    query.bindValue(3,status == true ? 1 : 0);
    flag = query.exec();
    //flag = query.exec(QString("insert into userMsg(UserName,Password, status) values('%1','%2',%3)").arg(name).arg(pwd).arg(status == true ? 1 : 0));
    if (!flag){
        qDebug() << "ConfigFile使用InsertData插入数据失败" << query.lastError();
        QSqlDatabase::database().rollback();
    }
    else QSqlDatabase::database().commit();
}

//查询用户相关信息
void ConfigFile::QueryTable()
{
    QSqlQuery query;
    query.exec(QString("select * from userMsg"));
    while(query.next()) {
        m_userName = query.value(2).toString();
        m_pwd = query.value(3).toString();
        m_status = query.value(4).toBool();
        //解密数据
        //m_userName = decipher(m_userName);
        m_pwd = decipher(m_pwd);
        qDebug() << "查询到了：" << m_userName << m_pwd << m_status;
    }
}

//修改用户数据库数据
void ConfigFile::ModifyValue(const QString Name, const QString Pwd, const bool status)
{
    QSqlQuery query;
    QSqlDatabase::database().transaction();
    bool flag = query.exec(QString("update userMsg set UserName='%1', Password='%2', status=%3 where UserName='%4'").arg(Name).arg(Pwd).arg(status).arg(m_userName));
    if (!flag){
        qDebug() << "ConfigFile使用ModifyValue修改数据失败" << query.lastError();
        QSqlDatabase::database().rollback();
    }
    else QSqlDatabase::database().commit();
}

//加密
QString ConfigFile::encrypt(QString target)
{
    QByteArray text = target.toLocal8Bit();
    QByteArray by = text.toBase64();
    QString str = QString(by);
    str = toXOREncryptUncrypt(str,'m');
    str.replace(QString(R"(\)"),QString(R"(\\)"));
    return str;
}

//解密
QString ConfigFile::decipher(QString target)
{
    target.replace(QString(R"(\\)"),QString(R"(\)"));
    QString str = toXOREncryptUncrypt(target,'m');
    QByteArray text = str.toLocal8Bit();
    QByteArray by = text.fromBase64(text);
    str = QString::fromLocal8Bit(by);
    return str;
}
