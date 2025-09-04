#include "MysqlDao.h"
#include "ConfigManager.h"

MySqlDao::MySqlDao()
{
	auto& cfg = ConfigManager::GetInstance();
	const auto& host = cfg["Mysql"]["Host"];
	const auto& port = cfg["Mysql"]["Port"];
	const auto& pwd = cfg["Mysql"]["Passwd"];
	const auto& schema = cfg["Mysql"]["Schema"];
	const auto& user = cfg["Mysql"]["User"];
	_pool.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}

MySqlDao::~MySqlDao()
{
	_pool->Close();
}

bool MySqlDao::GetAllMusicInfo(MusicInfoListPtr& music_list_info)
{
	auto conn = _pool->GetConnection();
	if (!conn) return false;
	Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });
	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement(R"(
		SELECT 
			s.title AS song_title,
			al.title AS album_title,
			s.duration,
			al.cover_url AS song_icon,
			GROUP_CONCAT(DISTINCT ar.name SEPARATOR ', ') AS artist_names,
			s.file_url
		FROM songs s
		JOIN albums al ON s.album_id = al.id
		LEFT JOIN song_artists sa ON s.id = sa.song_id
		LEFT JOIN artists ar ON sa.artist_id = ar.id
		GROUP BY s.id;)"));

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
			auto music = std::make_shared<MusicInfo>();
			music->title = res->getString("song_title");
			music->album = res->getString("album_title");
            music->artists = res->getString("artist_names");
            music->song_icon = res->getString("song_icon");
            music->file_url = res->getString("file_url");
            music->duration = res->getInt("duration");

			music_list_info.push_back(music);
        }
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

std::shared_ptr<UserInfo> MySqlDao::GetUserInfo(const int& uid)
{
	auto conn = _pool->GetConnection();
	if (!conn)	return nullptr;
	Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });
	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement("select * from user where id = ?"));
		pstmt->setInt(1, uid);

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::unique_ptr<UserInfo> user_ptr = nullptr;
		while (res->next()) {
			user_ptr.reset(new UserInfo);
			user_ptr->uid = uid;
			user_ptr->name = res->getString("name");
			user_ptr->pwd = res->getString("pwd");
			user_ptr->email = res->getString("email");
			user_ptr->sex = res->getInt("sex");
			user_ptr->icon = res->getString("icon");
			break;
		}
		return user_ptr;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return nullptr;
	}
}

std::shared_ptr<UserInfo> MySqlDao::GetUserInfo(const std::string& name)
{
	auto conn = _pool->GetConnection();
	if (!conn)	return nullptr;
	Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });
	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement("select * from user where name = ?"));
		pstmt->setString(1, name);

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::unique_ptr<UserInfo> user_ptr = nullptr;
		while (res->next()) {
			user_ptr.reset(new UserInfo);
			user_ptr->uid = res->getInt("uid");
			user_ptr->name = res->getString("name");
			user_ptr->pwd = res->getString("pwd");
			user_ptr->email = res->getString("email");
			user_ptr->sex = res->getInt("sex");
			user_ptr->icon = res->getString("icon");
			break;
		}
		return user_ptr;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return nullptr;
	}
}
