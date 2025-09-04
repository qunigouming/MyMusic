#pragma once
#include <grpcpp\grpcpp.h>
#include "Singleton.h"
#include "global.h"
#include "message.grpc.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class VerifyConPool {
public:
	VerifyConPool(std::size_t poolSize, std::string host, std::string port) : _poolSize(poolSize), _host(host), _port(port) {
		for (std::size_t i = 0; i < poolSize; ++i) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
			_connections.push(VerifyService::NewStub(channel));
		}
	}

	~VerifyConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		Close();
		while (!_connections.empty()) _connections.pop();
	}

	void returnConnection(std::unique_ptr<VerifyService::Stub> context) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop)	return;
		_connections.push(std::move(context));
		_con.notify_one();
	}

	std::unique_ptr<VerifyService::Stub> GetConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_con.wait(lock, [this] {
			if (_b_stop)	return true;
			return !_connections.empty();
		});
		auto context = std::move(_connections.front());
		_connections.pop();
		return context;
	}

	void Close() {
		_b_stop = true;
		_con.notify_all();
	}
private:
	std::atomic_bool _b_stop = false;
	std::string _host;
	std::string _port;
	std::size_t _poolSize = 0;
	std::queue<std::unique_ptr<VerifyService::Stub>> _connections;
	std::mutex _mutex;
	std::condition_variable _con;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	GetVerifyRsp GetVerifyCode(std::string email) {
		ClientContext context;
		GetVerifyRsp reply;
		GetVerifyReq request;
		request.set_email(email);
		auto stub = _pool->GetConnection();
		Status status = stub->GetVerifyCode(&context, request, &reply);

		_pool->returnConnection(std::move(stub));		//归还连接给连接池
		if (!status.ok()) {
			reply.set_error(ErrorCodes::RPCFailed);		//设置为RPC错误
		}
		return reply;
	}
private:
	VerifyGrpcClient();
	std::unique_ptr<VerifyConPool> _pool;
};

