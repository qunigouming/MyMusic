#ifndef REGISTERDIA_H
#define REGISTERDIA_H

#include <QDialog>
#include "tcpsocket.h"

namespace Ui {
class RegisterDia;
}

class RegisterDia : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDia(QWidget *parent = 0);
    ~RegisterDia();

private slots:
    void on_exitWinodwBtn_clicked();

    void on_QuitBtn_clicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event);      //事件过滤器

private:
    Ui::RegisterDia *ui;

    //标题栏
    QPoint dragPosition;
};

#endif // REGISTERDIA_H
