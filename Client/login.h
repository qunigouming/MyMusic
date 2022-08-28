#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include "tcpsocket.h"
#include "mainwindow.h"
#include <QCryptographicHash>           //包含MD5算法库

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

private:
    QString strToMd5(QString str);              //将密码转换为加密后的字符串
    Ui::Login *ui;
    QString m_name;             //用户名
    QString m_password;         //密码
    TcpSocket *m_tcp;
};

#endif // LOGIN_H
