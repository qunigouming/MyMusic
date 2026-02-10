#include "forgetpwddialog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "timerbutton.h"
#include "httpmanager.h"
#include "Common/Encrypt/Encrypt.h"

ForgetPwdDialog::ForgetPwdDialog(QWidget* parent)
    : QDialog(parent)
{
    initUi();
    initHttpReqHandler();
    initConnections();

    connect(HttpManager::GetInstance().get(), &HttpManager::sig_reg_mod_finish,
            this, &ForgetPwdDialog::slot_reg_mod_finish);
}

ForgetPwdDialog::~ForgetPwdDialog()
{
    disconnect(HttpManager::GetInstance().get(), &HttpManager::sig_reg_mod_finish,
               this, &ForgetPwdDialog::slot_reg_mod_finish);
}

void ForgetPwdDialog::initUi()
{
    setWindowTitle("找回密码");
    setFixedSize(420, 360);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(10);

    auto* title = new QLabel("找回密码", this);
    title->setStyleSheet("font-size: 18px; font-weight: 600;");
    mainLayout->addWidget(title);

    _emailLineE = new QLineEdit(this);
    _emailLineE->setPlaceholderText("输入登录邮箱");
    _emailLineE->setProperty("state", "normal");
    _emailTip = new QLabel("邮箱不能为空", this);

    _verifyCodeLineE = new QLineEdit(this);
    _verifyCodeLineE->setPlaceholderText("输入邮箱验证码");
    _verifyCodeLineE->setProperty("state", "normal");
    _verifyTip = new QLabel("验证码不能为空", this);

    _sendVerifyBtn = new TimerButton(this);
    _sendVerifyBtn->setText("发送验证码");
    _sendVerifyBtn->setProperty("state", "normal");

    _pwdLineE = new QLineEdit(this);
    _pwdLineE->setPlaceholderText("输入新密码");
    _pwdLineE->setEchoMode(QLineEdit::Password);
    _pwdLineE->setProperty("state", "normal");
    _pwdTip = new QLabel("密码不能为空", this);

    _confirmPwdLineE = new QLineEdit(this);
    _confirmPwdLineE->setPlaceholderText("再次输入新密码");
    _confirmPwdLineE->setEchoMode(QLineEdit::Password);
    _confirmPwdLineE->setProperty("state", "normal");
    _confirmPwdTip = new QLabel("请再次输入密码", this);

    _resetPwdBtn = new QPushButton("重置密码", this);

    const QString inputStyle =
        "QLineEdit {"
        " border: 1px solid #d9d9d9;"
        " border-radius: 4px;"
        " padding: 8px;"
        "}"
        "QLineEdit[state='error'] {"
        " border: 1px solid #f56c6c;"
        "}";

    _emailLineE->setStyleSheet(inputStyle);
    _verifyCodeLineE->setStyleSheet(inputStyle);
    _pwdLineE->setStyleSheet(inputStyle);
    _confirmPwdLineE->setStyleSheet(inputStyle);

    _sendVerifyBtn->setStyleSheet(
        "QPushButton { background-color: #0088ff; color: white; border: none; border-radius: 4px; padding: 8px 12px; }"
        "QPushButton:disabled { background-color: #7abaf7; }");
    _resetPwdBtn->setStyleSheet(
        "QPushButton { background-color: #0088ff; color: white; border: none; border-radius: 4px; padding: 9px 12px; }"
        "QPushButton:hover { background-color: #0074db; }");

    const QString tipStyle = "QLabel { color: #f56c6c; font-size: 12px; }";
    _emailTip->setStyleSheet(tipStyle);
    _verifyTip->setStyleSheet(tipStyle);
    _pwdTip->setStyleSheet(tipStyle);
    _confirmPwdTip->setStyleSheet(tipStyle);

    _emailTip->hide();
    _verifyTip->hide();
    _pwdTip->hide();
    _confirmPwdTip->hide();

    mainLayout->addWidget(_emailLineE);
    mainLayout->addWidget(_emailTip);

    auto* verifyLayout = new QHBoxLayout;
    verifyLayout->addWidget(_verifyCodeLineE, 1);
    verifyLayout->addWidget(_sendVerifyBtn);
    mainLayout->addLayout(verifyLayout);
    mainLayout->addWidget(_verifyTip);

    mainLayout->addWidget(_pwdLineE);
    mainLayout->addWidget(_pwdTip);

    mainLayout->addWidget(_confirmPwdLineE);
    mainLayout->addWidget(_confirmPwdTip);

    mainLayout->addSpacing(6);
    mainLayout->addWidget(_resetPwdBtn);
}

