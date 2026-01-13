#include "windowmanager.h"
#include "registerdialog.h"

#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QDir>
#include <QSettings>
#include "global.h"
#include <QDebug>
#include "mainwindow.h"
#include "complexionwidget.h"
#include "LogManager.h"

int main(int argc, char *argv[])
{
    LogManager::InitGlog(argv[0]);
    LOG(INFO) << "argv[0]: " << argv[0];
    qSetMessagePattern("[ %{time [hh:mm:ss:zzz]} %{file}: %{line} %{threadid} ] %{message}");
    QApplication a(argc, argv);
    //加载字体文件
    QFontDatabase::addApplicationFont(":/source/font/windowfont.ttf");
    //QFontDatabase::addApplicationFont(":/source/font/iconfont.ttf");
    QFontDatabase::addApplicationFont(":/source/font/otherfont.ttf");
    QFontDatabase::addApplicationFont(":/source/font/iconfont.ttf");

    //加载配置文件
    QSettings settings(":/config.ini", QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://" + gate_host + ":" + gate_port;

    WindowManager w;
    w.show();
    //MainWindow w;
    //w.show();
    return a.exec();
}
