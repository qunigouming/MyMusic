#include "MysqlDao.h"

MysqlDao::MysqlDao() : _db(QSqlDatabase::addDatabase("QMYSQL"))
{
    _db.setHostName("localhost");
    _db.setDatabaseName("mymusic");
    _db.setUserName("root");
    _db.setPassword("123");
    if (!_db.open()) {
        handleSQLError("Database connection failed");
    }

    _db.transaction();
}

MysqlDao::~MysqlDao()
{
    if (_db.isOpen()) {
        _db.commit();
        _db.close();
    }
}

int MysqlDao::getUserInfo()
{
    return 0;
}

int MysqlDao::getOrCreateArtist(const QString& artist_name)
{
    // 检测歌手是否存在
    int artist_id = getIDFromTable("artist", "name", artist_name);
    if (artist_id != -1) {
        qDebug() << "歌手已存在:" << artist_name << "(ID:" << artist_id << ")";
        return artist_id;
    }

    // 插入新歌手
    QMap<QString, QVariant> values;
    values["name"] = artist_name;

    artist_id = insertRecord("artists", values);

    if (artist_id != -1) {
        qDebug() << "创建歌手:" << artist_name << "(ID:" << artist_id << ")";
    }
    return artist_id;
}

int MysqlDao::getOrCreateAlbum(const Album& album)
{
    // 获取或创建歌手
    int artist_id = getOrCreateArtist(album.artist_name);
    if (artist_id == -1) {
        throw std::runtime_error("无法创建或获取歌手: " + album.artist_name.toStdString());
    }

    // 检查专辑是否已存在
    int album_id = getIDFromTable("albums", "title", album.title);
    if (album_id != -1) {
        // 验证歌手是否匹配
        if (verifyAlbumArtist(album_id, artist_id)) {
            qDebug() << "专辑已存在:" << album.title << "(ID:" << album_id << ")";
            return album_id;
        }
        else {
            throw std::runtime_error(("专辑 '" + album.title + "' 已存在但属于不同的歌手").toStdString());
        }
    }

    // 插入新专辑
    QMap<QString, QVariant> values;
    values["title"] = album.title;
    values["artist_id"] = artist_id;
    values["release_date"] = album.release_date;
    values["cover_url"] = album.cover_url;
    values["description"] = album.description;

    album_id = insertRecord("albums", values);

    if (album_id != -1) {
        qDebug() << "创建专辑:" << album.title << "(ID:" << album_id << ")";
    }
    return album_id;
}

bool MysqlDao::verifyAlbumArtist(int album_id, int artist_id)
{
    QSqlQuery query(_db);
    query.prepare("SELECT artist_id FROM albums WHERE id = ?");
    query.addBindValue(album_id);

    if (!query.exec()) {
        handleSQLError("Album artist verification failed");
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() == artist_id;
    }
    return false;
}

int MysqlDao::getOrCreateSong(const Song& song)
{
    // 获取或创建专辑
    Album album = { song.album_title, "", "", "", "" };
    int album_id = getOrCreateAlbum(album);
    if (album_id == -1) {
        throw std::runtime_error("无法创建或获取专辑: " + song.album_title.toStdString());
    }

    // 检查歌曲是否已存在
    QSqlQuery query(_db);
    query.prepare("SELECT id FROM songs WHERE title = ? AND album_id = ?");
    query.addBindValue(song.title);
    query.addBindValue(album_id);

    if (!query.exec()) {
        handleSQLError("Song existence check failed");
    }
    else if (query.next()) {
        int song_id = query.value(0).toInt();
        qDebug() << "歌曲已存在:" << song.title << "(专辑:" << song.album_title
            << ", ID:" << song_id << ")";
        return song_id;
    }

    // 插入新歌曲
    QMap<QString, QVariant> values;
    values["title"] = song.title;
    values["album_id"] = album_id;
    values["duration"] = song.duration;
    values["track_number"] = song.track_number;
    values["file_url"] = song.file_url;

    int song_id = insertRecord("songs", values);

    if (song_id != -1) {
        qDebug() << "创建歌曲:" << song.title << "(专辑:" << song.album_title
            << ", ID:" << song_id << ")";

        // 添加歌手关联
        for (const auto& artist_name : song.artist_names) {
            int artist_id = getOrCreateArtist(artist_name);
            if (artist_id != -1) {
                createSongArtistIfNotExists(song_id, artist_id);
            }
        }
    }
    return song_id;
}

void MysqlDao::createSongArtistIfNotExists(int song_id, int artist_id)
{
    QSqlQuery checkQuery(_db);
    checkQuery.prepare("SELECT COUNT(*) FROM song_artists WHERE song_id = ? AND artist_id = ?");
    checkQuery.addBindValue(song_id);
    checkQuery.addBindValue(artist_id);

    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "歌曲-歌手关联已存在: song_id=" << song_id
            << ", artist_id=" << artist_id;
        return;
    }

    // 创建新关联
    QMap<QString, QVariant> values;
    values["song_id"] = song_id;
    values["artist_id"] = artist_id;

    if (insertRecord("song_artists", values) != -1) {
        qDebug() << "创建歌曲-歌手关联: song_id=" << song_id
            << ", artist_id=" << artist_id;
    }
}

