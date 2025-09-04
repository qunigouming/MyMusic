#include "VerifyGrpcServer.h"
#include "ConfigManager.h"

VerifyGrpcServer::VerifyGrpcServer()
{
	auto& cfgMgr = ConfigManager::GetInstance();
	std::string host = cfgMgr["VerifyServer"]["Host"];
	std::string port = cfgMgr["VerifyServer"]["Port"];
	_pool.reset(new VerifyConPool(5, host, port));
}
