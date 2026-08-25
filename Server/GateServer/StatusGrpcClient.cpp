#include "StatusGrpcClient.h"
#include "ConfigManager.h"
#include <chrono>

GetChatServerRsp StatusGrpcClient::GetChatServer(int id)
{
	ClientContext context;
	// 防止对端不可达时无限阻塞（曾导致 /get_server 卡死）
	context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
	GetChatServerReq request;
	GetChatServerRsp reply;
	request.set_id(id);
	auto con = _pool->getConnection();
	Status status = con->GetChatServer(&context, request, &reply);
	Defer defer([&con, this] {
		_pool->returnConnection(std::move(con));
	});
	if (status.ok())	return reply;
	else {
		reply.set_error(ErrorCodes::RPCFailed);
		return reply;
	}
}

StatusGrpcClient::StatusGrpcClient()
{
	auto& cfgMgr = ConfigManager::GetInstance();
	std::string host = cfgMgr["StatusServer"]["Host"];
	std::string port = cfgMgr["StatusServer"]["Port"];
	_pool.reset(new StatusConPool(5, host, port));
}