int MysqlDao::getOrCreatePlaylist(const Playlist& playlist)
{
    // 获取用户ID
    int user_id = getUserInfo();
    if (user_id == -1) {
        return - 1; // throw std::runtime_error("无法创建或获取用户: " + playlist.user_name.toStdString());
    }

    // 检查歌单是否已存在
    QSqlQuery query(_db);
    query.prepare("SELECT id FROM playlists WHERE user_id = ? AND name = ?");
    query.addBindValue(user_id);
    query.addBindValue(playlist.name);

    if (!query.exec()) {
        handleSQLError("Playlist existence check failed");
    }
    else if (query.next()) {
        int playlist_id = query.value(0).toInt();
        qDebug() << "歌单已存在:" << playlist.name << "(用户:" << playlist.user_name
            << ", ID:" << playlist_id << ")";
        return playlist_id;
    }

    // 插入新歌单
    QMap<QString, QVariant> values;
    values["user_id"] = user_id;
    values["name"] = playlist.name;
    values["description"] = playlist.description;
    values["cover_url"] = playlist.cover_url;
    values["is_default"] = playlist.is_default;

    int playlist_id = insertRecord("playlists", values);

    if (playlist_id != -1) {
        qDebug() << "创建歌单:" << playlist.name << "(用户:" << playlist.user_name
            << ", ID:" << playlist_id << ")";
    }
    return playlist_id;
}

void MysqlDao::createPlaylistSong(const PlaylistSong& ps)
{
    // 获取歌单ID
    Playlist playlist = { ps.user_name, ps.playlist_name, "", "", false };
    int playlist_id = getOrCreatePlaylist(playlist);
    if (playlist_id == -1) {
        throw std::runtime_error("无法创建或获取歌单: " + ps.playlist_name.toStdString());
    }

    // 获取歌曲ID
    Song song = { ps.song_title, ps.album_title, 0, 0, "", {} };
    int song_id = getOrCreateSong(song);
    if (song_id == -1) {
        throw std::runtime_error("无法创建或获取歌曲: " + ps.song_title.toStdString());
    }

    // 检查关联是否已存在
    QSqlQuery checkQuery(_db);
    checkQuery.prepare("SELECT COUNT(*) FROM playlist_songs "
        "WHERE playlist_id = ? AND song_id = ?");
    checkQuery.addBindValue(playlist_id);
    checkQuery.addBindValue(song_id);

    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "歌单歌曲关联已存在: playlist_id=" << playlist_id
            << ", song_id=" << song_id;
        return;
    }

    // 获取下一个位置
    int position = ps.position;
    if (position <= 0) {
        QSqlQuery posQuery(_db);
        posQuery.prepare("SELECT MAX(position) FROM playlist_songs WHERE playlist_id = ?");
        posQuery.addBindValue(playlist_id);

        if (posQuery.exec() && posQuery.next() && !posQuery.value(0).isNull()) {
            position = posQuery.value(0).toInt() + 1;
        }
        else {
            position = 1;
        }
    }

    // 创建关联
    QMap<QString, QVariant> values;
    values["playlist_id"] = playlist_id;
    values["song_id"] = song_id;
    values["position"] = position;

    if (insertRecord("playlist_songs", values) != -1) {
        qDebug() << "创建歌单歌曲关联: playlist_id=" << playlist_id
            << ", song_id=" << song_id << ", position=" << position;
    }
}

void MysqlDao::commit()
{
    if (_db.isOpen()) {
        if (!_db.commit()) {
            handleSQLError("Database commit failed");
        }
    }
}

void MysqlDao::rollback()
{
    if (_db.isOpen()) {
        if (!_db.rollback()) {
            handleSQLError("Database rollback failed");
        }
    }
}

void MysqlDao::handleSQLError(const QString& error)
{
    QSqlError sqlError = _db.lastError();
    qCritical() << "SQL Error [" << error << "]:" << sqlError.text();
    qCritical() << "Database error:" << sqlError.databaseText();
    qCritical() << "Driver error:" << sqlError.driverText();
    rollback();
}

int MysqlDao::getIDFromTable(const QString& table, const QString& column, const QString& value)
{
    QSqlQuery query(_db);
    query.prepare(QString("SELECT id FROM %1 WHERE %2 = ?").arg(table).arg(column));
    query.addBindValue(value);

    if (!query.exec()) {
        handleSQLError("Failed to execute query");
        return -1;
    }
    if (query.next()) {
        return query.value(0).toInt();
    }
    return -1;      // 未找到
}

bool MysqlDao::recordExists(const QString& table, const QString& column, const QString& value)
{
    return getIDFromTable(table, column, value) != -1;
}

int MysqlDao::insertRecord(const QString& table, const QMap<QString, QVariant>& values)
{
    if (values.isEmpty()) return -1;

    QStringList columns = values.keys();
    QStringList placeholders = QStringList(columns.size(), "?");

    QSqlQuery query(_db);
    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(table).arg(columns.join(", ")).arg(placeholders.join(", "));
    query.prepare(sql);

    for (const auto& key : columns) {
        query.addBindValue(values[key]);
    }

    if (!query.exec()) {
        if (query.lastError().nativeErrorCode().toInt() == 1062) {
            return getIDFromTable(table, "name", values["name"].toString());
        }
        handleSQLError("Failed to execute query");
        return -1;
    }

    if (query.lastInsertId().isValid()) {
        return query.lastInsertId().toInt();
    }

    QSqlQuery idQuery(_db);
    if (idQuery.exec("SELECT LAST_INSERT_ID()") && idQuery.next()) {
        return idQuery.value(0).toInt();
    }
    handleSQLError("Failed to get last insert ID");
    return -1;
}
