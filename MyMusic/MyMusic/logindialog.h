#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "registerdialog.h"

/******************************************************************************
 *
 * @file       logindialog.h
 * @brief      Login Widget implementation
 *
 * @author     qunigouming
 * @date       2025/07/02
 * @history
 *****************************************************************************/

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

signals:
    void sig_con_tcpserver(ServerInfo& info);
    void sig_switchMainWindow();

private slots:
    void on_closeBtn_clicked();

    void on_registerBtn_clicked();

    void on_userLineE_textChanged(const QString &arg1);

    void on_pwdLineE_textChanged(const QString &arg1);

    void on_loginBtn_clicked();

    void slot_login_mod_finish(ReqID id, QString res, ErrorCode err);               //登录模块完成槽

private:
    void initHttpReqHandler();
    Ui::LoginDialog *ui;
    bool userIsEmpty = true;
    bool pwdIsEmpty = true;
    int _uid;
    QString _token;

    QMap<ReqID, std::function<void(const QJsonObject&)>> _handlers;
};

#endif // LOGINDIALOG_H
