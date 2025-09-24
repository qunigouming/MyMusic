#include "windowmanager.h"
#include "tcpmanager.h"
#include <QMessageBox>

WindowManager::WindowManager(QWidget *parent)
    : QMainWindow{parent}, _loginDialog(nullptr), _mainWindow(nullptr)
{
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);          //no frame handle
    setWindowIcon(QIcon(":/source/image/AppPic.png"));
    _sysTray = QSharedPointer<SysTray>(new SysTray(this));
    _loginDialog.reset(new LoginDialog(this));
    _loginDialog->show();
    setCentralWidget(_loginDialog.get());

    connect(TcpManager::GetInstance().get(), &TcpManager::sig_switch_mainwindow, this, &WindowManager::slotSwitchMainWindow);
    connect(TcpManager::GetInstance().get(), &TcpManager::sig_notify_off_line, this, &WindowManager::slotOffLine);
}

void WindowManager::slotSwitchMainWindow()
{
    _mainWindow.reset(new MainWindow(this));
    _mainWindow->show();
    setCentralWidget(_mainWindow.get());
    _loginDialog.reset();
}

void WindowManager::slotOffLine()
{
    QMessageBox::information(this, tr("下线提示"), tr("已有账号登录！！！"));
    TcpManager::GetInstance()->closeConnection();
    offLineLogin();
}

void WindowManager::offLineLogin()
{
    _mainWindow.reset();
    _loginDialog.reset(new LoginDialog(this));
    _loginDialog->show();
    setCentralWidget(_loginDialog.get());
}
