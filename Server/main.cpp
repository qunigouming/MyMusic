/*
 * 此程序结构必是一对多的结构关系
*/
#include "dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;

    return a.exec();
}
