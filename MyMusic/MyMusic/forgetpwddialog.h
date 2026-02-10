#ifndef FORGETPWDDIALOG_H
#define FORGETPWDDIALOG_H

#include <QDialog>
#include <QMap>
#include <functional>

#include "global.h"

class QLabel;
class QLineEdit;
class QPushButton;
class TimerButton;
class QWidget;

class ForgetPwdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForgetPwdDialog(QWidget* parent = nullptr);
    ~ForgetPwdDialog();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSendVerifyCode();
    void onResetPassword();
    void slot_reg_mod_finish(ReqID id, QString res, ErrorCode err);

private:
    void initUi();
    void initConnections();
    void initHttpReqHandler();

    bool checkEmailValid();
    bool checkVerifyCodeValid();
    bool checkPasswordValid();
    bool checkConfirmPasswordValid();

private:
    QLabel* _emailTip = nullptr;
    QLabel* _verifyTip = nullptr;
    QLabel* _pwdTip = nullptr;
    QLabel* _confirmPwdTip = nullptr;

    QLineEdit* _emailLineE = nullptr;
    QLineEdit* _verifyCodeLineE = nullptr;
    QLineEdit* _pwdLineE = nullptr;
    QLineEdit* _confirmPwdLineE = nullptr;

    TimerButton* _sendVerifyBtn = nullptr;
    QPushButton* _resetPwdBtn = nullptr;
    QWidget* _titleWidget = nullptr;
    QPushButton* _minimizeBtn = nullptr;
    QPushButton* _closeBtn = nullptr;

    QMap<ReqID, std::function<void(const QJsonObject&)>> _handlers;
};

#endif // FORGETPWDDIALOG_H
