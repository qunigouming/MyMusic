#include "windowmanager.h"

WindowManager::WindowManager(QWidget *parent)
    : QMainWindow{parent}
{
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);          //no frame handle
    _loginDialog = new LoginDialog(this);
    setCentralWidget(_loginDialog);

    //connect(_loginDialog, &LoginDialog::sig_switchMainWindow, );
}
