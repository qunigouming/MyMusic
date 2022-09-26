#ifndef TCPWORKTHREAD_H
#define TCPWORKTHREAD_H

#include <QThread>

class TcpWorkThread : public QThread
{
    Q_OBJECT
public:
    explicit TcpWorkThread(QObject *parent = nullptr);

protected:
    void run() override;            //重写run()函数

signals:

public slots:
};

#endif // TCPWORKTHREAD_H
