#include "login.h"
#include <QApplication>
#include "registerdia.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Login::getInstance().show();
    RegisterDia d;
    d.show();
    return a.exec();
}
