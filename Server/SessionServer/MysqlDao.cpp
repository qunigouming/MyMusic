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
			GROUP_CONCAT(DISTINCT ar.name SEPARATOR '/ ') AS artist_names,
			s.file_url,
			GROUP_CONCAT(DISTINCT album_ar.name SEPARATOR '/ ') AS album_artists
		FROM songs s
		JOIN albums al ON s.album_id = al.id
		LEFT JOIN song_artists sa ON s.id = sa.song_id
		LEFT JOIN artists ar ON sa.artist_id = ar.id
		LEFT JOIN album_artists aa ON al.id = aa.album_id
        LEFT JOIN artists album_ar ON aa.artist_id = album_ar.id
		GROUP BY s.id;)"));

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
			auto music = std::make_shared<MusicInfo>();
			music->title = res->getString("song_title");
			music->album = res->getString("album_title");
            music->artists = res->getString("artist_names");
			music->album_artists = res->getString("album_artists");
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
			user_ptr->pwd = res->getString("password");
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

int MySqlDao::getUserInfo()
{
	return 0;
}

int MySqlDao::getOrCreateArtist(const std::string& artist_name)
{
	// 检查歌手是否已存在
	int artist_id = getIDFromTable("artists", "name", artist_name);
	if (artist_id != -1) {
		std::cout << "歌手已存在: " << artist_name << " (ID: " << artist_id << ")" << std::endl;
		return artist_id;
	}

	// 插入新歌手
	std::vector<std::string> params = { artist_name };
	artist_id = insertRecord(
		"INSERT INTO artists (name) VALUES (?)",
		params
	);

	if (artist_id != -1) {
		std::cout << "创建歌手: " << artist_name << " (ID: " << artist_id << ")" << std::endl;
	}
	return artist_id;
}

int MySqlDao::getOrCreateAlbum(const Album& album)
{
	// 检查专辑是否已存在
	int album_id = getIDFromTable("albums", "title", album.title);
	if (album_id != -1) {
		std::cout << "专辑已存在: " << album.title << " (ID: " << album_id << ")" << std::endl;
		return album_id;
	}

	// 插入新专辑
	std::vector<std::string> params = {
		album.title,
		album.release_date,
		album.cover_url,
		album.description
	};

	album_id = insertRecord(
		"INSERT INTO albums (title, release_date, cover_url, description) "
		"VALUES (?, ?, ?, ?)",
		params
	);

	if (album_id != -1) {
		std::cout << "创建专辑: " << album.title << " (ID: " << album_id << ")" << std::endl;

		// 创建artist关联
		if (!album.artist_name.empty()) {
			int artist_id = getOrCreateArtist(album.artist_name);
			if (artist_id != -1) {
				createAlbumArtistIfNotExists(album_id, artist_id);
			}
		}
	}
	return album_id;
}

void MySqlDao::createAlbumArtistIfNotExists(int album_id, int artist_id)
{
	auto conn = _pool->GetConnection();
	if (!conn) return;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };

	try {
		// 检查关联是否已存在
		std::unique_ptr<sql::PreparedStatement> pstmt(
			conn->_conn->prepareStatement(
				"SELECT COUNT(*) FROM album_artists WHERE album_id = ? AND artist_id = ?"
			)
		);
		pstmt->setInt(1, album_id);
		pstmt->setInt(2, artist_id);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		if (res->next() && res->getInt(1) > 0) {
			std::cout << "专辑-艺术家关联已存在: album_id=" << album_id
				<< ", artist_id=" << artist_id << std::endl;
			return;
		}

		// 创建关联
		std::unique_ptr<sql::PreparedStatement> insertStmt(
			conn->_conn->prepareStatement(
				"INSERT INTO album_artists (album_id, artist_id) VALUES (?, ?)"
			)
		);
		insertStmt->setInt(1, album_id);
		insertStmt->setInt(2, artist_id);
		insertStmt->executeUpdate();

		std::cout << "创建专辑-艺术家关联: album_id=" << album_id
			<< ", artist_id=" << artist_id << std::endl;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
}

