#pragma once
#include <iostream>
#include <json/value.h>
#include <json/reader.h>
#include <json/json.h>
#include <functional>
#include <boost\asio.hpp>
#include <memory>
#include <mutex>

//data define length
#define MAX_LENGTH 1024 * 1024 * 2
#define HEAD_TOTAL_LEN 4				//head data total length
#define HEAD_ID_LEN 2					//head's id data length
#define HEAD_DATA_LEN 2					//head's data length
#define MAX_RECV_QUEUE 1024*1024				//maximum receive length
#define MAX_SEND_QUEUE 1024					//maximum send length

//custom prefix string
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

//User request message id
enum MESSAGE_REQ_IDS {
	MSG_USER_LOGIN_REQ = 0x2001,	//User login
    ID_HEARTBEAT_REQ = 0x2002,		// Heartbeat request
};

//User response message id
enum MSSAGE_RSP_IDS {
	MSG_USER_LOGIN_RSP = 0x5001,		//User login backsourcing
	ID_HEARTBEAT_RSP = 0x5002,			// Heartbeat response
};