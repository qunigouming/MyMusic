#include "tcpmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void TcpManager::sendData(ReqID id, QString data)
{
    quint16 len = static_cast<quint16>(data.size());
    QByteArray block;
    QDataStream stream(&block, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::BigEndian);
    stream << id << len;
    block.append(data.toLatin1());
    _socket->write(block);
}

void TcpManager::slot_tcp_connect(ServerInfo serverinfo)
{
    _host = serverinfo.Host;
    _port = static_cast<uint16_t>(serverinfo.Port.toUInt());
    _socket->connectToHost(_host, _port);
}

TcpManager::TcpManager(QObject *parent)
    : QObject{parent}
{
    _socket = new QTcpSocket(this);
    connect(_socket, &QTcpSocket::connected, [&]{
        qDebug() << "connected to server!!!";
        emit sig_con_status(true);
    });

    //handle tcp connect error
    connect(_socket, &QTcpSocket::errorOccurred, [&](QTcpSocket::SocketError error){
        qDebug() << "Error is:" << _socket->errorString();
        switch (error) {
            case QTcpSocket::ConnectionRefusedError: {
                qDebug() << "Connection Refused!";
                emit sig_con_status(false);
                break;
            }
            case QTcpSocket::RemoteHostClosedError: {
                qDebug() << "Remot host closed connection!";
                emit sig_con_status(false);
                break;
            }
            case QTcpSocket::HostNotFoundError: {
                qDebug() << "Host not found!";
                emit sig_con_status(false);
                break;
            }
            case QTcpSocket::SocketTimeoutError: {
                qDebug() << "Connection timeout!";
                emit sig_con_status(false);
                break;
            }
            case QTcpSocket::NetworkError: {
                qDebug() << "Network Error!";
                emit sig_con_status(false);
                break;
            }
            default: {
                qDebug() << "Other Error!";
                emit sig_con_status(false);
                break;
            }
        }
    });

    connect(_socket, &QTcpSocket::disconnected, [&]{
        qDebug() << "Disconnected from server.";
    });

    connect(_socket, &QTcpSocket::readyRead, [&]{
        _buffer->append(_socket->readAll());
        QDataStream stream(_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_0);

        while (true) {
            //first time to receive data
            if (!_b_recv_pending) {
                if (_buffer->size() < static_cast<int>(sizeof(quint16) * 2))    return;
                stream >> _message_id >> _message_len;
                _buffer->remove(0, sizeof(quint16)*2);          //delete id and len data
                qDebug() << "Message ID: " << _message_id << ", Length: " << _message_len;
            }

            //数据长度不足去处理
            if (_buffer->size() < _message_len) {
                _b_recv_pending = true;
                return;
            }

            //prepare to hanlder data
            _b_recv_pending = false;
            QByteArray messageBody = _buffer->mid(0, _message_len);
            _buffer->remove(0, _message_len);
            //process data
            handleMsg(static_cast<ReqID>(_message_id), _message_len, messageBody);
        }
    });

    initHandler();
}

void TcpManager::initHandler()
{
    _handlers.insert(IO_LOGIN_USER_RSP, [this](ReqID id, int len, QByteArray data) {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")) {
            int err = ErrorCode::ERR_JSON;
            emit sig_login_failed(static_cast<ErrorCode>(err));
            return;
        }
        int err = jsonObj["error"].toInt();
        if (err != ErrorCode::SUCCESS) {
            qDebug() << "Login Failed, error is " << err;
            emit sig_login_failed(static_cast<ErrorCode>(err));
            return;
        }

        auto uid = jsonObj["uid"].toInt();
        auto name = jsonObj["name"].toString();
        auto headIcon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        QList<QString> musicList;
        QJsonArray jsonArray = jsonObj["musicList"].toArray();
        for (auto value : jsonArray) {
            musicList.append(value.toString());
        }

    });
}

void TcpManager::handleMsg(ReqID id, int len, QByteArray data)
{
    auto find_iter = _handlers.find(id);
    if (find_iter == _handlers.end()) {
        qDebug() << "not found id [" << _message_id << "] to handle";
        return;
    }
    find_iter.value()(id, len, data);
}
