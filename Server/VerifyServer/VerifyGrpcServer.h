#pragma once

#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "Singleton.h"
#include <queue>

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

	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}

	std::unique_ptr<VerifyService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_b_stop)	return true;
			return !_connections.empty();
		});
		if (_b_stop) return nullptr;
		auto connect = std::move(_connections.front());
		_connections.pop();
		return connect;
	}

	void returnConnection(std::unique_ptr<VerifyService::Stub> connect) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop) return;
		_connections.push(std::move(connect));
		_cond.notify_one();
	}

private:
	std::atomic_bool _b_stop = false;
	std::size_t _poolSize = 0;
	std::string _host;
	std::string _port;
	std::queue<std::unique_ptr<VerifyService::Stub>> _connections;
	std::mutex _mutex;
	std::condition_variable _cond;
};

class VerifyGrpcServer : Singleton<VerifyGrpcServer>
{
	friend class Singleton<VerifyGrpcServer>;
public:
	~VerifyGrpcServer() = default;

private:
	VerifyGrpcServer();
	std::unique_ptr<VerifyConPool> _pool;
};

