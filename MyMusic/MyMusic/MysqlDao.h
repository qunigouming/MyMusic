#pragma once
#include <QSqlDataBase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include "global.h"

class MysqlDao
{
public:
	MysqlDao();

	~MysqlDao();

	int getUserInfo();

	// 获取或创建歌手
	int getOrCreateArtist(const QString& artist_name);

	// 获取或创建专辑
	int getOrCreateAlbum(const Album& album);

	// 获取或创建歌曲
	int getOrCreateSong(const Song& song);

	// 创建歌曲和歌手关联
	void createSongArtistIfNotExists(int song_id, int artist_id);

	// 获取或创建歌单
	int getOrCreatePlaylist(const Playlist& playlist);

	// 创建歌单歌曲关联
	void createPlaylistSong(const PlaylistSong& ps);

private:
	void commit();

	void rollback();

	void handleSQLError(const QString &error);

	// 通用ID查询
	int getIDFromTable(const QString& table, const QString& column, const QString& value);

	// 检查记录是否存在
	bool recordExists(const QString& table, const QString& column, const QString& value);

	// 插入记录并返回ID
	int insertRecord(const QString& table, const QMap<QString, QVariant>& values);

	// 验证专辑歌手匹配
	bool verifyAlbumArtist(int album_id, int artist_id);

private:
	QSqlDatabase _db;
};

