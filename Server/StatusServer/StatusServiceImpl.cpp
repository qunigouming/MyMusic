#include "StatusServiceImpl.h"
#include "ConfigManager.h"
#include "global.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>
#include "RedisManager.h"
#include <limits>

std::string generate_unique_token() {
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	return to_string(uuid);
}

StatusServiceImpl::StatusServiceImpl()
{
	//添加ChatServer服务器信息
	auto& config = ConfigManager::GetInstance();
	auto server_list = config["ChatServers"]["Name"];
	std::stringstream ss(server_list);
	std::vector<std::string> words;
	std::string word;
	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}

	for (auto& name : words) {
		if (config[name]["Name"].empty())	continue;
		ChatServer server;
		server.host = config[name]["Host"];
		server.port = config[name]["Port"];
		server.name = config[name]["Name"];
		_servers[name] = server;
	}
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* response)
{
	const auto& server = getChatServer();
	response->set_host(server.host);
	response->set_port(server.port);
	response->set_token(generate_unique_token());
	response->set_error(ErrorCodes::Success);
	insertToken(request->id(), response->token());
	return Status::OK;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* response)
{
	auto uid = request->uid();
	auto token = request->token();

	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisManager::GetInstance()->Get(token_key, token_value);
	if (success) {
		response->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}
	if (token_value != token) {
		response->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
	}
	response->set_error(ErrorCodes::Success);
	return Status::OK;
}

ChatServer StatusServiceImpl::getChatServer()
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto minServer = _servers.begin()->second;
	//auto con_count = RedisManager::GetInstance()->HGet(LOGINCOUNT, minServer.name);
	////不存在默认最大值
	//if (con_count.empty())	minServer.con_count = INT_MAX;
	//else minServer.con_count = std::stoi(con_count);
	//for (auto& server : _servers) {
	//	if (server.second.name == minServer.name)	continue;
	//	auto min_count_str = RedisManager::GetInstance()->HGet(LOGINCOUNT, server.second.name);
	//	if (min_count_str.empty()) server.second.con_count = INT_MAX;
	//	else server.second.con_count = stoi(min_count_str);
	//	if (server.second.con_count < minServer.con_count) minServer = server.second;
	//}
	return minServer;
}

void StatusServiceImpl::insertToken(int id, std::string token)
{
	std::string id_str = std::to_string(id);
	std::string token_key = USERTOKENPREFIX + id_str;
	RedisManager::GetInstance()->Set(token_key, token);
}
