#pragma once

#include "global.h"
#include <thread>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>
#include "dataInfo.h"
#include <queue>

class SqlConnection
{
public:
	SqlConnection(sql::Connection* conn, int64_t lasttime) : _conn(conn), _last_operate_time(lasttime) {}
	std::unique_ptr<sql::Connection> _conn;
	int64_t _last_operate_time;
};

class MySqlPool
{
public:
	MySqlPool(const std::string& url, const std::string& user, const std::string& pwd, const std::string& schema, int pool_size)
		: _url(url), _user(user), _pwd(pwd), _schema(schema), _pool_size(pool_size)
	{
		try {
			for (int i = 0; i < pool_size; i++) {
				sql::mysql::MySQL_Driver *driver = sql::mysql::get_driver_instance();
				auto* conn = driver->connect(url, user, pwd);
				conn->setSchema(schema);
				// 获取当前时间
				auto current_time = std::chrono::system_clock::now().time_since_epoch();
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(current_time).count();
				_pool.push(std::make_unique<SqlConnection>(conn, timestamp));
				std::cout << "Mysql Connection Success" << std::endl;
			}

			_check_thread = std::thread([this]() {
				int count = 0;
				while (!_b_stop) {
					if (count >= 60) {
						count = 0;
						CheckConnection();
					}
					std::this_thread::sleep_for(std::chrono::seconds(1));
					++count;
				}
			});
			_check_thread.detach();
		}
		catch (sql::SQLException &e) {
			std::cout << "Error: " << e.what() << std::endl;
			std::cout << "SQLState: " << e.getSQLState() << std::endl;
			std::cout << "VendorError: " << e.getErrorCode() << std::endl;
		}
	}
	~MySqlPool() {
		std::unique_lock<std::mutex> lock(_mutex);
		while (!_pool.empty()) _pool.pop();
	}
	void CheckConnection() {
		// 读取当前连接数
		size_t targetCount = 0;
		{
			std::lock_guard<std::mutex> lock(_mutex);
			targetCount = _pool.size();
		}
		size_t processed = 0;			// 已处理数

		auto now = std::chrono::system_clock::now().time_since_epoch();
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(now).count();

		while (processed < targetCount) { 
			std::unique_ptr<SqlConnection> conn;
			{
				std::lock_guard<std::mutex> lock(_mutex);
				if (_pool.empty()) break;
				conn = std::move(_pool.front());
				_pool.pop();
			}

			bool healthy = true;
			if (timestamp - conn->_last_operate_time > 5) {
				try {
					std::unique_ptr<sql::Statement> stmt(conn->_conn->createStatement());
					stmt->executeQuery("select 1");
					conn->_last_operate_time = timestamp;
				}
				catch (sql::SQLException& e) {
					std::cout << "Error keeping connection alive: " << e.what() << std::endl;
					healthy = false;
					++_fail_count;
				}
			}

			if (healthy) {
				std::unique_lock<std::mutex> lock(_mutex);
                _pool.push(std::move(conn));
				_cond.notify_one();
			}

			++processed;
		}

		while (_fail_count > 0) {
			bool b_res = ReConnect(timestamp);
			if (b_res) _fail_count = 0;
			else break;
		}
	}

	//重新连接
	bool ReConnect(long long timestamp) {
		try {
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			auto* conn = driver->connect(_url, _user, _pwd);
			conn->setSchema(_schema);

			auto newCon = std::make_unique<SqlConnection>(conn, timestamp);
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_pool.push(std::move(newCon));
			}
			std::cout << "connect mysql success" << std::endl;
			return true;
		}
		catch (sql::SQLException& e) {
			std::cout << "Reconnect failed, Error: " << e.what() << std::endl;
			return false;
		}
	}

	// 从连接池中获取连接
	std::unique_ptr<SqlConnection> GetConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_b_stop)	return true;
			return !_pool.empty();
		});
		if (_b_stop) return nullptr;
		std::unique_ptr<SqlConnection> conn = std::move(_pool.front());
		_pool.pop();
		return conn;
	}

	// 返回连接给连接池
	void ReturnConnection(std::unique_ptr<SqlConnection> conn) {
		if (_b_stop) return;
		std::unique_lock<std::mutex> lock(_mutex);
		_pool.push(std::move(conn));
		_cond.notify_one();
	}

	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}
private:
	std::string _url;
	std::string _user;
	std::string _pwd;
	std::string _schema;
	int _pool_size = 0;
	std::queue<std::unique_ptr<SqlConnection>> _pool;
	std::mutex _mutex;
	std::condition_variable _cond;
	std::atomic_bool _b_stop = false;
	std::thread _check_thread;
	std::atomic_int _fail_count = 0;
};

using MusicInfoListPtr = std::list<std::shared_ptr<MusicInfo>>;
class MySqlDao
{
public:
	MySqlDao();
	~MySqlDao();

	bool GetAllMusicInfo(MusicInfoListPtr& music_list_info);
	std::shared_ptr<UserInfo> GetUserInfo(const int& uid);
	std::shared_ptr<UserInfo> GetUserInfo(const std::string& name);

private:
	std::unique_ptr<MySqlPool> _pool;
};

