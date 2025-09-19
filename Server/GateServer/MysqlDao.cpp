#include "MysqlDao.h"
#include "ConfigManager.h"
#include "Encrypt/Encrypt.h"

MysqlDao::MysqlDao()
{
	auto& cfgMgr = ConfigManager::GetInstance();
	const std::string& host = cfgMgr["Mysql"]["Host"];
	const std::string& port = cfgMgr["Mysql"]["Port"];
	const std::string& user = cfgMgr["Mysql"]["User"];
	const std::string& passwd = cfgMgr["Mysql"]["Passwd"];
	const std::string& schema = cfgMgr["Mysql"]["Schema"];
	_pool.reset(new MySqlConPool(host + ":" + port, user, passwd, schema, 5));
}

MysqlDao::~MysqlDao()
{
	_pool->Close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& passwd_hash, const std::string& passwd_salt, const std::string& email)
{
	auto con = _pool->getConnection();
	//Defer defer([this, &con] {
	//	_pool->returnConnection(std::move(con));
	//});
	try {
		if (!con)	return false;
		std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("call reg_user(?, ?, ?, ?, @result)"));
		std::cout << "user name is: " << name << std::endl;
		stmt->setString(1, name);
		stmt->setString(2, email);
		stmt->setString(3, passwd_salt);
		stmt->setString(4, passwd_hash);
		stmt->execute();
		std::unique_ptr<sql::Statement> state(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(state->executeQuery("select @result as result"));
		if (res->next()) {
			int result = res->getInt("result");
			std::cout << "Result: " << result << std::endl;
			_pool->returnConnection(std::move(con));
			return result;
		}
		_pool->returnConnection(std::move(con));
		return -1;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << "(MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		_pool->returnConnection(std::move(con));
		return -1;
	}
}

bool MysqlDao::LoginValid(const std::string& name, const std::string& passwd_hash, int& id)
{
	auto con = _pool->getConnection();
	if (!con) return false;
	Defer defer([this, &con] {
		_pool->returnConnection(std::move(con));
	});
	try {
		std::unique_ptr<sql::PreparedStatement> stmp(con->_con->prepareStatement("select id, password_hash from user where name = ?"));
		stmp->setString(1, name);

		std::unique_ptr<sql::ResultSet> res(stmp->executeQuery());
		std::string password_hash = "";
		if (res->next()) {
            password_hash = res->getString("password_hash");
			//std::cout << "Password is: " << password_hash << std::endl;
		}
		if (password_hash.empty())	return false;
		// 解密
		if (!Encrypt::VerifyHashes(password_hash, passwd_hash)) return false;
		id = res->getInt("id");
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << "(MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::GetPasswdSalt(const std::string& name, std::string& salt)
{
	auto conn = _pool->getConnection();
    if (!conn) return false;
    Defer defer([this, &conn] { _pool->returnConnection(std::move(conn)); });
	try {
		std::unique_ptr<sql::PreparedStatement> stmp(conn->_con->prepareStatement("select password_salt from user where name = ?"));
        stmp->setString(1, name);
        std::unique_ptr<sql::ResultSet> res(stmp->executeQuery());
        if (res->next()) {
            salt = res->getString("password_salt");
            return true;
        }
        return false;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << "(MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}
