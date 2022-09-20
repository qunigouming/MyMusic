#include "registerdia.h"
#include "ui_registerdia.h"

#include <QMouseEvent>

RegisterDia::RegisterDia(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDia)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    setModal(true);         //设置窗口为模态显示
    ui->userNameLine->setFocus();                   //设置焦点
    ui->titleWidget->installEventFilter(this);     //给标题栏安装事件过滤器
}

RegisterDia::~RegisterDia()
{
    delete ui;
}

void RegisterDia::on_exitWinodwBtn_clicked()
{
    close();
}

bool RegisterDia::eventFilter(QObject *watched, QEvent *event)
{
    //捕获标题窗口
    if (watched == ui->titleWidget) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = (QMouseEvent*)event;
            if (e->buttons() == Qt::LeftButton)
                dragPosition = e->globalPos();      //记录鼠标全局位置
        }
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *e = (QMouseEvent*)event;
            if (e->buttons() == Qt::LeftButton) {
                QPoint tempPos = e->globalPos() - dragPosition;
                move(this->pos() + tempPos);
                dragPosition = e->globalPos();
            }
        }
    }
    return QDialog::eventFilter(watched,event);
}

void RegisterDia::on_QuitBtn_clicked()
{
    close();
}
