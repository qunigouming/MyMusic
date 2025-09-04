#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <iostream>
#include <queue>
#include <map>
#include <unordered_map>

#define USERTOKENPREFIX "utoken_"
#define LOGINCOUNT "logincount"

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

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
};

class Defer {
public:
	Defer(std::function<void()> func) : _func(func) {}
	~Defer() { _func(); }
private:
	std::function<void()> _func;
};