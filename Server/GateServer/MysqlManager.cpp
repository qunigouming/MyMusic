#include "MysqlManager.h"

int MysqlManager::RegUser(const std::string& name, const std::string& passwd, const std::string& email)
{
	int result = _dao.RegUser(name, passwd, email);
	if (result == 0)		return ErrorCodes::Success;
	else if (result == -2)	return ErrorCodes::UserExist;
	else if (result == -3)	return ErrorCodes::EmailExist;
	else					return ErrorCodes::OtherError;
}

bool MysqlManager::LoginValid(const std::string& name, const std::string& passwd, int& id)
{
	return _dao.LoginValid(name, passwd, id);
}
