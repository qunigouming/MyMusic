#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMouseEvent>
#include <QDebug>
#include <QJsonDocument>
#include "httpmanager.h"
#include <QMessageBox>
#include "tcpmanager.h"
#include "Common/Encrypt/Encrypt.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    ui->setBtn->setText(QChar(0xe61c));
    ui->closeBtn->setText(QChar(0xe67d));
    ui->TitleWidget->installEventFilter(this);
    ui->loginBtn->installEventFilter(this);
    ui->loginBtn->setVisible(true);
    connect(HttpManager::GetInstance().get(), &HttpManager::sig_login_mod_finish, this, &LoginDialog::slot_login_mod_finish);
    connect(this, &LoginDialog::sig_con_tcpserver, TcpManager::GetInstance().get(), &TcpManager::slot_tcp_connect);
    connect(TcpManager::GetInstance().get(), &TcpManager::sig_con_status, this, [&](bool status){
        if (status) {
            //connect server succeeded
            QJsonObject jsonObj;
            jsonObj["uid"] = _uid;
            jsonObj["token"] = _token;

            QJsonDocument jsonDoc(jsonObj);
            QByteArray jsonStr = jsonDoc.toJson(QJsonDocument::Indented);
            emit TcpManager::GetInstance()->sig_send_data(ReqID::ID_LOGIN_USER_REQ, jsonStr);
        } else {
            QMessageBox::warning(this, "连接服务器", "服务器连接失败");
        }
    });
    connect(TcpManager::GetInstance().get(), &TcpManager::sig_login_status, [&](ErrorCode error){
        if (error != ErrorCode::SUCCESS) {
            QMessageBox::warning(this, "用户登录", QString("登录失败，错误码: %1").arg(error));
            return;
        }
        //inform transform-window that change to mainwindow
        emit sig_switchMainWindow();
    });

    initHttpReqHandler();
}

LoginDialog::~LoginDialog()
{
    qDebug() << "LoginDialog destory";
    disconnect(HttpManager::GetInstance().get(), &HttpManager::sig_login_mod_finish, this, &LoginDialog::slot_login_mod_finish);
    disconnect(TcpManager::GetInstance().get(), &TcpManager::sig_con_status, this, nullptr);
    delete ui;
}

bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
{
    static QPoint dragPosition;
    //对标题栏进行事件判断
    if (obj == ui->TitleWidget){
        //鼠标按下时记录位置
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->buttons() == Qt::LeftButton) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                dragPosition = e->globalPos() - parentWidget()->geometry().topLeft();
#else
                dragPosition = e->globalPosition().toPoint() - parentWidget()->geometry().topLeft();
#endif
                return true;
            }
        }
        //鼠标移动并且左键按下，移动窗口
        if (event->type() == QEvent::MouseMove){
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->buttons() == Qt::LeftButton){
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                parentWidget()->move(e->globalPos() - dragPosition);
#else
                parentWidget()->move(e->globalPosition().toPoint() - dragPosition);
#endif
                return true;
            }
        }
    }

    //对登录按钮进行事件判断
    if (obj == ui->loginBtn) {
        if (event->type() == QEvent::Enter) {
            if (!ui->loginBtn->isEnabled()) QApplication::setOverrideCursor(Qt::ForbiddenCursor);
            return true;
        } else if (event->type() == QEvent::Leave) {
            QApplication::setOverrideCursor(Qt::ArrowCursor);
            return true;
        }
        return false;
    }

    return QWidget::eventFilter(obj,event);
}

void LoginDialog::on_closeBtn_clicked()
{
    QCoreApplication::quit();
}


void LoginDialog::on_registerBtn_clicked()
{
    RegisterDialog* registerDlg = new RegisterDialog();
    registerDlg->show();
}


void LoginDialog::on_userLineE_textChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {
        userIsEmpty = true;
        ui->loginBtn->setEnabled(false);
        return;
    }
    userIsEmpty = false;
    if (!pwdIsEmpty) ui->loginBtn->setEnabled(true);
}


void LoginDialog::on_pwdLineE_textChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {
        pwdIsEmpty = true;
        ui->loginBtn->setEnabled(false);
        return;
    }
    pwdIsEmpty = false;
    if (!userIsEmpty) ui->loginBtn->setEnabled(true);
}


void LoginDialog::on_loginBtn_clicked()
{
    QJsonObject json;
    json["name"] = ui->userLineE->text();
    HttpManager::GetInstance()->PostRequest(gate_url_prefix + "/get_password_salt", json, ReqID::ID_GET_PWD_SALT, Modules::LOGINMOD);
}

void LoginDialog::slot_login_mod_finish(ReqID id, QString res, ErrorCode err)
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
    qDebug() << jsonDoc.object();
    _handlers[id](jsonDoc.object());
}

void LoginDialog::initHttpReqHandler()
{
    _handlers.insert(ReqID::ID_GET_SERVER, [this](const QJsonObject& json){
        if (json["error"].toInt() != ErrorCode::SUCCESS) {
            QMessageBox::warning(this, "错误", "用户名或密码错误", QMessageBox::Close);
            return;
        }

        ServerInfo serverInfo;
        serverInfo.Id = json["id"].toInt();
        serverInfo.Host = json["host"].toString();
        serverInfo.Port = json["port"].toString();
        serverInfo.Token = json["token"].toString();

        _uid = serverInfo.Id;
        _token = serverInfo.Token;
        emit sig_con_tcpserver(serverInfo);
    });

    _handlers.insert(ReqID::ID_GET_PWD_SALT, [this](const QJsonObject& json) {
        if (json["error"].toInt() != ErrorCode::SUCCESS) {
            QMessageBox::warning(this, "错误", "用户名或密码错误", QMessageBox::Close);
            return;
        }

        std::string salt = json["passwd_salt"].toString().toStdString();

        QJsonObject retjson;
        retjson["name"] = ui->userLineE->text();
        // 密码加密
        std::string hash = Encrypt::ComputeHashWithSalt(ui->pwdLineE->text().toStdString(), salt);
        retjson["passwd_hash"] = QString::fromStdString(hash);
        HttpManager::GetInstance()->PostRequest(gate_url_prefix + "/get_server", retjson, ReqID::ID_GET_SERVER, Modules::LOGINMOD);
    });
}