void ForgetPwdDialog::initConnections()
{
    connect(_sendVerifyBtn, &QPushButton::clicked, this, &ForgetPwdDialog::onSendVerifyCode);
    connect(_resetPwdBtn, &QPushButton::clicked, this, &ForgetPwdDialog::onResetPassword);

    connect(_emailLineE, &QLineEdit::textChanged, this, [this] { checkEmailValid(); });
    connect(_verifyCodeLineE, &QLineEdit::textChanged, this, [this] { checkVerifyCodeValid(); });
    connect(_pwdLineE, &QLineEdit::textChanged, this, [this] {
        checkPasswordValid();
        checkConfirmPasswordValid();
    });
    connect(_confirmPwdLineE, &QLineEdit::textChanged, this, [this] { checkConfirmPasswordValid(); });
}

void ForgetPwdDialog::initHttpReqHandler()
{
    _handlers.insert(ReqID::ID_GET_VARIFY_CODE, [this](const QJsonObject& json) {
        if (json["error"].toInt() != ErrorCode::SUCCESS) {
            QMessageBox::warning(this, "验证码", "验证码发送失败，请稍后重试", QMessageBox::Close);
            return;
        }
        QMessageBox::information(this, "验证码", "验证码已发送到邮箱", QMessageBox::Close);
    });

    _handlers.insert(ReqID::ID_RESET_PWD, [this](const QJsonObject& json) {
        if (json["error"].toInt() != ErrorCode::SUCCESS) {
            switch (json["error"].toInt()) {
            case ErrorCode::VerifyCodeErr:
                _verifyTip->setText("验证码错误");
                _verifyTip->show();
                _verifyCodeLineE->setProperty("state", "error");
                _sendVerifyBtn->setProperty("state", "error");
                repolish(_verifyCodeLineE);
                repolish(_sendVerifyBtn);
                break;
            case ErrorCode::EmailNotMatch:
                _emailTip->setText("该邮箱与账号不匹配");
                _emailTip->show();
                _emailLineE->setProperty("state", "error");
                repolish(_emailLineE);
                break;
            case ErrorCode::PasswdInvalid:
                _pwdTip->setText("密码无效");
                _pwdTip->show();
                _pwdLineE->setProperty("state", "error");
                _confirmPwdLineE->setProperty("state", "error");
                repolish(_pwdLineE);
                repolish(_confirmPwdLineE);
                break;
            default:
                QMessageBox::warning(this, "重置密码", "重置密码失败，请稍后重试", QMessageBox::Close);
                break;
            }
            return;
        }

        QMessageBox::information(this, "重置密码", "密码重置成功，请使用新密码登录", QMessageBox::Close);
        close();
    });
}

bool ForgetPwdDialog::checkEmailValid()
{
    const QString emailAddr = _emailLineE->text();
    if (emailAddr.isEmpty()) {
        _emailTip->setText("邮箱不能为空");
        _emailTip->show();
        _emailLineE->setProperty("state", "error");
        repolish(_emailLineE);
        return false;
    }

    QRegularExpression exp(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!exp.match(emailAddr).hasMatch()) {
        _emailTip->setText("邮箱输入有误");
        _emailTip->show();
        _emailLineE->setProperty("state", "error");
        repolish(_emailLineE);
        return false;
    }

    _emailTip->hide();
    _emailLineE->setProperty("state", "normal");
    repolish(_emailLineE);
    return true;
}

bool ForgetPwdDialog::checkVerifyCodeValid()
{
    if (_verifyCodeLineE->text().isEmpty()) {
        _verifyTip->setText("验证码不能为空");
        _verifyTip->show();
        _verifyCodeLineE->setProperty("state", "error");
        _sendVerifyBtn->setProperty("state", "error");
        repolish(_verifyCodeLineE);
        repolish(_sendVerifyBtn);
        return false;
    }

    _verifyTip->hide();
    _verifyCodeLineE->setProperty("state", "normal");
    _sendVerifyBtn->setProperty("state", "normal");
    repolish(_verifyCodeLineE);
    repolish(_sendVerifyBtn);
    return true;
}