int MySqlDao::getOrCreateSong(const Song& song)
{
	// 获取或创建专辑
	int album_id = getOrCreateAlbum({ song.album_title, "", std::vector<std::string>(), "", "", ""});
	if (album_id == -1) {
		throw std::runtime_error("无法创建或获取专辑: " + song.album_title);
	}

	auto conn = _pool->GetConnection();
	if (!conn)	return -1;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };
	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement("SELECT id FROM songs WHERE title = ? AND album_id = ?"));
		pstmt->setString(1, song.title);
        pstmt->setInt(2, album_id);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		if (res->next()) {
			int song_id = res->getInt(1);
			std::cout << "歌曲已存在: " << song.title << " (专辑: " << song.album_title
				<< ", ID: " << song_id << ")" << std::endl;
			return song_id;
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}

	try { 
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement(R"( INSERT INTO songs
						(title, album_id, duration, track_number, file_url)
						VALUES (?, ?, ?, ?, ?) )"));
		pstmt->setString(1, song.title);
		pstmt->setInt(2, album_id);
		pstmt->setInt(3, song.duration);
        pstmt->setInt(4, song.track_number);
        pstmt->setString(5, song.file_url);
        pstmt->executeUpdate();

		std::unique_ptr<sql::Statement> stmt(conn->_conn->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT LAST_INSERT_ID()"));
		if (res->next()) {
			int song_id = res->getInt(1);
			std::cout << "创建歌曲: " << song.title << " (专辑: " << song.album_title
				<< ", ID: " << song_id << ")" << std::endl;
			// 添加歌手关联
			for (const auto& artist_name : song.artist_names) {
				int artist_id = getOrCreateArtist(artist_name);
				if (artist_id != -1) {
					createSongArtistIfNotExists(song_id, artist_id);
				}
			}
			return song_id;
		}
		return -1;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}

void MySqlDao::createSongArtistIfNotExists(int song_id, int artist_id)
{
	auto conn = _pool->GetConnection();
	if (!conn)	return;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };
    try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement("SELECT COUNT(*) FROM song_artists WHERE song_id = ? AND artist_id = ?"));
        pstmt->setInt(1, song_id);
        pstmt->setInt(2, artist_id);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		if (res->next() && res->getInt(1) > 0) {
			std::cout << "歌曲-歌手关联已存在: song_id=" << song_id
				<< ", artist_id=" << artist_id << std::endl;
			return;
		}

		// 创建关联
		std::unique_ptr<sql::PreparedStatement> stmt(
			conn->_conn->prepareStatement("insert into song_artists(song_id, artist_id) values(?,?)"));
        stmt->setInt(1, song_id);
        stmt->setInt(2, artist_id);
        stmt->executeUpdate();

        std::cout << "歌曲-歌手关联已创建: song_id=" << song_id
			<< ", artist_id=" << artist_id << std::endl;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
}

