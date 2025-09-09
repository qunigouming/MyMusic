#pragma once
#include "Singleton.h"
class MysqlManager : public Singleton<MysqlManager>
{
	friend class Singleton<MysqlManager>;
public:
	~MysqlManager() {}
};

