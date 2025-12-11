#include "MysqlDao.h"
#include "ConfigManager.h"
#include "LogManager.h"

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

bool MySqlDao::GetAllMusicInfo(int user_id, MusicInfoListPtr& music_list_info)
{
	auto conn = _pool->GetConnection();
	if (!conn) return false;
	Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });
	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement(R"(
        SELECT
            s.id AS song_id,
            s.title AS song_title,
            al.title AS album_title,
            s.duration,
            fm.storage_path AS song_icon,
            GROUP_CONCAT(DISTINCT ar.name SEPARATOR '/ ') AS artist_names,
            s.file_url,
            GROUP_CONCAT(DISTINCT album_ar.name SEPARATOR '/ ') AS album_artists,
            CASE WHEN ps.song_id IS NOT NULL THEN 1 ELSE 0 END AS is_like
        FROM songs s
        JOIN albums al ON s.album_id = al.id
		LEFT JOIN file_map fm ON al.cover_url_id = fm.id
        LEFT JOIN song_artists sa ON s.id = sa.song_id
        LEFT JOIN artists ar ON sa.artist_id = ar.id
        LEFT JOIN album_artists aa ON al.id = aa.album_id
        LEFT JOIN artists album_ar ON aa.artist_id = album_ar.id
        LEFT JOIN (
            SELECT song_id 
            FROM playlist_songs 
            WHERE playlist_id = (SELECT id FROM playlists WHERE user_id = ? AND is_default = 1)
        ) ps ON s.id = ps.song_id
        GROUP BY s.id, fm.storage_path;)"));

		pstmt->setInt(1, user_id);  // 设置用户ID参数

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		while (res->next()) {
			auto music = std::make_shared<MusicInfo>();
			music->id = res->getInt("song_id");
			music->title = res->getString("song_title");
			music->album = res->getString("album_title");
			music->artists = res->getString("artist_names");
			music->album_artists = res->getString("album_artists");
			music->song_icon = res->getString("song_icon");
			music->file_url = res->getString("file_url");
			music->duration = res->getInt("duration");
			music->is_like = res->getInt("is_like");  // 设置收藏状态

			music_list_info.push_back(music);
		}
		return true;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
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
			user_ptr->email = res->getString("email");
			user_ptr->sex = res->getInt("sex");
			user_ptr->icon = res->getString("icon");
			break;
		}
		return user_ptr;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
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
			user_ptr->email = res->getString("email");
			user_ptr->sex = res->getInt("sex");
			user_ptr->icon = res->getString("icon");
			break;
		}
		return user_ptr;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return nullptr;
	}
}

int MySqlDao::getOrCreateArtist(const std::string& artist_name)
{
	// 检查歌手是否已存在
	int artist_id = getIDFromTable("artists", "name", artist_name);
	if (artist_id != -1) {
		LOG(INFO) << "歌手已存在: " << artist_name << " (ID: " << artist_id << ")";
		return artist_id;
	}

	// 插入新歌手
	Params params = { artist_name };
	artist_id = insertRecord(
		"INSERT INTO artists (name) VALUES (?)",
		params
	);

	if (artist_id != -1) {
		LOG(INFO) << "创建歌手: " << artist_name << " (ID: " << artist_id << ")";
	}
	return artist_id;
}

int MySqlDao::getOrCreateAlbum(const Album& album)
{
	// 检查专辑是否已存在
	int album_id = getIDFromTable("albums", "title", album.title);
	if (album_id != -1) {
		LOG(INFO) << "专辑已存在: " << album.title << " (ID: " << album_id << ")";
		return album_id;
	}

	// 插入新专辑
	Params params = {
		album.title,
		album.release_date,
		album.cover_url_id,
		album.description
	};

	album_id = insertRecord(
		"INSERT INTO albums (title, release_date, cover_url_id, description) "
		"VALUES (?, ?, ?, ?)",
		params
	);

	if (album_id != -1) {
		LOG(INFO) << "创建专辑: " << album.title << " (ID: " << album_id << ")";

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
			LOG(INFO) << "专辑-艺术家关联已存在: album_id=" << album_id
				<< ", artist_id=" << artist_id;
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

		LOG(INFO) << "创建专辑-艺术家关联: album_id=" << album_id
			<< ", artist_id=" << artist_id;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
	}
}

