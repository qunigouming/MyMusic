#include "tcpmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "UserManager.h"

void TcpManager::slot_tcp_connect(ServerInfo serverinfo)
{
    _host = serverinfo.Host;
    _port = static_cast<uint16_t>(serverinfo.Port.toUInt());
    _socket->connectToHost(_host, _port);
}

void TcpManager::slot_send_data(ReqID reqId, QByteArray data)
{
    uint16_t id = reqId;
    quint16 len = static_cast<quint16>(data.size());
    QByteArray block;
    QDataStream stream(&block, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::BigEndian);
    stream << id << len;
    block.append(data);
    _socket->write(block);
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
    connect(this, &TcpManager::sig_send_data, this, &TcpManager::slot_send_data);
    connect(_socket, &QTcpSocket::readyRead, [&]{
        _buffer.append(_socket->readAll());
        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_0);

        while (true) {
            //first time to receive data
            if (!_b_recv_pending) {
                if (_buffer.size() < static_cast<int>(sizeof(quint16) * 2))    return;
                stream >> _message_id >> _message_len;
                _buffer.remove(0, sizeof(quint16)*2);          //delete id and len data
                qDebug() << "Message ID: " << _message_id << ", Length: " << _message_len;
            }

            //数据长度不足去处理
            if (_buffer.size() < _message_len) {
                _b_recv_pending = true;
                return;
            }

            //prepare to hanlder data
            _b_recv_pending = false;
            QByteArray messageBody = _buffer.mid(0, _message_len);
            _buffer.remove(0, _message_len);
            //process data
            handleMsg(static_cast<ReqID>(_message_id), _message_len, messageBody);
        }
    });

    initHandler();
}

void TcpManager::initHandler()
{
    _handlers.insert(ID_LOGIN_USER_RSP, [this](ReqID id, int len, QByteArray data) {
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
        auto token = jsonObj["token"].toString();
        QJsonObject userInfoObj = jsonObj["user_self_info"].toObject();
        auto uid = userInfoObj["uid"].toInt();
        auto name = userInfoObj["name"].toString();
        auto headIcon = userInfoObj["icon"].toString();
        auto sex = userInfoObj["sex"].toInt();
        auto email = userInfoObj["email"].toString();

        UserInfo userInfo;
        userInfo.uid = uid;
        userInfo.name = name.toStdString();
        userInfo.icon = headIcon.toStdString();
        userInfo.sex = sex;
        userInfo.email = email.toStdString();

        std::vector<std::shared_ptr<MusicInfo>> musicList;
        QJsonArray jsonArray = jsonObj["music_list"].toArray();
        for (auto music : jsonArray) {
            QJsonObject musicObj = music.toObject();
            MusicInfo musicInfo;
            musicInfo.title = musicObj["title"].toString().toStdString();
            musicInfo.album = musicObj["album"].toString().toStdString();
            musicInfo.song_icon = musicObj["song_icon"].toString().toStdString();
            musicInfo.artists = musicObj["artists"].toString().toStdString();
            musicInfo.duration = musicObj["duration"].toInt();
            musicInfo.file_url = musicObj["file_url"].toString().toStdString();
            musicInfo.is_like = musicObj["is_like"].toBool();
            musicList.push_back(std::make_shared<MusicInfo>(musicInfo));
        }

        UserManager::GetInstance()->setUserInfo(std::make_shared<UserInfo>(userInfo));
        UserManager::GetInstance()->setToken(token);

        emit sig_switch_mainwindow();
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
