#pragma once
#include "global.h"
#include "hiredis.h"
#include "Singleton.h"

class RedisConPool {
public:
	RedisConPool(std::size_t poolSize, const char* host, int port, const char* pwd) : _poolSize(poolSize), _host(host), _port(port), _pwd(pwd) {
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
			std::cout << "Redis认证成功" << std::endl;
			_connections.push(context);
		}

		_checkThread = std::thread([this]() {
			while (!_b_stop) {
				_counter++;
				if (_counter >= 60) {
					checkThread();
					_counter = 0;
				}

				std::this_thread::sleep_for(std::chrono::seconds(1));		//30秒发送ping
			}
		});
	}

	~RedisConPool() {

	}

	//清空连接
	void ClearConnections() {
		std::lock_guard<std::mutex> lock(_mutex);
		while (!_connections.empty()) {
			auto* context = _connections.front();
			redisFree(context);
			_connections.pop();
		}
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
		_checkThread.join();
	}

private:
	void checkThread() {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop)	return;
		auto poolSize = _connections.size();
		//检查连接
		for (int i = 0; i < poolSize && !_b_stop; i++) {
			redisContext* context = _connections.front();
			_connections.pop();
			try {
				auto reply = (redisReply*)redisCommand(context, "PING");
				if (!reply) {
					std::cout << "redis ping error" << std::endl;
					_connections.push(context);
					continue;
				}
			}
			catch (std::exception& exp) {
				std::cout << "Error keeping connection alive: " << exp.what() << std::endl;
				redisFree(context);
				context = redisConnect(_host, _port);
				if (context == nullptr || context->err != 0) {
					if (context != nullptr)	redisFree(context);
					continue;
				}
				auto reply = (redisReply*)redisCommand(context, "AUTH %s", _pwd);

				if (reply->type == REDIS_REPLY_ERROR) {
					std::cout << "认证失败" << std::endl;
					freeReplyObject(reply);
					continue;
				}

				//认证成功
				freeReplyObject(reply);
				std::cout << "认证成功" << std::endl;
				_connections.push(context);
			}
		}
	}

private:
	std::atomic_bool _b_stop = false;
	std::size_t _poolSize = 0;
	const char* _host;
	const char* _pwd;
	int _port;
	std::queue<redisContext*> _connections;
	std::mutex _mutex;
	std::condition_variable _cond;
	std::thread _checkThread;
	int _counter = 0;
};

class RedisManager : public Singleton<RedisManager>
{
	friend class Singleton<RedisManager>;
public:
	~RedisManager();
	bool Get(const std::string& key, std::string& value);
	bool Set(const std::string& key, const std::string& value);
	std::string HGet(const std::string& key, const std::string& hkey);
	bool Auth(const std::string& password);
	void Close();
private:
	RedisManager();

	std::unique_ptr<RedisConPool> _pool;
};

