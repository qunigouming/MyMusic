#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <mutex>
#include "dataInfo.h"
#include "Server.h"
#include <memory>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::SessionService;

using message::KickUserReq;
using message::KickUserRsp;

class SessionServiceImpl final : public SessionService::Service
{
public:
	SessionServiceImpl() = default;
	Status NotifyKickUser(grpc::ServerContext* context, const KickUserReq* request, KickUserRsp* response) override;
	void RegisterServer(std::shared_ptr<Server> server);
private:
	std::shared_ptr<Server> _server;
};

