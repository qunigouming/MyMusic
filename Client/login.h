#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include "tcpsocket.h"
#include "mainwindow.h"

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();
    static Login &getInstance();
    TcpSocket* SendSocket();            //给其他窗口转发套接字

private slots:
    void on_loginBtn_clicked();

    void on_registerBtn_clicked();

    void RecvRegist(REGISTTYPE);
    void RecvLogin(LOGINTYPE);

    void ConfigRead();      //读取到配置文件

private:
    QString strToMd5(QString str);              //将密码转换为加密后的字符串
    void Login_Successfully();
    Ui::Login *ui;
    QString m_name;             //用户名
    QString m_password;         //密码
    TcpSocket *m_tcp;
    ConfigFile *m_config;
};

#endif // LOGIN_H