int MySqlDao::getOrCreateSong(const Song& song)
{
	// 获取或创建专辑
	int album_id = getOrCreateAlbum({ song.album_title, "", std::vector<std::string>(), "", 1, ""});		// 防止专辑未创建(正常情况只会获取id)
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
			LOG(INFO) << "歌曲已存在: " << song.title << " (专辑: " << song.album_title
				<< ", ID: " << song_id << ")";
			return song_id;
		}
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
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
			LOG(INFO) << "创建歌曲: " << song.title << " (专辑: " << song.album_title
				<< ", ID: " << song_id << ")";
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
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
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
			LOG(INFO) << "歌曲-歌手关联已存在: song_id=" << song_id
				<< ", artist_id=" << artist_id;
			return;
		}

		// 创建关联
		std::unique_ptr<sql::PreparedStatement> stmt(
			conn->_conn->prepareStatement("insert into song_artists(song_id, artist_id) values(?,?)"));
        stmt->setInt(1, song_id);
        stmt->setInt(2, artist_id);
        stmt->executeUpdate();

        LOG(INFO) << "歌曲-歌手关联已创建: song_id=" << song_id
			<< ", artist_id=" << artist_id;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
	}
}

int MySqlDao::getOrCreatePlaylist(const Playlist& playlist)
{
	// 检查歌单是否已存在
	auto conn = _pool->GetConnection();
	if (!conn)	return -1;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };
	try {
		std::unique_ptr<sql::PreparedStatement> ps(conn->_conn->prepareStatement("SELECT id FROM playlists WHERE user_id = ? AND name = ?"));
        ps->setInt(1, playlist.user_id);
        ps->setString(2, playlist.name);
        std::unique_ptr<sql::ResultSet> res(ps->executeQuery());
        if (res->next()) {
			int playlist_id = res->getInt(1);
			LOG(INFO) << "歌单已存在: " << playlist.name << " (用户: " << playlist.user_id
				<< ", ID: " << playlist_id << ")";
			return playlist_id;
		}
		return -1;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
	}

	// 插入新歌单
	try {
		Params params = { playlist.user_id, playlist.name, playlist.description, playlist.cover_url_id, playlist.is_default };
		int playlist_id = insertRecord(R"(INSERT INTO playlists
					(user_id, name, description, cover_url_id, is_default)
					VALUES (?, ?, ?, ?, ?))", params);
        LOG(INFO) << "创建歌单: " << playlist.name << " (用户: " << playlist.user_id
			<< ", ID: " << playlist_id << ")";
		return playlist_id;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
	}
}

void MySqlDao::createPlaylistSong(const PlaylistSong& ps)
{
	// 获取歌单ID
	int playlist_id = getOrCreatePlaylist({ ps.user_id, ps.playlist_name, "", 1, false });
	if (playlist_id == -1) {
		throw std::runtime_error("无法创建或获取歌单: " + ps.playlist_name);
	}

	// 获取歌曲ID
	int song_id = ps.song_id;

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
			LOG(INFO) << "歌单歌曲关联已存在: playlist_id=" << playlist_id
				<< ", song_id=" << song_id;
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

		LOG(INFO) << "创建歌单歌曲关联: playlist_id=" << playlist_id
			<< ", song_id=" << song_id << ", position=" << position;
	}
	catch (sql::SQLException& e) {
		// 忽略重复键错误
		if (e.getErrorCode() != 1062) {
			LOG(ERROR) << "SQLException: " << e.what();
			LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
			LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		}
	}
}

std::string MySqlDao::getCoverUrl(int song_id)
{
	auto conn = _pool->GetConnection();
	if (!conn)	return "";
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };
	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			conn->_conn->prepareStatement(
				R"(SELECT
						fm.storage_path AS cover_url
				   FROM songs s
				   INNER JOIN albums a ON s.album_id = a.id
				   LEFT JOIN file_map fm ON a.cover_url_id = fm.id
				   WHERE s.id = ?)"
			)
		);

		// 设置参数
		pstmt->setInt(1, song_id);

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		// 处理结果
		if (res->next()) {
			return res->getString("cover_url");
		}
		else {
			LOG(ERROR) << "No song found with ID: " << song_id;
			return "";
		}
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return "";
	}
}

int MySqlDao::getCoverUrlId(int song_id)
{
	auto conn = _pool->GetConnection();
    if (!conn)	return -1;
    Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
			conn->_conn->prepareStatement(
				R"(SELECT a.cover_url_id AS cover_url_id
				   FROM songs s
				   LEFT JOIN albums a ON s.album_id = a.id
				   WHERE id = ?)"
			)
		);
        pstmt->setInt(1, song_id);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
			return res->getInt("cover_url_id");
		}
        return -1;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return -1;
	}
}

std::string MySqlDao::getSongTitle(int song_id)
{
	auto conn = _pool->GetConnection();
	if (!conn)	return "";
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };
	try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->_conn->prepareStatement(
                "SELECT title FROM songs WHERE id = ?"
            )
        );
        pstmt->setInt(1, song_id);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) { 
			return res->getString("title");
        } else {
            LOG(ERROR) << "No song found with ID: " << song_id;
            return "";
        }
	}
    catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return "";
	}
}

