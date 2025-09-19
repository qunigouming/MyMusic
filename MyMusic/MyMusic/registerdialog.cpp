#include "registerdialog.h"
#include "ui_registerdialog.h"

#include <QAction>
#include <QMouseEvent>
#include <QDebug>
#include <QRegularExpression>
#include <QJsonDocument>
#include "global.h"
#include "httpmanager.h"
#include "Common/Encrypt/Encrypt.h"

#define VISIBLE 0xe004
#define INVISIBLE 0xe003

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    ui->minimizeBtn->setText(QChar(0xe650));
    ui->closeBtn->setText(QChar(0xe67d));
    ui->TitleWidget->installEventFilter(this);
    ui->userLineE->installEventFilter(this);
    ui->pwdLineE->installEventFilter(this);
    ui->emailLineE->installEventFilter(this);
    ui->pwdVisibleBtn->installEventFilter(this);
    _timer = new QTimer(this);
    connect(_timer, &QTimer::timeout, [this]{
        static int current = 5;
        if (current == 0) {
            _timer->stop();
            current = 5;
            on_closeBtn_clicked();
        }
        --current;
        ui->returnTip->setText(QString("注册成功，%1秒后返回登录").arg(current));
    });

    //锁定为英文输入法
    ui->pwdLineE->setAttribute(Qt::WA_InputMethodEnabled, false);

    initHttpReqHandler();
    connect(HttpManager::GetInstance().get(), &HttpManager::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

    connect(ui->pwdVisibleBtn, &QPushButton::clicked, [this](bool checked){
        if (!ui->pwdVisibleBtn->isCheckable()) return;
        if (checked) {
            ui->pwdVisibleBtn->setText(QChar(VISIBLE));
            ui->pwdLineE->setEchoMode(QLineEdit::Normal);
        } else {
            ui->pwdVisibleBtn->setText(QChar(INVISIBLE));
            ui->pwdLineE->setEchoMode(QLineEdit::Password);
        }
    });

    //隐藏提示
    ui->userTip->hide();
    ui->pwdTip->hide();
    ui->emailTip->hide();
    ui->verifyCodeTip->hide();
}

RegisterDialog::~RegisterDialog()
{
    qDebug() << "destructing Register Dialog";
    delete ui;
}

bool RegisterDialog::eventFilter(QObject *obj, QEvent *event)
{
    static QPoint dragPosition;
    //对标题栏进行事件判断
    if (obj == ui->TitleWidget){
        //鼠标按下时记录位置
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->buttons() == Qt::LeftButton) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                dragPosition = e->globalPos();
#else
                dragPosition = e->globalPosition().toPoint();
#endif
                return true;
            }
        }
        //鼠标移动并且左键按下，移动窗口
        if (event->type() == QEvent::MouseMove){
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->buttons() == Qt::LeftButton){
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QPoint tempPos = e->globalPos() - dragPosition;     //用一个临时位置记录鼠标移动变化
#else
                QPoint tempPos = e->globalPosition().toPoint() - dragPosition;
#endif
                move(this->pos() + tempPos);        //鼠标移动多少，窗口走动多少
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                dragPosition = e->globalPos();
#else
                dragPosition = e->globalPosition().toPoint();
#endif
                return true;
            }
        }
    }

    //用户名输入框事件
    if (obj == ui->userLineE) {
        //输入框焦点变化
        if (event->type() == QEvent::FocusAboutToChange) {
            CheckUserValid();
            return true;
        }
    }

    //密码输入框事件
    if (obj == ui->pwdLineE) {
        if (event->type() == QEvent::FocusAboutToChange) {
            CheckPasswdValid();
            return true;
        }
    }

    //邮箱输入框事件
    if (obj == ui->emailLineE) {
        if (event->type() == QEvent::FocusAboutToChange) {
            CheckEmailValid();
            return true;
        }
    }

    return QWidget::eventFilter(obj,event);
}

void RegisterDialog::on_closeBtn_clicked()
{
    deleteLater();
}


void RegisterDialog::on_minimizeBtn_clicked()
{
    showMinimized();
}


