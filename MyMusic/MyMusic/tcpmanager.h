#ifndef TCPMANAGER_H
#define TCPMANAGER_H

#include <QObject>
#include "Singleton.h"
#include "global.h"
#include <QTcpSocket>
#include <QMap>

class TcpManager : public QObject, public Singleton<TcpManager>, public std::enable_shared_from_this<TcpManager>
{
    Q_OBJECT
    friend class Singleton<TcpManager>;
public:
    ~TcpManager() = default;
    void sendData(ReqID id, QString data);

public slots:
    void slot_tcp_connect(ServerInfo serverinfo);
private:
    explicit TcpManager(QObject *parent = nullptr);
    void initHandler();
    void handleMsg(ReqID id, int len, QByteArray data);

    QTcpSocket* _socket;
    QString _host;
    uint16_t _port;
    QByteArray* _buffer;
    bool _b_recv_pending = false;
    quint16 _message_id = 0;
    quint16 _message_len = 0;
    QMap<ReqID, std::function<void(ReqID id, int len, QByteArray data)>> _handlers;
signals:
    void sig_con_status(bool status);
    void sig_login_status(ErrorCode error);
    void sig_login_failed(ErrorCode error);
    void sig_send_data(ReqID id, QByteArray data);
};

#endif // TCPMANAGER_H
