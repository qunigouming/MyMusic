#include "login.h"
#include <QApplication>
#include "registerdia.h"
#include "tcpworkthread.h"
#include <QDebug>

TcpWorkThread* tcpworkthread;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //注册类型，否则不能跨线程使用信号槽的队列连接
    qRegisterMetaType<TcpMSGType>("TcpMSGType");
    qRegisterMetaType<LOGINTYPE>("LOGINTYPE");
    qRegisterMetaType<REGISTTYPE>("REGISTTYPE");
    Login::getInstance().show();
//    RegisterDia d;
//    d.show();
    return a.exec();
}
