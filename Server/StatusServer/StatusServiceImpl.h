#pragma once
#include <grpcpp\grpcpp.h>
#include "message.grpc.pb.h"
#include <mutex>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;

class ChatServer {
public:
	ChatServer() : host(""), port(""), name("") {}
	ChatServer(const ChatServer& cs) : host(cs.host), port(cs.port), name(cs.name), con_count(cs.con_count) {}
	ChatServer& operator=(const ChatServer& cs) {
		if (&cs == this) return *this;
		host = cs.host;
		port = cs.port;
		name = cs.name;
		con_count = cs.con_count;
		return *this;
	}
	std::string host;
	std::string port;
	std::string name;
	int con_count = 0;			//服务器连接数量
};

class StatusServiceImpl final : public StatusService::Service
{
public:
	StatusServiceImpl();
	Status GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* response) override;
	Status Login(ServerContext* context, const LoginReq* request, LoginRsp* response) override;
private:
	ChatServer getChatServer();			//获取连接数最小的服务器
	void insertToken(int id, std::string token);
	std::unordered_map<std::string, ChatServer> _servers;
	std::mutex _mutex;
};

