#include "MysqlManager.h"

int MysqlManager::RegUser(const std::string& name, const std::string& passwd_hash, const std::string& passwd_salt, const std::string& email)
{
	int result = _dao.RegUser(name, passwd_hash, passwd_salt, email);
	if (result == 0)		return ErrorCodes::Success;
	else if (result == -2)	return ErrorCodes::UserExist;
	else if (result == -3)	return ErrorCodes::EmailExist;
	else					return ErrorCodes::OtherError;
}

bool MysqlManager::LoginValid(const std::string& name, const std::string& passwd_hash, int& id)
{
	return _dao.LoginValid(name, passwd_hash, id);
}

bool MysqlManager::GetPasswdSalt(const std::string& name, std::string& salt)
{
	return _dao.GetPasswdSalt(name, salt);
}
