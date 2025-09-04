#include "RedisManager.h"
#include "ConfigManager.h"

RedisManager::~RedisManager()
{
	Close();
}

bool RedisManager::Get(const std::string& key, std::string& value)
{
	auto connect = _pool->getConnection();
	if (!connect) return false;
	auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());
	if (!reply) {
		std::cout << "[ Get " << key << " ] failed" << std::endl;
		freeReplyObject(reply);
		return false;
	}
	if (reply->type != REDIS_REPLY_STRING) {
		std::cout << "[ Get " << key << "] failed" << std::endl;
		freeReplyObject(reply);
		return false;
	}
	value = reply->str;
	freeReplyObject(reply);
	return true;
}

bool RedisManager::Set(const std::string& key, const std::string& value)
{
	auto connect = _pool->getConnection();
	if (!connect) return false;
	auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());
	if (!reply) {
		std::cout << "[ Set " << key << " ] failed" << std::endl;
		freeReplyObject(reply);
		return false;
	}
	if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))) {
		std::cout << "[ Set " << key << "] failed" << std::endl;
		freeReplyObject(reply);
		return false;
	}
	freeReplyObject(reply);
	return true;
}

bool RedisManager::Del(const std::string& key)
{
	auto connection = _pool->getConnection();
	if (!connection) {
		return false;
	}
	Defer defer([&]() {
		_pool->returnConnection(connection);
	});
	redisReply* reply = (redisReply*)redisCommand(connection, "DEL %s", key.c_str());
	if (!reply) {
		std::cout << "[ Del " << key << "] failed" << std::endl;
		return false;
	}
	if (reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "[ Del " << key << "] failed" << std::endl;
        freeReplyObject(reply);
        return false;
	}

	std::cout << "[ Del " << key << "] success" << std::endl;
    freeReplyObject(reply);
    return true;
}

std::string RedisManager::HGet(const std::string& key, const std::string& hkey)
{
	auto connect = _pool->getConnection();
	if (connect == nullptr) {
		return "";
	}
	Defer defer([this, &connect] {
		_pool->returnConnection(connect);
	});
	const char* argv[3];
	size_t argvlen[3];
	argv[0] = "HGET";
	argvlen[0] = 4;
	argv[1] = key.c_str();
	argvlen[1] = key.length();
	argv[2] = hkey.c_str();
	argvlen[2] = hkey.length();

	auto reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);
	if (reply == nullptr) {
		std::cout << "reply is nullptr" << std::endl;
		std::cout << "Execut command [ HGet " << key << " " << hkey << "  ] failure ! " << std::endl;
		return "";
	}
	if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(reply);
		std::cout << "Execut command [ HGet " << key << " " << hkey << "  ] failure ! " << std::endl;
		return "";
	}

	std::string value = reply->str;
	freeReplyObject(reply);
	std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
	return value;
}

bool RedisManager::HSet(const std::string& key, const std::string& hkey, const std::string& value)
{
	auto connect = _pool->getConnection();
	if (connect == nullptr) {
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(reply);
		_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(reply);
	_pool->returnConnection(connect);
	return true;
}

bool RedisManager::HDel(const std::string& key, const std::string& field)
{
	auto connect = _pool->getConnection();
	if (connect == nullptr) {
        return false;
    }

	Defer defer([this, &connect] {
		_pool->returnConnection(connect);
	});

	redisReply* reply = (redisReply*)redisCommand(connect, "HDEL %s %s", key.c_str(), field.c_str());
    if (reply == nullptr) {
        std::cout << "Execut command [ HDel " << key << "  " << field << " ] failure ! " << std::endl;
        return false;
    }

	bool success = false;
	if (reply->type == REDIS_REPLY_INTEGER) {
		success = reply->integer > 0;
	}

	freeReplyObject(reply);
	return success;
}

bool RedisManager::Auth(const std::string& password)
{
	auto connect = _pool->getConnection();
	if (!connect) return false;
	auto reply = (redisReply*)redisCommand(connect, "AUTH %s", password.c_str());
	if (reply->type == REDIS_REPLY_ERROR) {
		std::cout << "Redis verify failed!!!" << std::endl;
		freeReplyObject(reply);
		return false;
	}
	freeReplyObject(reply);
	return true;
}

void RedisManager::Close()
{
	_pool->Close();
}

std::string RedisManager::acquireLock(const std::string& lockName, int lockTimeOut, int acquireTimeOut)
{
	auto connect = _pool->getConnection();
	if (!connect) {
		return "";
	}

	Defer defer([this, connect]() {
		_pool->returnConnection(connect);
	});

	
}

bool RedisManager::releaseLock(const std::string& lockName, const std::string& identifier)
{
	return false;
}

RedisManager::RedisManager()
{
	auto& cfgMgr = ConfigManager::GetInstance();
	auto host = cfgMgr["Redis"]["Host"];
	auto port = cfgMgr["Redis"]["Port"];
	auto pwd = cfgMgr["Redis"]["Passwd"];
	_pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
