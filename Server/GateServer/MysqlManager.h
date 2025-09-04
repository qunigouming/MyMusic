#pragma once
#include "global.h"
#include "MysqlDao.h"
#include "Singleton.h"

class MysqlManager : public Singleton<MysqlManager>
{
	friend class Singleton<MysqlManager>;
public:
	~MysqlManager() = default;
	int RegUser(const std::string& name, const std::string& passwd, const std::string& email);
	bool LoginValid(const std::string& name, const std::string& passwd, int& id);
private:
	MysqlManager() = default;
	MysqlDao _dao;
};

