/*
 *
*/

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "tcpserver.h"
#include "tcpsocket.h"


class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    TcpServer *m_server;
};

#endif // DIALOG_H
