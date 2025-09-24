#pragma once

#include "global.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::SessionService;

using message::KickUserReq;
using message::KickUserRsp;

class SessionConPool {
public:
	SessionConPool(std::size_t poolSize, std::string host, std::string port)
		: _poolSize(poolSize), _host(host), _port(port), _stop(false) {
		for (size_t i = 0; i < poolSize; ++i) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(_host + ":" + _port, grpc::InsecureChannelCredentials());
			_conPool.push(SessionService::NewStub(channel));
		}
	}

	~SessionConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		Close();
		while (!_conPool.empty()) {
			_conPool.pop();
		}
	}

	std::unique_ptr<SessionService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_stop)	return true;
			return !_conPool.empty();
		});
		if (_stop)	return nullptr;
		auto con = std::move(_conPool.front());
        _conPool.pop();
        return con;
	}

	void returnConnection(std::unique_ptr<SessionService::Stub> con) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_stop)	return;
		_conPool.push(std::move(con));
		_cond.notify_one();
	}

	void Close() {
		_stop = true;
		_cond.notify_all();
	}
private:
	std::size_t _poolSize;
	std::string _host;
    std::string _port;
    std::queue<std::unique_ptr<SessionService::Stub>> _conPool;
    std::mutex _mutex;
	std::condition_variable _cond;
	std::atomic_bool _stop;
};

class SessionGrpcClient : public Singleton<SessionGrpcClient>
{
	friend class Singleton<SessionGrpcClient>;
public:
	~SessionGrpcClient() = default;

	KickUserRsp NotifyKickUser(const std::string& server_ip, const KickUserReq& req);
private:
	SessionGrpcClient();
	std::unordered_map<std::string, std::unique_ptr<SessionConPool>> _pool;
};

