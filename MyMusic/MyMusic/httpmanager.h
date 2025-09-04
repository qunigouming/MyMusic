#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

/******************************************************************************
 *
 * @file       httpmanager.h
 * @brief      Http Manager Class, be used to manage Http connection
 *
 * @author     qunigouming
 * @date       2025/07/02
 * @history
 *****************************************************************************/

#include "Singleton.h"
#include "global.h"
#include <QNetworkAccessManager>
#include <QObject>
#include <QJsonObject>
#include <QUrl>

class HttpManager : public QObject, public Singleton<HttpManager>, public std::enable_shared_from_this<HttpManager>
{
    Q_OBJECT
    friend class Singleton<HttpManager>;
public:
    ~HttpManager() = default;
    void PostRequest(QUrl url, QJsonObject json, ReqID req_id, Modules mod);

signals:
    void sig_http_finish(ReqID id, QString res, ErrorCode err, Modules mod);            //private
    void sig_reg_mod_finish(ReqID id, QString res, ErrorCode err);
    void sig_login_mod_finish(ReqID id, QString res, ErrorCode err);

private slots:
    void slot_http_finish(ReqID id, QString res, ErrorCode err, Modules mod);

private:
    HttpManager();

    QNetworkAccessManager _manager;
};

#endif // HTTPMANAGER_H
