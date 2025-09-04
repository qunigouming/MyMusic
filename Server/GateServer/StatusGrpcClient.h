#pragma once
#include "global.h"
#include "Singleton.h"
#include <grpcpp\grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;

class StatusConPool {
public:
	StatusConPool(std::size_t poolSize, std::string host, std::string port) : _poolSize(poolSize), _host(host), _port(port) {
		for (std::size_t i = 0; i < poolSize; ++i) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
			_pool.push(StatusService::NewStub(channel));
		}
	}

	~StatusConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		Close();
		while (!_pool.empty()) _pool.pop();
	}

	std::unique_ptr<StatusService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_b_stop)	return true;
			return !_pool.empty();
		});
		if (_b_stop)	return nullptr;
		auto con = std::move(_pool.front());
		_pool.pop();
		return con;
	}

	void returnConnection(std::unique_ptr<StatusService::Stub> con) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop)	return;
		_pool.push(std::move(con));
		_cond.notify_one();
	}

	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}
private:
	std::atomic_bool _b_stop = false;
	std::size_t _poolSize = 0;
	std::string _host;
	std::string _port;
	std::queue<std::unique_ptr<StatusService::Stub>> _pool;
	std::condition_variable _cond;
	std::mutex _mutex;
};

class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
	friend class Singleton<StatusGrpcClient>;
public:
	~StatusGrpcClient() = default;
	GetChatServerRsp GetChatServer(int id);
private:
	StatusGrpcClient();
	std::unique_ptr<StatusConPool> _pool;
};

