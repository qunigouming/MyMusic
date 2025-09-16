#pragma once

#include "hiredis.h"
#include <string>
#include "Singleton.h"
class DistLock : public Singleton<DistLock>
{
    friend class Singleton<DistLock>;
public:
    ~DistLock() = default;
    std::string acquireLock(redisContext* context, const std::string& lockName, int lockTimeout, int acquireTimeout);
    bool releaseLock(redisContext* context, const std::string& lockName, const std::string& identifier);
private:
    DistLock() = default;
};

