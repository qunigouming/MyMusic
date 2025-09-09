#include "MysqlDao.h"
#include "ConfigManager.h"

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

int MysqlDao::RegUser(const std::string& name, const std::string& passwd, const std::string& email)
{
	auto con = _pool->getConnection();
	//Defer defer([this, &con] {
	//	_pool->returnConnection(std::move(con));
	//});
	try {
		if (!con)	return false;
		std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("call reg_user(?, ?, ?, @result)"));
		std::cout << "user name is: " << name << std::endl;
		stmt->setString(1, name);
		stmt->setString(2, email);
		stmt->setString(3, passwd);
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

bool MysqlDao::LoginValid(const std::string& name, const std::string& passwd, int& id)
{
	auto con = _pool->getConnection();
	if (!con) return false;
	Defer defer([this, &con] {
		_pool->returnConnection(std::move(con));
	});
	try {
		std::unique_ptr<sql::PreparedStatement> stmp(con->_con->prepareStatement("select * from user where name = ?"));
		stmp->setString(1, name);
		std::unique_ptr<sql::ResultSet> res(stmp->executeQuery());
		std::string pwd = "";
		if (res->next()) {
			pwd = res->getString("password");
			std::cout << "Password is: " << pwd << std::endl;
		}
		if (pwd != passwd) return false;
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
