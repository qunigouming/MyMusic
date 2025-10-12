#pragma once

#include "MysqlDao.h"
#include "Singleton.h"
#include <vector>

class MysqlManager : public Singleton<MysqlManager>
{
	friend class Singleton<MysqlManager>;
public:
	~MysqlManager() {}
	bool GetAllMusicInfo(int user_id, MusicInfoListPtr& music_list_info);
	std::shared_ptr<UserInfo> GetUserInfo(const int& uid);
	std::shared_ptr<UserInfo> GetUserInfo(const std::string& name);

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

	// 获取封面url
    std::string getCoverUrl(int song_id);

	// 获取歌曲标题
	std::string getSongTitle(int song_id);

	// 删除歌单中的歌曲
	bool deletePlaylistSong(int playlist_id, int song_id);
	bool deletePlaylistSong(int user_id, const std::string& playlist_name, int song_id);
	
	// 获取歌单信息
	std::shared_ptr<SongListPageInfo> getSongListPageInfo(int user_id, const std::string& playlist_name);
	// 获取歌单歌曲
	MusicInfoListPtr getPlaylistSongs(int user_id, const std::string& playlist_name);
private:
	MysqlManager() {}
	MySqlDao _dao;
};

