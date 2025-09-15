#pragma once

#include "MysqlDao.h"
#include "Singleton.h"
#include <vector>

class MysqlManager : public Singleton<MysqlManager>
{
	friend class Singleton<MysqlManager>;
public:
	~MysqlManager() {}
	bool GetAllMusicInfo(MusicInfoListPtr& music_list_info);
	std::shared_ptr<UserInfo> GetUserInfo(const int& uid);
	std::shared_ptr<UserInfo> GetUserInfo(const std::string& name);

	int getUserInfo();
	// 获取或创建歌手
	int getOrCreateArtist(const std::string& artist_name);

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
	MysqlManager() {}
	MySqlDao _dao;
};

