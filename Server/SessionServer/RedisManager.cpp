#include "RedisManager.h"
#include "ConfigManager.h"
#include "DistributeLock.h"

RedisManager::~RedisManager()
{
	Close();
}

bool RedisManager::Get(const std::string& key, std::string& value)
{
	auto connect = _pool->getConnection();
	if (!connect) return false;
	Defer defer([this, &connect] { _pool->returnConnection(connect); });
	auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());
	if (!reply) {
		LOG(ERROR) << "[ Get " << key << " ] failed";
		// freeReplyObject(reply);
		return false;
	}
	if (reply->type != REDIS_REPLY_STRING) {
		LOG(ERROR) << "[ Get " << key << "] failed";
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
	Defer defer([this, &connect] { _pool->returnConnection(connect); });
	auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());
	if (!reply) {
		LOG(ERROR) << "[ Set " << key << " ] failed";
		// freeReplyObject(reply);
		return false;
	}
	if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))) {
		LOG(ERROR) << "[ Set " << key << "] failed";
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
	Defer defer([this, &connection] { _pool->returnConnection(connection); });
	redisReply* reply = (redisReply*)redisCommand(connection, "DEL %s", key.c_str());
	if (!reply) {
		LOG(ERROR) << "[ Del " << key << "] failed";
		return false;
	}
	if (reply->type != REDIS_REPLY_INTEGER) {
		LOG(ERROR) << "[ Del " << key << "] failed";
        freeReplyObject(reply);
        return false;
	}

	LOG(INFO) << "[ Del " << key << "] success";
    freeReplyObject(reply);
    return true;
}

std::string RedisManager::HGet(const std::string& key, const std::string& hkey)
{
	auto connect = _pool->getConnection();
	if (connect == nullptr) {
		return "";
	}
	Defer defer([this, &connect] { _pool->returnConnection(connect); });
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
		LOG(ERROR) << "reply is nullptr";
		LOG(ERROR) << "Execute command [ HGet " << key << " " << hkey << "  ] failure ! ";
		return "";
	}
	if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(reply);
		LOG(ERROR) << "Execute command [ HGet " << key << " " << hkey << "  ] failure ! ";
		return "";
	}

	std::string value = reply->str;
	freeReplyObject(reply);
	LOG(INFO) << "Execute command [ HGet " << key << " " << hkey << " ] success ! ";
	return value;
}

bool RedisManager::HSet(const std::string& key, const std::string& hkey, const std::string& value)
{
	auto connect = _pool->getConnection();
	if (connect == nullptr) {
		return false;
	}
	Defer defer([this, &connect] { _pool->returnConnection(connect); });
	auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (reply == nullptr) {
		LOG(ERROR) << "reply is nullptr";
		LOG(ERROR) << "Execute command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! ";
		return false;
	}
	if (reply->type != REDIS_REPLY_INTEGER) {
		LOG(ERROR) << "Execute command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! ";
		freeReplyObject(reply);
		return false;
	}
	LOG(INFO) << "Execute command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! ";
	freeReplyObject(reply);
	return true;
}

bool RedisManager::HDel(const std::string& key, const std::string& field)
{
	auto connect = _pool->getConnection();
	if (connect == nullptr) {
        return false;
    }

	Defer defer([this, &connect] { _pool->returnConnection(connect); });

	redisReply* reply = (redisReply*)redisCommand(connect, "HDEL %s %s", key.c_str(), field.c_str());
    if (reply == nullptr) {
		LOG(ERROR) << "Execute command [ HDel " << key << "  " << field << " ] failure ! ";
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
	Defer defer([this, &connect] { _pool->returnConnection(connect); });
	auto reply = (redisReply*)redisCommand(connect, "AUTH %s", password.c_str());
	if (reply->type == REDIS_REPLY_ERROR) {
		LOG(ERROR) << "Redis verify failed!!!";
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

	Defer defer([this, &connect]() { _pool->returnConnection(connect); });

	return DistributeLock::GetInstance()->acquireLock(connect, lockName, lockTimeOut, acquireTimeOut);
}

bool RedisManager::releaseLock(const std::string& lockName, const std::string& identifier)
{
	if (identifier.empty())	return true;
	auto conn = _pool->getConnection();
	if (!conn)	return false;
	Defer defer([this, &conn]() { _pool->returnConnection(conn); });
	return DistributeLock::GetInstance()->releaseLock(conn, lockName, identifier);
}

void RedisManager::IncreaseCount(std::string server_name)
{
	auto lock_key = LOCK_COUNT;
	auto identifier = acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	Defer defer([this, identifier, lock_key]() { releaseLock(lock_key, identifier); });

	// 增加当前服务器的登录人数
	auto res = HGet(LOGINCOUNT, server_name);
	int count = 0;
	if (!res.empty()) {
        count = std::stoi(res);
    }
	++count;
	auto count_str = std::to_string(count);
    HSet(LOGINCOUNT, server_name, count_str);
}

void RedisManager::DecreaseCount(std::string server_name)
{
	auto lock_key = LOCK_COUNT;
	auto identifier = acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	Defer defer([this, identifier, lock_key]() { releaseLock(lock_key, identifier); });

	// 减少当前服务器的登录人数
	auto res = HGet(LOGINCOUNT, server_name);
	int count = 0;
	if (!res.empty()) {
		count = std::stoi(res);
		if (count > 0) --count;
	}
	auto count_str = std::to_string(count);
    HSet(LOGINCOUNT, server_name, count_str);
}

void RedisManager::InitCount(std::string server_name)
{
	auto lock_key = LOCK_COUNT;
    auto identifier = acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
    Defer defer([this, identifier, lock_key]() { releaseLock(lock_key, identifier); });
    HSet(LOGINCOUNT, server_name, "0");
}

void RedisManager::DelCount(std::string server_name)
{
    auto lock_key = LOCK_COUNT;
    auto identifier = acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
    Defer defer([this, identifier, lock_key]() { releaseLock(lock_key, identifier); });
    HDel(LOGINCOUNT, server_name);
}

RedisManager::RedisManager()
{
	auto& cfgMgr = ConfigManager::GetInstance();
	auto host = cfgMgr["Redis"]["Host"];
	auto port = cfgMgr["Redis"]["Port"];
	auto pwd = cfgMgr["Redis"]["Passwd"];
	_pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
