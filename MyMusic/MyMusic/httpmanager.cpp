#include "httpmanager.h"
#include <QJsonDocument>
#include <QNetworkReply>

void HttpManager::PostRequest(QUrl url, QJsonObject json, ReqID req_id, Modules mod)
{
    QByteArray data = QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    qDebug() << "Request url is: " << url;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));
    auto self = shared_from_this();
    QNetworkReply* reply = _manager.post(request, data);
    connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod]{
        //错误处理
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            emit self->sig_http_finish(req_id, "", ErrorCode::ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }

        QString res = reply->readAll();
        emit self->sig_http_finish(req_id, res, ErrorCode::SUCCESS, mod);
        reply->deleteLater();
    });
}

void HttpManager::slot_http_finish(ReqID id, QString res, ErrorCode err, Modules mod)
{
    switch (mod) {
    case Modules::REGISTERMOD:
        emit sig_reg_mod_finish(id, res, err);
        break;
    case Modules::LOGINMOD:
        emit sig_login_mod_finish(id, res, err);
        break;
    default:
        qDebug() << "Error: undefine error!!!";
        break;
    }
}

HttpManager::HttpManager()
{
    connect(this, &HttpManager::sig_http_finish, this, &HttpManager::slot_http_finish);
}


