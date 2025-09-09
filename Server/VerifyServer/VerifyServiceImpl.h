#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <mutex>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class VerifyServiceImpl final : public VerifyService::Service
{
public:
	VerifyServiceImpl() = default;
	Status GetVerifyCode(ServerContext* context, const GetVerifyReq* request, GetVerifyRsp* response) override;
};

