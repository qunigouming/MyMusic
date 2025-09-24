#include "SessionServiceImpl.h"
#include "UserManager.h"

Status SessionServiceImpl::NotifyKickUser(grpc::ServerContext* context, const KickUserReq* request, KickUserRsp* response)
{
	auto uid = request->uid();
	auto session = UserManager::GetInstance()->GetSession(uid);

	Defer defer([request, response]() {
		response->set_error(ErrorCodes::Success);
		response->set_uid(request->uid());
	});

	if (session == nullptr)	return Status::OK;

	// 踢掉用户
	session->NotifyOffline(uid);
	_server->ClearSession(session->GetSessionId());

	return Status::OK;
}

void SessionServiceImpl::RegisterServer(std::shared_ptr<Server> server)
{
	_server = server;
}