bool MySqlDao::deletePlaylistSong(int playlist_id, int song_id)
{
	auto conn = _pool->GetConnection();
	if (!conn) return false;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };
    try { 
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->_conn->prepareStatement(
                "DELETE FROM playlist_songs WHERE playlist_id = ? AND song_id = ?"
            )
        );
        pstmt->setInt(1, playlist_id);
        pstmt->setInt(2, song_id);

        int result = pstmt->executeUpdate();
        if (result > 0) {
            LOG(INFO) << "删除成功: playlist_id=" << playlist_id
				<< ", song_id=" << song_id;
			return true;
        }
        else {
            LOG(INFO) << "删除失败: playlist_id=" << playlist_id
				<< ", song_id=" << song_id;
			return false;
        }
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
        return false;
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
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return -1;
	}	
}

bool MySqlDao::recordExists(const std::string& table, const std::string& column, const std::string& value)
{
	return getIDFromTable(table, column, value) != -1;
}

int MySqlDao::insertRecord(const std::string& sql, const Params& params)
{
	auto conn = _pool->GetConnection();
	if (!conn) return -1;
	Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });
	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->_conn->prepareStatement(sql));
		for (std::size_t i = 0; i < params.size(); ++i) {
			const auto& param = params[i];
			std::visit([&pstmt, i](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, std::string>)
					pstmt->setString(i + 1, arg);
				else if constexpr (std::is_same_v<T, int>)
					pstmt->setInt(i + 1, arg);
			}, param);
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
		// 重复键错误，返回现有id(暂不处理)
		//if (e.getErrorCode() == 1062) { // ER_DUP_ENTRY
		//	return getIDFromTable(sql.substr(12, sql.find(' ') - 12), "name", params[0]);
		//}
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return -1;
	}
}

bool MySqlDao::updatePlaylistSongPosition(int playlist_id)
{
	auto conn = _pool->GetConnection();
    if (!conn) return false;
    Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });
    try {
		conn->_conn->setAutoCommit(false);

		std::unique_ptr<sql::PreparedStatement> selectStmt(
			conn->_conn->prepareStatement(
				"SELECT song_id, position FROM playlist_songs "
				"WHERE playlist_id = ? ORDER BY position"
			)
		);

        selectStmt->setInt(1, playlist_id);
        std::unique_ptr<sql::ResultSet> res(selectStmt->executeQuery());
		int new_position = 1;
		while (res->next()) {
			int song_id = res->getInt("song_id");
			int position = res->getInt("position");
			if (position != new_position) {
				std::unique_ptr<sql::PreparedStatement> updateStmt(
					conn->_conn->prepareStatement(
						"UPDATE playlist_songs SET position = ? "
						"WHERE playlist_id = ? AND song_id = ?"
					)
				);
				updateStmt->setInt(1, new_position);
				updateStmt->setInt(2, playlist_id);
				updateStmt->setInt(3, song_id);
                updateStmt->executeUpdate();
			}
            new_position++;
        }
        conn->_conn->commit();
		conn->_conn->setAutoCommit(true);
        return true;
    }
	catch (sql::SQLException& e) {
		try {
			conn->_conn->rollback();
            conn->_conn->setAutoCommit(true);
		}
		catch (sql::SQLException& e) {
			LOG(ERROR) << "rollback failed: " << e.what();
		}
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
        return false;
	}
}

int MySqlDao::getPlaylistId(int user_id, const std::string& playlist_name)
{
	auto conn = _pool->GetConnection();
	if (!conn) return -1;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			conn->_conn->prepareStatement(
				"SELECT id FROM playlists WHERE user_id = ? AND name = ?"
			)
		);

		pstmt->setInt(1, user_id);
		pstmt->setString(2, playlist_name);

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		if (res->next()) {
			int playlist_id = res->getInt(1);
			return playlist_id;
		}
		else {
			LOG(INFO) << "歌单不存在: user_id=" << user_id
				<< ", playlist_name=" << playlist_name;
			return -1;
		}
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return -1;
	}
}

