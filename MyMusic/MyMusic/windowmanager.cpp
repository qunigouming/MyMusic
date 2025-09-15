#include "windowmanager.h"
#include "tcpmanager.h"

WindowManager::WindowManager(QWidget *parent)
    : QMainWindow{parent}, _loginDialog(nullptr), _mainWindow(nullptr)
{
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);          //no frame handle
    _loginDialog.reset(new LoginDialog(this));
    _loginDialog->show();
    setCentralWidget(_loginDialog.get());

    connect(TcpManager::GetInstance().get(), &TcpManager::sig_switch_mainwindow, this, &WindowManager::slotSwitchMainWindow);
}

void WindowManager::slotSwitchMainWindow()
{
    _mainWindow.reset(new MainWindow(this));
    _mainWindow->show();
    setCentralWidget(_mainWindow.get());
    _loginDialog.reset();
}
