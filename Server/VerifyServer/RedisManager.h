#pragma once
#include "global.h"
#include "hiredis.h"
#include "Singleton.h"

class RedisConPool {
public:
	RedisConPool(std::size_t poolSize, const char* host, int port, const char* pwd) : _poolSize(poolSize), _host(host), _port(port) {
		for (std::size_t i = 0; i < _poolSize; ++i) {
			auto* context = redisConnect(host, port);
			if (nullptr == context || context->err != 0) {
				if (context) redisFree(context);
				continue;
			}
			auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
			if (reply->type == REDIS_REPLY_ERROR) {
				std::cout << "Redis认证失败" << std::endl;
				freeReplyObject(reply);
				continue;
			}

			//创建连接成功，释放其他资源
			freeReplyObject(reply);
			_connections.push(std::unique_ptr<redisContext>(context));
		}
	}

	~RedisConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		while (!_connections.empty()) _connections.pop();
	}

	std::unique_ptr<redisContext> getConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_b_stop) return true;
			return !_connections.empty();
		});
		if (_b_stop) return nullptr;
		auto context = std::move(_connections.front());
		_connections.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<redisContext> context) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop) return;
		_connections.push(std::move(context));
		_cond.notify_one();
	}

	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}
private:
	std::atomic_bool _b_stop = false;
	std::size_t _poolSize = 0;
	const char* _host;
	int _port;
	std::queue<std::unique_ptr<redisContext>> _connections;
	std::mutex _mutex;
	std::condition_variable _cond;
};

class RedisManager : public Singleton<RedisManager>
{
	friend class Singleton<RedisManager>;
public:
	~RedisManager();
	bool Get(const std::string& key, std::string& value);
	bool Set(const std::string& key, const std::string& value);
	bool Auth(const std::string& password);
	bool Expire(const std::string& key, int seconds);
	void Close();
private:
	RedisManager();

	std::unique_ptr<RedisConPool> _pool;
};