void RegisterDialog::on_pwdLineE_textChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {
        ui->pwdVisibleBtn->setText("");
        ui->pwdVisibleBtn->setCheckable(false);
        return;
    }
    ui->pwdVisibleBtn->setText(QChar(INVISIBLE));
    ui->pwdVisibleBtn->setCheckable(true);
}

void RegisterDialog::on_sendVerify_clicked()
{
    //判断输入格式
    QString emailAddr = ui->emailLineE->text();
    QRegularExpression exp(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    QRegularExpressionMatch match = exp.match(emailAddr);
    if (!match.hasMatch())  {
        ui->emailTip->setText("邮箱输入有误");
        ui->emailTip->show();
        return;
    }
    //发送验证码请求
    QJsonObject json;
    json["email"] = emailAddr;
    HttpManager::GetInstance()->PostRequest(QUrl(gate_url_prefix + "/get_verifycode"), json, ReqID::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);
}

void RegisterDialog::slot_reg_mod_finish(ReqID id, QString res, ErrorCode err)
{
    if (err != ErrorCode::SUCCESS) {
        qDebug() << "网络请求错误";
        return;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull()) {
        qDebug() << "json 解析失败";
        return;
    }

    _handlers[id](jsonDoc.object());
}

void RegisterDialog::initHttpReqHandler()
{
    _handlers.insert(ReqID::ID_GET_VARIFY_CODE, [this](const QJsonObject& json){
        if (json["error"].toInt() != ErrorCode::SUCCESS) {
            qDebug() << "获取验证码失败" << json["error"].toInt();
            return;
        }
        qDebug() << "email is " << json["email"].toString();
    });

    _handlers.insert(ReqID::ID_REGISTER_USER, [this](const QJsonObject& json){
        if (json["error"].toInt() != ErrorCode::SUCCESS) {
            qDebug() << "注册用户失败" << static_cast<ErrorCode>(json["error"].toInt());
            switch (json["error"].toInt()) {
                case ErrorCode::VerifyCodeErr: {
                    ui->verifyCodeTip->show();
                    ui->verifyCodeTip->setText("验证码错误");
                    ui->emailLineE->setProperty("state", "error");
                    repolish(ui->emailLineE);
                    ui->sendVerify->setProperty("state", "error");
                    repolish(ui->sendVerify);
                    break;
                }
                case ErrorCode::UserExist: {
                    ui->userTip->show();
                    ui->userTip->setText("用户名已经存在");
                    ui->userLineE->setProperty("state", "error");
                    repolish(ui->userLineE);
                    break;
                }
                case ErrorCode::EmailExist: {
                    ui->emailTip->show();
                    ui->emailTip->setText("邮箱已经存在");
                    ui->emailLineE->setProperty("state", "error");
                    repolish(ui->emailLineE);
                    break;
                }
            }
            return;
        }
        //注册成功，跳转窗口
        RegistFinish();
        qDebug() << "注册用户成功";
    });
}

bool RegisterDialog::CheckUserValid()
{
    if (ui->userLineE->text().isEmpty()) {
        ui->userTip->show();
        ui->userTip->setText("用户名不能为空");
        ui->userLineE->setProperty("state", "error");
        repolish(ui->userLineE);
        return false;
    } else {
        ui->userTip->hide();
        ui->userLineE->setProperty("state", "normal");
        repolish(ui->userLineE);
        return true;
    }
}

bool RegisterDialog::CheckPasswdValid()
{
    if (ui->pwdLineE->text().isEmpty()) {
        ui->pwdTip->setText("密码不能为空");
        ui->pwdTip->show();
        ui->pwdLineE->setProperty("state", "error");
        ui->pwdVisibleBtn->setProperty("state", "error");
        repolish(ui->pwdLineE);
        repolish(ui->pwdVisibleBtn);
        return false;
    } else if (ui->pwdLineE->text().contains(" ")) {
        ui->pwdTip->setText("不能包括空行");
        ui->pwdTip->show();
        ui->pwdLineE->setProperty("state", "error");
        ui->pwdVisibleBtn->setProperty("state", "error");
        repolish(ui->pwdLineE);
        repolish(ui->pwdVisibleBtn);
        return false;
    } else if (ui->pwdLineE->text().length() < 8 || ui->pwdLineE->text().length() > 16) {
        ui->pwdTip->setText("长度为8-16个字符");
        ui->pwdTip->show();
        ui->pwdLineE->setProperty("state", "error");
        ui->pwdVisibleBtn->setProperty("state", "error");
        repolish(ui->pwdLineE);
        repolish(ui->pwdVisibleBtn);
        return false;
    } else if (!ui->pwdLineE->text().contains(QRegularExpression("^(?:"                                     // 非捕获分组开始
                                                                 "(?=.*[A-Za-z])(?=.*\\d)"                 // 字母+数字组合
                                                                 "|(?=.*[A-Za-z])(?=.*[!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?])" // 字母+符号组合
                                                                 "|(?=.*\\d)(?=.*[!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?])"      // 数字+符号组合
                                                                 ")"                                       // 非捕获分组结束
                                                                 "[A-Za-z\\d!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?]+$"))) {
        ui->pwdTip->setText("必须包含字母、数字、符号中至少2种");
        ui->pwdTip->show();
        ui->pwdLineE->setProperty("state", "error");
        ui->pwdVisibleBtn->setProperty("state", "error");
        repolish(ui->pwdLineE);
        repolish(ui->pwdVisibleBtn);
        return false;
    } else {
        ui->pwdTip->hide();
        ui->pwdLineE->setProperty("state", "normal");
        ui->pwdVisibleBtn->setProperty("state", "normal");
        repolish(ui->pwdLineE);
        repolish(ui->pwdVisibleBtn);
        return true;
    }
}

bool RegisterDialog::CheckEmailValid()
{
    if (ui->emailLineE->text().isEmpty()) {
        ui->emailTip->setText("邮箱不能为空");
        ui->emailTip->show();
        ui->emailLineE->setProperty("state", "error");
        repolish(ui->emailLineE);
        return false;
    } else {
        ui->emailTip->hide();
        ui->emailLineE->setProperty("state", "normal");
        repolish(ui->emailLineE);
        return true;
    }
}

bool RegisterDialog::CheckVerifyCodeValid()
{
    if (ui->verifCodeLineE->text().isEmpty()) {
        ui->verifyCodeTip->show();
        ui->emailLineE->setProperty("state", "error");
        repolish(ui->emailLineE);
        ui->sendVerify->setProperty("state", "error");
        repolish(ui->sendVerify);
        return false;
    } else {
        ui->verifyCodeTip->hide();
        ui->emailLineE->setProperty("state", "normal");
        repolish(ui->emailLineE);
        ui->sendVerify->setProperty("state", "normal");
        repolish(ui->sendVerify);
        return true;
    }
}

void RegisterDialog::RegistFinish()
{
    ui->stackedWidget->setCurrentIndex(1);
    _timer->start(1000);
}


void RegisterDialog::on_registerBtn_clicked()
{
    if (!(CheckUserValid() && CheckPasswdValid() && CheckEmailValid() && CheckVerifyCodeValid())) {
        //校检错误处理
        return;
    }
    //正常处理
    QJsonObject json;
    json["name"] = ui->userLineE->text();
    
    // 密码加密
    Encrypt encrypt(ui->pwdLineE->text().toStdString());
    json["passwd_hash"] = QString::fromStdString(encrypt.getPasswordHash());
    json["passwd_salt"] = QString::fromStdString(encrypt.getPasswordSalt());

    json["email"] = ui->emailLineE->text();
    json["icon"] = ":/source/image/default_user_head.png";      // 使用默认头像
    json["verifycode"] = ui->verifCodeLineE->text();
    HttpManager::GetInstance()->PostRequest(QUrl(gate_url_prefix + "/user_register"), json, ReqID::ID_REGISTER_USER, Modules::REGISTERMOD);
}


void RegisterDialog::on_backBtn_clicked()
{
    on_closeBtn_clicked();
}

