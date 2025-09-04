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
	Error_Json = 1001,			//Json��������
	RPCFailed = 1002,			//RPC�������
	VerifyExpired = 1003,		//��֤�����
	VerifyCodeErr = 1004,		//��֤�����
	UserExist = 1005,			//�û��Ѿ�����
	EmailExist = 1006,			//�����Ѿ�����
	PasswdErr = 1007,			//�������
	EmailNotMatch = 1008,		//���䲻ƥ��
	PasswdUpFailed = 1009,		//��������ʧ��
	PasswdInvalid = 1010,		//������Ч
	OtherError					//��������
};

class Defer {
public:
	Defer(std::function<void()> func) : _func(func) {}
	~Defer() { _func(); }
private:
	std::function<void()> _func;
};