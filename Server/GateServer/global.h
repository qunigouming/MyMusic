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
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#define CODEPREFIX "code_"

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,			//Json解析错误
	RPCFailed = 1002,			//RPC请求错误
	VerifyExpired = 1003,		//验证码过期
	VerifyCodeErr = 1004,		//验证码错误
	UserExist = 1005,			//用户已经存在
	EmailExist = 1006,			//邮箱已经存在
	PasswdErr = 1007,			//密码错误
	EmailNotMatch = 1008,		//邮箱不匹配
	PasswdUpFailed = 1009,		//更新密码失败
	PasswdInvalid = 1010,		//密码无效
	OtherError					//其他错误
};

class Defer {
public:
	Defer(std::function<void()> func) : _func(func) {}
	~Defer() { _func(); }
private:
	std::function<void()> _func;
};