bool ForgetPwdDialog::checkPasswordValid()
{
    const QString pwd = _pwdLineE->text();
    if (pwd.isEmpty()) {
        _pwdTip->setText("密码不能为空");
        _pwdTip->show();
        _pwdLineE->setProperty("state", "error");
        repolish(_pwdLineE);
        return false;
    } else if (pwd.contains(" ")) {
        _pwdTip->setText("不能包括空格");
        _pwdTip->show();
        _pwdLineE->setProperty("state", "error");
        repolish(_pwdLineE);
        return false;
    } else if (pwd.length() < 8 || pwd.length() > 16) {
        _pwdTip->setText("长度为8-16个字符");
        _pwdTip->show();
        _pwdLineE->setProperty("state", "error");
        repolish(_pwdLineE);
        return false;
    } else if (!pwd.contains(QRegularExpression("^(?:"
                                                 "(?=.*[A-Za-z])(?=.*\\d)"
                                                 "|(?=.*[A-Za-z])(?=.*[!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?])"
                                                 "|(?=.*\\d)(?=.*[!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?])"
                                                 ")"
                                                 "[A-Za-z\\d!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?]+$"))) {
        _pwdTip->setText("必须包含字母、数字、符号中至少2种");
        _pwdTip->show();
        _pwdLineE->setProperty("state", "error");
        repolish(_pwdLineE);
        return false;
    }

    _pwdTip->hide();
    _pwdLineE->setProperty("state", "normal");
    repolish(_pwdLineE);
    return true;
}

bool ForgetPwdDialog::checkConfirmPasswordValid()
{
    if (_confirmPwdLineE->text().isEmpty()) {
        _confirmPwdTip->setText("请再次输入密码");
        _confirmPwdTip->show();
        _confirmPwdLineE->setProperty("state", "error");
        repolish(_confirmPwdLineE);
        return false;
    }

    if (_confirmPwdLineE->text() != _pwdLineE->text()) {
        _confirmPwdTip->setText("两次输入密码不一致");
        _confirmPwdTip->show();
        _confirmPwdLineE->setProperty("state", "error");
        repolish(_confirmPwdLineE);
        return false;
    }

    _confirmPwdTip->hide();
    _confirmPwdLineE->setProperty("state", "normal");
    repolish(_confirmPwdLineE);
    return true;
}

void ForgetPwdDialog::onSendVerifyCode()
{
    if (!checkEmailValid()) {
        return;
    }

    QJsonObject json;
    json["email"] = _emailLineE->text();
    HttpManager::GetInstance()->PostRequest(QUrl(gate_url_prefix + "/get_verifycode"),
                                            json, ReqID::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);
}

void ForgetPwdDialog::onResetPassword()
{
    if (!(checkEmailValid() && checkVerifyCodeValid() && checkPasswordValid() && checkConfirmPasswordValid())) {
        return;
    }

    Encrypt encrypt(_pwdLineE->text().toStdString());
    QJsonObject json;
    json["email"] = _emailLineE->text();
    json["verifycode"] = _verifyCodeLineE->text();
    json["passwd_hash"] = QString::fromStdString(encrypt.getPasswordHash());
    json["passwd_salt"] = QString::fromStdString(encrypt.getPasswordSalt());

    HttpManager::GetInstance()->PostRequest(QUrl(gate_url_prefix + "/reset_password"),
                                            json, ReqID::ID_RESET_PWD, Modules::REGISTERMOD);
}

void ForgetPwdDialog::slot_reg_mod_finish(ReqID id, QString res, ErrorCode err)
{
    if (err != ErrorCode::SUCCESS) {
        QMessageBox::warning(this, "网络错误", "请求失败，请检查网络后重试", QMessageBox::Close);
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull()) {
        QMessageBox::warning(this, "错误", "响应解析失败", QMessageBox::Close);
        return;
    }

    if (_handlers.contains(id)) {
        _handlers[id](jsonDoc.object());
    }
}
