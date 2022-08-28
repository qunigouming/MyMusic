#include "dialog.h"
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <QSqlError>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    setFixedSize(300,200);
    initDataBase();
    QPushButton *Btn = new QPushButton("上传",this);
    connect(Btn,&QPushButton::clicked,this,[=]{
        QStringList list = QFileDialog::getOpenFileNames(this);
        bool flag = true;
        QSqlDatabase::database().transaction();     //开启事务
        for (QString pathname : list){
            QString filename = QFileInfo(pathname).baseName();
            qDebug() << filename;
            QSqlQuery query;
            flag &= query.exec(QString("insert into music_list (music_name,music_path) value('%1','%2')").arg(filename).arg(pathname));
            if (!flag){
                qDebug() << "error: " << query.lastError().text();
            }
        }
        if (flag)  QSqlDatabase::database().commit();
        else {
            qDebug() << "错误";
            QSqlDatabase::database().rollback();
        }
    });
}

Dialog::~Dialog()
{
    m_db.close();
}

void Dialog::initDataBase()
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
