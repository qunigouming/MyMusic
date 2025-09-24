#include "DistributeLock.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "global.h"

static std::string generateUUID() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return to_string(uuid);
}

std::string DistributeLock::acquireLock(redisContext* context, const std::string& lockName, int lockTimeOut, int acquireTimeOut)
{
    std::string identifier = generateUUID();
    std::string lockKey = "lock:" + lockName;
    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(acquireTimeOut);

    while (std::chrono::steady_clock::now() < endTime) {
        redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s NX EX %d", lockKey.c_str(), identifier.c_str(), lockTimeOut);
        if (reply == nullptr) {
            return "";
        }
        Defer defer([&]() { freeReplyObject(reply); });
        if (reply->type == REDIS_REPLY_ERROR) {
            return "";
        }
        if (reply->type == REDIS_REPLY_STATUS) {
            if (std::string(reply->str) == "OK") {
                return identifier;
            }
        }
        
        // 防止忙等待
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return "";
}

bool DistributeLock::releaseLock(redisContext* context, const std::string& lockName, const std::string& identifier)
{
    std::string lockKey = "lock:" + lockName;
    const char* script = R"(if redis.call('get', KEYS[1]) == ARGV[1] then
                                return redis.call('del', KEYS[1])
                            else
                                return 0
                            end)";
    redisReply* reply = (redisReply*)redisCommand(context, "EVAL %s 1 %s %s", script, lockKey.c_str(), identifier.c_str());
    bool success = false;
    if (reply != nullptr) {
        // 成功删除锁
        if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
            success = true;
        }
        freeReplyObject(reply);
    }
    return success;
}
