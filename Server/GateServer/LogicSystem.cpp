#include "LogicSystem.h"
#include "VerifyGrpcClient.h"
#include "RedisManager.h"
#include "MysqlManager.h"
#include "StatusGrpcClient.h"
#include "LogManager.h"

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
		LOG(INFO) << "receive body is " << body_str;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		//解析错误
		if (!parse_success) {
			LOG(ERROR) << "Failed to parse JSON data!";
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return false;
		}

		//Json格式错误
		if (!src_root.isMember("email")) {
			LOG(ERROR) << "Failed to parse JSON data!";
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return false;
		}

		auto email = src_root["email"].asString();
		GetVerifyRsp reply = VerifyGrpcClient::GetInstance()->GetVerifyCode(email);
		LOG(INFO) << "email is :" << email;
		root["error"] = reply.error();
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});

	RegisterPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = beast::buffers_to_string(connection->_request.body().data());
		LOG(INFO) << "receive body is " << body_str;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value value;
		Json::Reader reader;
		Json::Value retJson;
		Defer defer([&connection, &retJson] {
			std::string jsonstr = retJson.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
		});
		//解析Json
		bool parse_success = reader.parse(body_str, value);
		if (!parse_success) {
			LOG(ERROR) << "Failed to parse Json data!";
			retJson["error"] = ErrorCodes::Error_Json;
			return false;
		}
		auto name = value["name"].asString();
		auto email = value["email"].asString();
		auto passwd_hash = value["passwd_hash"].asString();
		auto passwd_salt = value["passwd_salt"].asString();

		//判断验证码的合理性
		std::string verify_code;
		bool b_get_success = RedisManager::GetInstance()->Get(CODEPREFIX + email, verify_code);
		if (!b_get_success) {
			LOG(INFO) << "verify code are expired";
			retJson["error"] = ErrorCodes::VerifyCodeErr;
			return false;
		}

		if (verify_code != value["verifycode"].asString()) {
			LOG(ERROR) << "verify code error" << verify_code;
			retJson["error"] = ErrorCodes::VerifyCodeErr;
			return false;
		}

		//注册用户
		int ecode = MysqlManager::GetInstance()->RegUser(name, passwd_hash, passwd_salt, email);
		LOG(INFO) << "register code is: " << ecode;
		retJson["error"] = ecode;
		retJson["email"] = email;
		retJson["user"] = name;
		return true;
	});

	// in name
	// out passwd_salt error
	RegisterPost("/get_password_salt", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = beast::buffers_to_string(connection->_request.body().data());
        LOG(INFO) << "receive body is " << body_str;
        connection->_response.set(http::field::content_type, "text/json");
        Json::Value retJson;
        Defer defer([&connection, &retJson] {
            std::string jsonstr = retJson.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
        });
		Json::Value value;
        Json::Reader reader;
		bool parse_success = reader.parse(body_str, value);
        if (!parse_success) {
            LOG(ERROR) << "Failed to parse Json data!";
            retJson["error"] = ErrorCodes::Error_Json;
            return false;
        }
		auto name = value["name"].asString();
		std::string passwd_salt;
        bool get_success = MysqlManager::GetInstance()->GetPasswdSalt(name, passwd_salt);
        if (!get_success) {
            LOG(ERROR) << "Failed to get passwd salt";
			if (passwd_salt.empty()) retJson["error"] = ErrorCodes::UserNameInvalid;
			else retJson["error"] = ErrorCodes::OtherError;
            return false;
        }
		retJson["error"] = ErrorCodes::Success;
        retJson["passwd_salt"] = passwd_salt;
		return true;
	});

	// in name, passwd_hash
	// out error
	RegisterPost("/get_server", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = beast::buffers_to_string(connection->_request.body().data());
		LOG(INFO) << "receive body is " << body_str;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value value;
		Json::Reader reader;
		Json::Value retJson;
		Defer defer([&connection, &retJson] {
			std::string jsonstr = retJson.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
		});
		bool parse_success = reader.parse(body_str, value);
		if (!parse_success) {
			LOG(ERROR) << "Failed to parse Json data!";
			retJson["error"] = ErrorCodes::Error_Json;
			return false;
		}

		//验证登录用户的有效性
		auto name = value["name"].asString();
		auto passwd_hash = value["passwd_hash"].asString();
		int id = 0;
		bool login_valid = MysqlManager::GetInstance()->LoginValid(name, passwd_hash, id);
		if (!login_valid) {
			retJson["error"] = ErrorCodes::PasswdErr;
			return true;
		}

		//获取聊天服务器的端口地址
		auto reply = StatusGrpcClient::GetInstance()->GetChatServer(id);
		if (reply.error()) {
			LOG(ERROR) << "grpc get chat server failed, error is " << reply.error();
			retJson["error"] = ErrorCodes::RPCFailed;
			return true;
		}

		LOG(INFO) << "user login successs, id is: " << id;
		retJson["error"] = 0;
		retJson["id"] = id;
		retJson["token"] = reply.token();
		retJson["host"] = reply.host();
		retJson["port"] = reply.port();
		return true;
	});
}
