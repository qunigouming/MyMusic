#pragma once

#include <hiredis.h>
#include <string>
#include "Singleton.h"

class DistributeLock : public Singleton<DistributeLock>
{
	friend class Singleton<DistributeLock>;
public:
	~DistributeLock() = default;
	std::string acquireLock(redisContext* context, const std::string& lockName, int lockTimeOut, int acquireTimeOut);
	bool releaseLock(redisContext* context, const std::string& lockName, const std::string& identifier);
private:
	DistributeLock() = default;
};

