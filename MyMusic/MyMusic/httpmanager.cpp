#include "httpmanager.h"
#include <QJsonDocument>
#include <QNetworkReply>
#include <QFile>
#include "LogManager.h"
#include <QHttpMultiPart>
#include <memory>

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

void HttpManager::UploadFile(QUrl url, QString filepath, QJsonObject extraFields, ReqID req_id, Modules mod)
{
    QFile file(filepath);
    if (!file.exists()) {
        LOG(INFO) << "Upload File is not exitst;" << filepath.toStdString();
        emit sig_http_finish(req_id, "", ErrorCode::ERR_UPLOAD_FAILED, mod);
        return;
    }

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + file.fileName() + "\""));
    if (file.open(QIODevice::ReadOnly)) {
        LOG(INFO) << "Cannot open file " << file.fileName().toStdString();
        emit sig_http_finish(req_id, "", ErrorCode::ERR_UPLOAD_FAILED, mod);
        return;
    }
    filePart.setBodyDevice(&file);
    file.setParent(multiPart);
    multiPart->append(filePart);

    for (auto it = extraFields.begin(); it != extraFields.end(); ++it) {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + it.key() + "\""));
        part.setBody(it.value().toString().toUtf8());
        multiPart->append(part);
    }

    QNetworkRequest request(url);
    auto self = shared_from_this();
    QNetworkReply* reply = _manager.post(request, multiPart);
    connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod] {
        if (reply->error() != QNetworkReply::NoError) {
            LOG(INFO) << "Upload error: " << reply->errorString().toStdString();
            emit self->sig_http_finish(req_id, "", ErrorCode::ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }

        QString res = reply->readAll();
        emit self->sig_http_finish(req_id, res, ErrorCode::SUCCESS, mod);
        reply->deleteLater();
    });
    multiPart->setParent(reply);
    connect(reply, &QNetworkReply::uploadProgress, this, [](qint64 bytesSent, qint64 bytesTotal) {
        LOG(INFO) << "Upload progress: " << bytesSent << "/" << bytesTotal;
        if (bytesSent == bytesTotal) {
            LOG(INFO) << "Upload file finished";
        }
    });
    LOG(INFO) << "Start uploading file: " << filepath.toStdString() << " to " << url.toString().toStdString();
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
    case Modules::FILETRANSMOD:
        emit sig_file_trans_mod_finish(id, res, err);
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


