#include "LogicSystem.h"
#include "VerifyGrpcClient.h"
#include "RedisManager.h"
#include "MysqlManager.h"
#include "StatusGrpcClient.h"

void LogicSystem::RegisterPost(std::string url, HttpTask task)
{
	_post_handlers.insert(std::make_pair(url, task));
}

void LogicSystem::RegisterGet(std::string url, HttpTask task)
{
	_get_handlers.insert(std::make_pair(url, task));
}

bool LogicSystem::HandlePost(std::string url, std::shared_ptr<HttpConnection> connection)
{
	if (_post_handlers.find(url) == _post_handlers.end())	return false;
	_post_handlers[url](connection);
	return true;
}

bool LogicSystem::HandleGet(std::string url, std::shared_ptr<HttpConnection> connection)
{
	if (_get_handlers.find(url) == _get_handlers.end())	return false;
	_get_handlers[url](connection);
	return true;
}

LogicSystem::LogicSystem()
{
	RegisterPost("/get_verifycode", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		//解析错误
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return false;
		}

		//Json格式错误
		if (!src_root.isMember("email")) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return false;
		}

		auto email = src_root["email"].asString();
		GetVerifyRsp reply = VerifyGrpcClient::GetInstance()->GetVerifyCode(email);
		std::cout << "email is :" << email << std::endl;
		root["error"] = reply.error();
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});

	RegisterPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value value;
		Json::Reader reader;
		Json::Value root;
		Defer defer([&connection, &root] {
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
		});
		//解析Json
		bool parse_success = reader.parse(body_str, root);
		if (!parse_success) {
			std::cout << "Failed to parse Json data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			return false;
		}
		auto name = root["name"].asString();
		auto email = root["email"].asString();
		auto pwd = root["passwd"].asString();

		//判断验证码的合理性
		std::string verify_code;
		bool b_get_success = RedisManager::GetInstance()->Get(CODEPREFIX + email, verify_code);
		if (!b_get_success) {
			std::cout << "verify code are expired" << std::endl;
			root["error"] = ErrorCodes::VerifyCodeErr;
			return false;
		}

		if (verify_code != root["verifycode"].asString()) {
			std::cout << "verify code error" << verify_code << std::endl;
			root["error"] = ErrorCodes::VerifyCodeErr;
			return false;
		}

		//注册用户
		int ecode = MysqlManager::GetInstance()->RegUser(name, pwd, email);
		std::cout << "register code is: " << ecode << std::endl;
		root["error"] = ecode;
		root["email"] = email;
		root["user"] = name;
		root["passwd"] = pwd;
		return true;
	});

	RegisterPost("/user_login", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value value;
		Json::Reader reader;
		Json::Value root;
		Defer defer([&connection, &root] {
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
		});
		bool parse_success = reader.parse(body_str, root);
		if (!parse_success) {
			std::cout << "Failed to parse Json data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			return false;
		}

		//验证登录用户的有效性
		auto name = root["name"].toStyledString();
		auto passwd = root["passwd"].toStyledString();
		int id = 0;
		bool login_valid = MysqlManager::GetInstance()->LoginValid(name, passwd, id);
		if (!login_valid) {
			root["error"] = ErrorCodes::PasswdErr;
			return true;
		}

		//获取聊天服务器的端口地址
		auto reply = StatusGrpcClient::GetInstance()->GetChatServer(id);
		if (reply.error()) {
			std::cout << "grpc get chat server failed, error is " << reply.error() << std::endl;
			root["error"] = ErrorCodes::RPCFailed;
			return true;
		}

		std::cout << "user login successs, id is: " << id << std::endl;
		root["error"] = 0;
		root["id"] = id;
		root["token"] = reply.token();
		root["host"] = reply.host();
		root["port"] = reply.port();
		return true;
	});
}