int MySqlDao::getOrCreatePlaylist(const Playlist& playlist)
{
	// 获取用户ID
	int user_id = getUserInfo();
	if (user_id == -1) {
		throw std::runtime_error("无法创建或获取用户: " + playlist.user_name);
	}

	// 检查歌单是否已存在
	auto conn = _pool->GetConnection();
	if (!conn)	return -1;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };
	try {
		std::unique_ptr<sql::PreparedStatement> ps(conn->_conn->prepareStatement("SELECT id FROM playlists WHERE user_id = ? AND name = ?"));
        ps->setInt(1, user_id);
        ps->setString(2, playlist.name);
        std::unique_ptr<sql::ResultSet> res(ps->executeQuery());
        if (res->next()) {
			int playlist_id = res->getInt(1);
			std::cout << "歌单已存在: " << playlist.name << " (用户: " << playlist.user_name
				<< ", ID: " << playlist_id << ")" << std::endl;
			return playlist_id;
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}

	// 插入新歌单
	try {
		std::unique_ptr<sql::PreparedStatement> ps(conn->_conn->prepareStatement(R"(INSERT INTO playlists
					(user_id, name, description, cover_url, is_default)
					VALUES (?, ?, ?, ?, ?))"));
        ps->setInt(1, user_id);
        ps->setString(2, playlist.name);
        ps->setString(3, playlist.description);
        ps->setString(4, playlist.cover_url);
        ps->setInt(5, playlist.is_default);
        ps->executeUpdate();

		std::unique_ptr<sql::Statement> stmt(conn->_conn->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT LAST_INSERT_ID()"));
		if (res->next()) {
			int playlist_id = res->getInt(1);
			std::cout << "创建歌单: " << playlist.name << " (用户: " << playlist.user_name
				<< ", ID: " << playlist_id << ")" << std::endl;
			return playlist_id;
		}
		return -1;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
}

void MySqlDao::createPlaylistSong(const PlaylistSong& ps)
{
	// 获取歌单ID
	int playlist_id = getOrCreatePlaylist({ ps.user_name, ps.playlist_name, "", "", false });
	if (playlist_id == -1) {
		throw std::runtime_error("无法创建或获取歌单: " + ps.playlist_name);
	}

	// 获取歌曲ID
	int song_id = getOrCreateSong({ ps.song_title, ps.album_title, 0, 0, "", {} });
	if (song_id == -1) {
		throw std::runtime_error("无法创建或获取歌曲: " + ps.song_title);
	}

	auto conn = _pool->GetConnection();
	if (!conn)	return;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };
	// 创建关联
	try {
		// 检查关联是否已存在
		std::unique_ptr<sql::PreparedStatement> checkStmt(
			conn->_conn->prepareStatement(
				"SELECT COUNT(*) FROM playlist_songs "
				"WHERE playlist_id = ? AND song_id = ?"
			)
		);
		checkStmt->setInt(1, playlist_id);
		checkStmt->setInt(2, song_id);
		std::unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());

		if (res->next() && res->getInt(1) > 0) {
			std::cout << "歌单歌曲关联已存在: playlist_id=" << playlist_id
				<< ", song_id=" << song_id << std::endl;
			return;
		}

		// 获取下一个位置
		int next_position = 1;
		std::unique_ptr<sql::PreparedStatement> posStmt(
			conn->_conn->prepareStatement(
				"SELECT MAX(position) FROM playlist_songs WHERE playlist_id = ?"
			)
		);
		posStmt->setInt(1, playlist_id);
		std::unique_ptr<sql::ResultSet> posRes(posStmt->executeQuery());
		if (posRes->next() && !posRes->isNull(1)) {
			next_position = posRes->getInt(1) + 1;
		}

		// 使用指定位置或自动计算位置
		int position = (ps.position > 0) ? ps.position : next_position;

		// 插入关联
		std::unique_ptr<sql::PreparedStatement> insertStmt(
			conn->_conn->prepareStatement(
				"INSERT INTO playlist_songs (playlist_id, song_id, position) "
				"VALUES (?, ?, ?)"
			)
		);
		insertStmt->setInt(1, playlist_id);
		insertStmt->setInt(2, song_id);
		insertStmt->setInt(3, position);
		insertStmt->executeUpdate();

		std::cout << "创建歌单歌曲关联: playlist_id=" << playlist_id
			<< ", song_id=" << song_id << ", position=" << position << std::endl;
	}
	catch (sql::SQLException& e) {
		// 忽略重复键错误
		if (e.getErrorCode() != 1062) {
			std::cerr << "SQLException: " << e.what();
			std::cerr << " (MySQL error code: " << e.getErrorCode();
			std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		}
	}
}

int MySqlDao::getIDFromTable(const std::string& table, const std::string& column, const std::string& value)
{
	auto conn = _pool->GetConnection();
	if (!conn) {
		return -1;
	}
	Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });

	try {
		std::string sql = "select id from " + table + " where " + column + " = ?";
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement(sql));
        pstmt->setString(1, value);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
            return res->getInt(1);
        }
        return -1;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}	
}

bool MySqlDao::recordExists(const std::string& table, const std::string& column, const std::string& value)
{
	return getIDFromTable(table, column, value) != -1;
}

int MySqlDao::insertRecord(const std::string& sql, const std::vector<std::string>& params)
{
	auto conn = _pool->GetConnection();
	if (!conn) return -1;
	Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });
	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement(sql));
		for (int i = 0; i < params.size(); i++) {
			pstmt->setString(i + 1, params[i]);
		}
		pstmt->executeUpdate();
		std::unique_ptr<sql::Statement> stmt(conn->_conn->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT LAST_INSERT_ID()"));
        if (res->next()) {
            return res->getInt(1);
        }
        return -1;
	}
    catch (sql::SQLException &e) {
		// 重复键错误，返回现有id
		if (e.getErrorCode() == 1062) { // ER_DUP_ENTRY
			return getIDFromTable(sql.substr(12, sql.find(' ') - 12), "name", params[0]);
		}
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}
