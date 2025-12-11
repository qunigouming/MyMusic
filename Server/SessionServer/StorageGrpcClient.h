#pragma once

#include "Singleton.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <queue>
#include <memory>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::FileMetadata;
using message::UploadImageRequest;
using message::UploadImageResponse;
using message::StorageService;

class StorageConPool {
public:
	StorageConPool(std::size_t poolSize, std::string host, std::string port) : _poolSize(poolSize), _host(host), _port(port) {
		for (std::size_t i = 0; i < poolSize; ++i) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
			_pool.push(StorageService::NewStub(channel));
		}
	}

	~StorageConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		Close();
		while (!_pool.empty()) _pool.pop();
	}

	std::unique_ptr<StorageService::Stub> getConnection() {
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

	void returnConnection(std::unique_ptr<StorageService::Stub> con) {
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
	std::queue<std::unique_ptr<StorageService::Stub>> _pool;
	std::condition_variable _cond;
	std::mutex _mutex;
};

class StorageGrpcClient : public Singleton<StorageGrpcClient>
{
	friend class Singleton<StorageGrpcClient>;
public:
	~StorageGrpcClient() = default;
	UploadImageResponse UploadImage(std::string file_name, std::string file_data);

private:
    StorageGrpcClient();

private:
	std::unique_ptr<StorageConPool> _pool;
};

