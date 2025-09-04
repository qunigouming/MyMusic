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

RedisManager::RedisManager()
{
	auto& cfgMgr = ConfigManager::GetInstance();
	auto host = cfgMgr["Redis"]["Host"];
	auto port = cfgMgr["Redis"]["Port"];
	auto pwd = cfgMgr["Redis"]["Passwd"];
	_pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
