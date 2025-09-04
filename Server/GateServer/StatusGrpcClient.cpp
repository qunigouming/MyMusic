#include "StatusGrpcClient.h"
#include "ConfigManager.h"

GetChatServerRsp StatusGrpcClient::GetChatServer(int id)
{
	ClientContext context;
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
