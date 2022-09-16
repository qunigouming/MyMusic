#include "login.h"
#include <QApplication>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Login::getInstance().show();
    //MainWindow::getInstance().show();
    return a.exec();
}
