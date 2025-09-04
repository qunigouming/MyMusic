#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"
#include <QMap>
#include <functional>
#include <QTimer>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_closeBtn_clicked();         //关闭窗口

    void on_minimizeBtn_clicked();      //最小化窗口

    void on_pwdLineE_textChanged(const QString &arg1);          //文本长度变化样式设置

    void on_sendVerify_clicked();       //发送验证码

    void slot_reg_mod_finish(ReqID id, QString res, ErrorCode err);

    void on_registerBtn_clicked();

    void on_backBtn_clicked();

private:
    void initHttpReqHandler();
    bool CheckUserValid();
    bool CheckPasswdValid();
    bool CheckEmailValid();
    bool CheckVerifyCodeValid();
    void RegistFinish();
    QTimer* _timer;

    Ui::RegisterDialog *ui;
    QMap<ReqID, std::function<void(const QJsonObject&)>> _handlers;
};

#endif // REGISTERDIALOG_H
