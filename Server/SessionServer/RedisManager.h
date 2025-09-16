#pragma once
#include "global.h"
#include "hiredis.h"
#include "Singleton.h"
#include <queue>

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
			_connections.push(context);
		}
	}

	~RedisConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		while (!_connections.empty()) _connections.pop();
	}

	redisContext* getConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_b_stop) return true;
			return !_connections.empty();
		});
		if (_b_stop) return nullptr;
		auto* context = _connections.front();
		_connections.pop();
		return context;
	}

	void returnConnection(redisContext* context) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop) return;
		_connections.push(context);
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
	std::queue<redisContext*> _connections;
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
	bool Del(const std::string& key);
	std::string HGet(const std::string& key, const std::string& hkey);
	bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
	bool HDel(const std::string& key, const std::string& field);
	bool Auth(const std::string& password);
	void Close();

	std::string acquireLock(const std::string& lockName, int lockTimeOut, int acquireTimeOut);
	bool releaseLock(const std::string& lockName, const std::string& identifier);
private:
	RedisManager();

	std::unique_ptr<RedisConPool> _pool;
};

