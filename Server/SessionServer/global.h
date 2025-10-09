#pragma once
#include <iostream>
#include <json/value.h>
#include <json/reader.h>
#include <json/json.h>
#include <functional>
#include <boost\asio.hpp>
#include <memory>
#include <mutex>
#include <vector>
#include <queue>

//data define length
#define MAX_LENGTH 1024 * 1024 * 2
#define HEAD_TOTAL_LEN 4				//head data total length
#define HEAD_ID_LEN 2					//head's id data length
#define HEAD_DATA_LEN 2					//head's data length
#define MAX_RECV_QUEUE 1024*1024				//maximum receive length
#define MAX_SEND_QUEUE 1024					//maximum send length

//custom prefix string
#define USER_IP_PREFIX "uip_"
#define USERTOKENPREFIX "utoken_"
#define LOGINCOUNT		"logincount"
#define USER_BASE_INFO	"ubaseinfo_"
#define USER_SESSION_PREFIX "usession_"
#define LOCK_PREFIX "lock_"
#define LOCK_COUNT "lockcount"

// 分布式锁的持有时间
#define LOCK_TIME_OUT 10
// 分布式锁的重试时间
#define ACQUIRE_TIME_OUT 5

#define DEFAULT_COLLECT_SONGLIST false		// 默认收藏歌单
#define OTHER_COLLECT_SONGLIST true		// 其他收藏歌单

enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,  //Json解析错误
	RPCFailed = 1002,  //RPC请求错误
	VarifyExpired = 1003, //验证码过期
	VarifyCodeErr = 1004, //验证码错误
	UserExist = 1005,       //用户已经存在
	PasswdErr = 1006,    //密码错误
	EmailNotMatch = 1007,  //邮箱不匹配
	PasswdUpFailed = 1008,  //更新密码失败
	PasswdInvalid = 1009,   //密码更新失败
	TokenInvalid = 1010,   //Token失效
	UidInvalid = 1011,  //uid无效
	EtherInvalid = 0x7777,  //Ether错误
};

class Defer {
public:
	Defer(std::function<void()> func) : _func(func) {}

	~Defer() {
		_func();
	}

private:
	std::function<void()> _func;
};

enum MSG_ID {
	ID_LOGIN_USER_REQ = 1004,
	ID_LOGIN_USER_RSP = 1005,
	ID_UPLOAD_FILE_REQ = 1006,
	ID_UPLOAD_FILE_RSP = 1007,
	ID_UPLOAD_META_TYPE_REQ = 1008,
	ID_UPLOAD_META_TYPE_RSP = 1009,
	ID_NOTIFY_OFF_LINE_REQ = 1010,		// 通知用户下线
    ID_NOTIFY_OFF_LINE_RSP = 1011,
	ID_HEARTBEAT_REQ = 1012,
    ID_HEARTBEAT_RSP = 1013,
	ID_GET_PWD_SALT = 1014,             // 获取密码盐值       Http
	ID_COLLECT_SONG_REQ = 1015,         // 收藏歌曲请求       Tcp
	ID_COLLECT_SONG_RSP = 1016,         // 收藏歌曲回复       Tcp
};