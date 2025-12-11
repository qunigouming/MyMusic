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
#include <vector>
#include <variant>

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
				std::unique_ptr<sql::Statement> stmt(conn->createStatement());
				stmt->execute("SET NAMES 'utf8mb4'");
				stmt->execute("SET CHARACTER SET utf8mb4");
				stmt->execute("SET character_set_connection=utf8mb4");
				// 获取当前时间
				auto current_time = std::chrono::system_clock::now().time_since_epoch();
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(current_time).count();
				_pool.push(std::make_unique<SqlConnection>(conn, timestamp));
				LOG(INFO) << "Mysql Connection Success";
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
			LOG(ERROR) << "Error: " << e.what();
			LOG(ERROR) << "SQLState: " << e.getSQLState();
			LOG(ERROR) << "VendorError: " << e.getErrorCode();
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
					LOG(INFO) << "Error keeping connection alive: " << e.what();
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
			LOG(INFO) << "connect mysql success";
			return true;
		}
		catch (sql::SQLException& e) {
			LOG(ERROR) << "Reconnect failed, Error: " << e.what();
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
	using Params = std::vector<std::variant<std::string, int>>;
public:
	MySqlDao();
	~MySqlDao();

	bool GetAllMusicInfo(int user_id, MusicInfoListPtr& music_list_info);
	std::shared_ptr<UserInfo> GetUserInfo(const int& uid);
	std::shared_ptr<UserInfo> GetUserInfo(const std::string& name);

	// 获取或创建歌手
	int getOrCreateArtist(const std::string& artist_name);

	// 获取或创建专辑
	int getOrCreateAlbum(const Album& album);

	// 创建专辑和歌手关联
	void createAlbumArtistIfNotExists(int album_id, int artist_id);

	// 获取或创建歌曲
	int getOrCreateSong(const Song& song);

	// 创建歌曲和歌手关联
	void createSongArtistIfNotExists(int song_id, int artist_id);

	// 获取或创建歌单
	int getOrCreatePlaylist(const Playlist& playlist);

	// 创建歌单歌曲关联
	void createPlaylistSong(const PlaylistSong& ps);

	std::string getCoverUrl(int song_id);
	int getCoverUrlId(int song_id);

	std::string getSongTitle(int song_id);

	// 删除歌单中的歌曲
	bool deletePlaylistSong(int playlist_id, int song_id);

	// 更新歌单歌曲位置
	bool updatePlaylistSongPosition(int playlist_id);

	int getPlaylistId(int user_id, const std::string& playlist_name);

	std::shared_ptr<SongListPageInfo> getSongListPageInfo(int playlist_id);

	MusicInfoListPtr getPlaylistSongs(int playlist_id, int user_id);

	int createFileMap(FileMapInfo file_info);		// 创建文件映射 返回记录ID

private:
	// 通用ID查询
	int getIDFromTable(const std::string& table, const std::string& column, const std::string& value);

	// 检查记录是否存在
	bool recordExists(const std::string& table, const std::string& column, const std::string& value);

	// 插入记录并返回ID
	int insertRecord(const std::string& sql, const Params& params);

private:
	std::unique_ptr<MySqlPool> _pool;
};