std::shared_ptr<SongListPageInfo> MySqlDao::getSongListPageInfo(int playlist_id)
{
	auto conn = _pool->GetConnection();
	if (!conn) return nullptr;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };

	try {
		// SQL查询，获取歌单基本信息以及创建者信息
		std::unique_ptr<sql::PreparedStatement> pstmt(
			conn->_conn->prepareStatement(R"(
                SELECT 
                    p.name AS title,
                    fm.storage_path AS songlist_icon,
                    p.description,
                    p.created_at AS createTime,
                    u.name AS author,
                    u.icon AS authorIcon
                FROM playlists p
                JOIN user u ON p.user_id = u.id
				LEFT JOIN file_map fm ON p.cover_url_id = fm.id
                WHERE p.id = ?
            )")
		);
		pstmt->setInt(1, playlist_id);

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		if (res->next()) {
			auto songListInfo = std::make_shared<SongListPageInfo>();

			// 设置歌单基本信息
			songListInfo->title = res->getString("title");
			songListInfo->songlist_icon = res->getString("songlist_icon");
			songListInfo->description = res->getString("description");
			songListInfo->createTime = res->getString("createTime");

			// 设置作者信息
			songListInfo->author = res->getString("author");
			songListInfo->authorIcon = res->getString("authorIcon");

			LOG(INFO) << "获取歌单页面信息成功: " << playlist_id
				<< ", 歌单名称: " << songListInfo->title
				<< ", 作者: " << songListInfo->author;

			return songListInfo;
		}
		else {
			LOG(INFO) << "未找到歌单ID: " << playlist_id;
			return nullptr;
		}
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException in getSongListPageInfo: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return nullptr;
	}
}

int MySqlDao::createFileMap(FileMapInfo file_info)
{
	auto conn = _pool->GetConnection();
	if (!conn) return -1;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };

	try {
		// 创建文件映射信息
		Params params = { file_info.storage_path, file_info.mime_type, file_info.create_id };
		int file_id = insertRecord("INSERT INTO file_map (storage_path, mime_type, created_by) VALUES (?, ?, ?)", params);

		if (file_id != -1) {
			LOG(INFO) << "创建文件映射信息成功: " << "存储路径: " << file_info.storage_path << ", MIME类型: " << file_info.mime_type << ", 创建者ID: " << file_info.create_id;
		}
		return file_id;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException in getSongListPageInfo: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return -1;
	}
}

MusicInfoListPtr MySqlDao::getPlaylistSongs(int playlist_id, int user_id)
{
	MusicInfoListPtr songList;
	auto conn = _pool->GetConnection();
	if (!conn) return songList;
	Defer defer{ [this, &conn]() { _pool->ReturnConnection(std::move(conn)); } };

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(
			conn->_conn->prepareStatement(R"(
                SELECT
                    s.id AS song_id,
                    s.title AS song_title,
                    al.title AS album_title,
                    s.duration,
                    fm.storage_path AS song_icon,
                    GROUP_CONCAT(DISTINCT ar.name SEPARATOR '/ ') AS artist_names,
                    s.file_url,
                    GROUP_CONCAT(DISTINCT album_ar.name SEPARATOR '/ ') AS album_artists,
                    CASE WHEN ps_like.song_id IS NOT NULL THEN 1 ELSE 0 END AS is_like,
                    ps.position
                FROM playlist_songs ps
                JOIN songs s ON ps.song_id = s.id
                JOIN albums al ON s.album_id = al.id
				LEFT JOIN file_map fm ON al.cover_url_id = fm.id
                LEFT JOIN song_artists sa ON s.id = sa.song_id
                LEFT JOIN artists ar ON sa.artist_id = ar.id
                LEFT JOIN album_artists aa ON al.id = aa.album_id
                LEFT JOIN artists album_ar ON aa.artist_id = album_ar.id
                LEFT JOIN (
                    SELECT song_id 
                    FROM playlist_songs 
                    WHERE playlist_id = (SELECT id FROM playlists WHERE user_id = ? AND is_default = 1)
                ) ps_like ON s.id = ps_like.song_id
                WHERE ps.playlist_id = ?
                GROUP BY s.id, ps.position
                ORDER BY ps.position ASC
            )")
		);
		pstmt->setInt(1, user_id);
		pstmt->setInt(2, playlist_id);

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		while (res->next()) {
			auto music = std::make_shared<MusicInfo>();
			music->id = res->getInt("song_id");
			music->title = res->getString("song_title");
			music->album = res->getString("album_title");
			music->artists = res->getString("artist_names");
			music->album_artists = res->getString("album_artists");
			music->song_icon = res->getString("song_icon");
			music->file_url = res->getString("file_url");
			music->duration = res->getInt("duration");
			music->is_like = res->getInt("is_like");

			songList.push_back(music);
		}

		LOG(INFO) << "获取歌单歌曲列表成功: " << playlist_id
			<< ", 歌曲数量: " << songList.size();

		return songList;
	}
	catch (sql::SQLException& e) {
		LOG(ERROR) << "SQLException in getPlaylistSongs: " << e.what();
		LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
		LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";
		return songList;
	}
}
