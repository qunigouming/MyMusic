#ifndef GLOBAL_H
#define GLOBAL_H

/******************************************************************************
 *
 * @file       global.h
 * @brief      glabal define text, there have common protocol
 *
 * @author     qunigouming
 * @date       2025/07/02
 * @history
 *****************************************************************************/

#include <QWidget>
#include <QStyle>
#include <functional>

extern std::function<void(QWidget*)> repolish;

extern QString gate_url_prefix;

enum ReqID {
    ID_GET_VARIFY_CODE = 1001,
    ID_REGISTER_USER = 1002,
    ID_GET_SERVER = 1003,
    ID_LOGIN_USER_REQ = 1004,
    IO_LOGIN_USER_RSP = 1005
};

enum Modules {
    REGISTERMOD = 0,
    LOGINMOD = 1,
};

enum ErrorCode {
    SUCCESS = 0,
    ERR_JSON = 1,
    ERR_NETWORK = 2,
    VerifyExpired = 1003,		//验证码过期
    VerifyCodeErr = 1004,		//验证码错误
    UserExist = 1005,			//用户已经存在
    EmailExist = 1006,			//邮箱已经存在
    EtherInvalid = 0x777        //Ether错误
};

struct ServerInfo {
    int Id;
    QString Host;
    QString Port;
    QString Token;
};

// 播放模式
enum class PlayModel {
    LISTLOOP = 0,
    SINGLELOOP,
    RANDOM,
    SEQUENCE
};

#endif // GLOBAL_H
