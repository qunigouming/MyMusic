#pragma once
#include <string>
#include <vector>

struct UserInfo {
	int uid;
	int sex;
	std::string name;
	std::string email;
	std::string icon;
	std::string back;
};

struct MusicInfo {
	int id;
	std::string title;
	std::string album;
	std::string song_icon;
	std::string artists;
	std::string album_artists;
	std::string file_url;
	int duration = 0;
	bool is_like = false;
};

// 数据库中存储的歌曲信息

struct Artist {
	std::string name;
};

struct Album {
	std::string title;
	std::string artist_name;
	std::vector<std::string> artist_names;
	std::string release_date;
	int cover_url_id;
	std::string description;
};

struct Song {
	std::string title;
	std::string album_title;
	int duration;
	int track_number;
	std::string file_url;
	std::vector<std::string> artist_names;

	void Clear() {
		title.clear();
		album_title.clear();
		file_url.clear();
		artist_names.clear();
        duration = 0;
        track_number = 0;
	}
};

// 歌单
struct Playlist {
	int user_id;
	std::string name;
	std::string description;
	int cover_url_id;
	bool is_default;
};

// 歌单歌曲
struct PlaylistSong {
	std::string playlist_name;
	int user_id;
	int song_id;
	std::string album_title;
	int position;
};

// 该结构体专门用于描述歌单页面信息
struct SongListPageInfo
{
	std::string title;
	std::string songlist_icon;
	std::string description;		// 歌单描述
	std::string author;
	std::string authorIcon;
	std::string createTime;
	int songNumber = 0;			// 歌曲数量
	int songCount = 0;			// 听过的次数
	bool isModify = false;		// 歌单信息是否可以修改
	bool isCollect = false;		// 歌单是否可以被收藏
};

// 该结构体描述文件映射信息
struct FileMapInfo
{
	std::string storage_path;
	std::string mime_type;
	unsigned int create_id;
};