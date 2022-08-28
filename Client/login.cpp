#include "login.h"
#include "ui_login.h"
#include <QDebug>
#include <QMessageBox>
#include <QRegExpValidator>

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    setObjectName("LoGinWindow");
    ui->registerBtn->setStyleSheet("QPushButton:hover{color: #0ff;}");
    ui->userLinE->setFocus();
    ui->pwdLinE->setValidator(new QRegExpValidator(QRegExp(("^[\u4E00-\u9FA5A-Za-z0-9_]+$")))); //输入限制，只能输入数字字符下划线
    m_tcp = new TcpSocket(this);
    connect(m_tcp,&TcpSocket::RegistJudge,this,&Login::RecvRegist);     //注册信号
    connect(m_tcp,&TcpSocket::LoginJudge,this,&Login::RecvLogin);       //登录信号
}

Login::~Login()
{
    delete ui;
}

Login &Login::getInstance()
{
    static Login instance;
    return instance;
}

TcpSocket *Login::SendSocket()
{
    return m_tcp;
}

//登录按钮
void Login::on_loginBtn_clicked()
{
    if (ui->userLinE->text().isEmpty()){
        ui->userHint->setText(QString("用户名不能为空"));
    }
    else if(ui->pwdLinE->text().isEmpty()){
        ui->pwdHint->setText(QString("密码不能为空"));
    }
    else{
        m_name = ui->userLinE->text();
        m_password = strToMd5(ui->pwdLinE->text());
        m_tcp->getUserPwd(m_name,m_password);
        m_tcp->sendmsg(TcpMSGType::LOGIN_REQUEST);
    }
}

//注册按钮
void Login::on_registerBtn_clicked()
{
    if (ui->userLinE->text().isEmpty()){
        ui->userHint->setText(QString("用户名不能为空"));
    }
    else if(ui->pwdLinE->text().isEmpty()){
        ui->pwdHint->setText(QString("密码不能为空"));
    }
    else if (ui->pwdLinE->text().size() < PASSWORDMINSIZE){
        ui->pwdLinE->clear();
        ui->pwdHint->setText(QString("密码长度不能小于%1").arg(PASSWORDMINSIZE));
    }
    else{
        m_name = ui->userLinE->text();
        m_password = strToMd5(ui->pwdLinE->text());
        m_tcp->getUserPwd(m_name,m_password);
        m_tcp->sendmsg(TcpMSGType::REGIST_REQUEST);
    }
}

//对注册类型进行判断
void Login::RecvRegist(REGISTTYPE type)
{
    switch(type){
        case REGISTTYPE::HAVEUSER:{
            ui->userLinE->clear();
            ui->pwdLinE->clear();
            ui->userHint->setText("已有该用户");
            break;
        }
        case REGISTTYPE::EQUAL_USERNAME:{
            ui->userLinE->clear();
            ui->pwdLinE->clear();
            ui->userHint->setText("重复的用户名");
            break;
        }
        case REGISTTYPE::OK:{
            QMessageBox::information(this,"注册","注册成功");
            this->hide();       //隐藏登录窗口
            MainWindow::getInstance().ShowWindow(m_name);
            break;
        }
        default:
            qDebug() << "注册消息：没有此类型";
        break;
    }
}

void Login::RecvLogin(LOGINTYPE type)
{
    switch(type){
        case LOGINTYPE::NOTUSER:{
            ui->userHint->setText("没有该用户");
            break;
        }
        case LOGINTYPE::LOGIN:{
            ui->userHint->setText("用户已在线上");
            break;
        }
        case LOGINTYPE::OK:{
            this->hide();
            MainWindow::getInstance().ShowWindow(m_name);
            break;
        }
        default:
            qDebug() << "登录消息：没有此类型";
        break;
    }
}

QString Login::strToMd5(QString str)
{
    QString strMd5;
    QByteArray array = QCryptographicHash::hash(str.toLatin1(),QCryptographicHash::Md5);
    strMd5.append(array.toHex());
    return strMd5;
}
