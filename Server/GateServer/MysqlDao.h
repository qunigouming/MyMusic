#pragma once
#include "global.h"
#include <thread>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>

class MySqlConnection {
public:
	MySqlConnection(sql::Connection* con, int64_t lasttime) : _con(con), _last_opt_time(lasttime) {}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_opt_time = 0;
};

class MySqlConPool {
public:
	MySqlConPool(const std::string& url, const std::string& user, const std::string& passwd, const std::string& schema, std::size_t poolSize)
		: _url(url), _user(user), _passwd(passwd), _schema(schema), _poolSize(poolSize) {
		try {
			for (std::size_t i = 0; i < poolSize; ++i) {
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* con = driver->connect(url, user, passwd);
				con->setSchema(schema);
				auto currentTime = std::chrono::system_clock::now().time_since_epoch();
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
				_pool.push(std::make_unique<MySqlConnection>(con, timestamp));
			}

			_check_thread = std::thread([this] {
				while (!_b_stop) {
					checkConnection();
					std::this_thread::sleep_for(std::chrono::seconds(60));
				}
			});
			_check_thread.detach();
		}
		catch(sql::SQLException& e) {
			std::cout << e.what() << std::endl;
		}
	}

	~MySqlConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		while (!_pool.empty()) _pool.pop();
	}

	void checkConnection() {
		std::lock_guard<std::mutex> lock(_mutex);
		auto currentTime = std::chrono::system_clock::now().time_since_epoch();
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
		for (std::size_t i = 0; i < _poolSize; ++i) {
			auto con = std::move(_pool.front());
			_pool.pop();
			//归还连接
			Defer def([this, &con] {
				_pool.push(std::move(con));
			});
			if (timestamp - con->_last_opt_time < 5) continue;			//操作时间相近不做操作

			try {
				std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
				stmt->executeQuery("select 1");
				con->_last_opt_time = timestamp;
				std::cout << "execute timer alive query, current is " << timestamp << std::endl;
			}
			catch (sql::SQLException& e) {
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* newcon = driver->connect(_url, _user, _passwd);
				newcon->setSchema(_schema);
				con->_con.reset(newcon);
				con->_last_opt_time = timestamp;
				std::cout << "keeping connection alive failed: " << e.what() << std::endl;
			}
		}
	}

	std::unique_ptr<MySqlConnection> getConnection(){
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_b_stop) return true;
			return !_pool.empty();
		});
		if (_b_stop) return nullptr;
		auto con = std::move(_pool.front());
		_pool.pop();
		return con;
	}

	void returnConnection(std::unique_ptr<MySqlConnection> connect) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop)	return;
		_pool.push(std::move(connect));
		_cond.notify_one();
	}

	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}
private:
	std::string _url;
	std::string _user;
	std::string _passwd;
	std::string _schema;
	std::size_t _poolSize = 1;
	std::atomic_bool _b_stop = false;
	std::queue<std::unique_ptr<MySqlConnection>> _pool;
	std::mutex _mutex;
	std::condition_variable _cond;
	std::thread _check_thread;
};

class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& passwd_hash, const std::string& passwd_salt, const std::string& email);
	bool LoginValid(const std::string& name, const std::string& passwd_hash, int& id);
	bool GetPasswdSalt(const std::string& name, std::string& salt);
private:
	std::unique_ptr<MySqlConPool> _pool;
};

