#include "StatusGrpcClient.h"
#include "ConfigManager.h"

LoginRsp StatusGrpcClient::Login(int uid, std::string token)
{
	ClientContext context;
	LoginReq request;
	LoginRsp response;
	request.set_uid(uid);
	request.set_token(token);

	auto con = _pool->getConnection();
	Status status = con->Login(&context, request, &response);
	Defer defer([&con, this] {
		_pool->returnConnection(std::move(con));
	});
	if (status.ok()) return response;
	else {
		response.set_error(ErrorCodes::RPCFailed);
		return response;
	}
}

StatusGrpcClient::StatusGrpcClient()
{
	auto& cfgMgr = ConfigManager::GetInstance();
	std::string host = cfgMgr["StatusServer"]["Host"];
	std::string port = cfgMgr["StatusServer"]["Port"];
	_pool.reset(new StatusConPool(5, host, port));
}
