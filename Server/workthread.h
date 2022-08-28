#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QObject>
#include <QRunnable>
#include "tcpsocket.h"

class WorkThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit WorkThread(qintptr handle,QObject *parent = nullptr);
    ~WorkThread();

protected:
     void run() override;

signals:

private slots:
     void over();       //有断开信号关闭连接

private:
     TcpSocket* m_tcpsocket;
     bool flag = true;
};

#endif // WORKTHREAD_H
