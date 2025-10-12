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

#define DEFAULT_COLLECT_SONGLIST false
#define OTHER_COLLECT_SONGLIST true

extern std::function<void(QWidget*)> repolish;

extern QString gate_url_prefix;


enum ReqID {
    ID_GET_VARIFY_CODE = 1001,              // 获取验证码         Http
    ID_REGISTER_USER = 1002,                // 注册用户           Http
    ID_GET_SERVER = 1003,                   // 获取服务器参数     Http
    ID_LOGIN_USER_REQ = 1004,               // 登录用户请求       Tcp
    ID_LOGIN_USER_RSP = 1005,               // 登录用户回复       Tcp
    ID_UPLOAD_FILE_REQ = 1006,              // 上传文件请求       Tcp
    ID_UPLOAD_FILE_RSP = 1007,              // 上传文件回复       Tcp
    ID_UPLOAD_META_TYPE_REQ = 1008,         // 上传文件元数据请求 Tcp
    ID_UPLOAD_META_TYPE_RSP = 1009,         // 上传文件元数据回复 Tcp
    ID_NOTIFY_OFF_LINE_REQ = 1010,		    // 通知用户下线       Tcp
    ID_NOTIFY_OFF_LINE_RSP = 1011,          // 暂存
    ID_HEARTBEAT_REQ = 1012,                // 心跳请求           Tcp
    ID_HEARTBEAT_RSP = 1013,                // 心跳回复           Tcp
    ID_GET_PWD_SALT = 1014,                 // 获取密码盐值       Http
    ID_COLLECT_SONG_REQ = 1015,             // 收藏歌曲请求       Tcp
    ID_COLLECT_SONG_RSP = 1016,             // 收藏歌曲回复       Tcp
    ID_GET_COLLECT_SONG_LIST_INFO_REQ = 1017,    // 获取收藏歌单信息请求 Tcp
    ID_GET_COLLECT_SONG_LIST_INFO_RSP = 1018,    // 获取收藏歌单信息回复 Tcp
    ID_GET_COLLECT_SONG_LIST_REQ = 1019,    // 获取收藏歌单歌曲列表请求 Tcp
    ID_GET_COLLECT_SONG_LIST_RSP = 1020,    // 获取收藏歌单歌曲列表回复 Tcp
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
    PasswdErr = 1007,			//密码错误
    EmailNotMatch = 1008,		//邮箱不匹配
    PasswdUpFailed = 1009,		//更新密码失败
    PasswdInvalid = 1010,		//密码无效
    UserNameInvalid = 1011,		//用户名无效
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
    RANDOM
};

#endif // GLOBAL_H
