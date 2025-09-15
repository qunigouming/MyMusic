#pragma once
#include <string>
#include <vector>

struct UserInfo {
	int uid;
	int sex;
	std::string name;
	std::string pwd;
	std::string email;
	std::string icon;
	std::string back;
};

struct MusicInfo {
	std::string title;
	std::string album;
	std::string song_icon;
	std::string artists;
	std::string file_url;
	int duration = 0;
	bool is_like = false;
};

struct Artist {
	std::string name;
};

struct Album {
	std::string title;
	std::string artist_name;
	std::string release_date;
	std::string cover_url;
	std::string description;
};

struct Song {
	std::string title;
	std::string album_title;
	int duration;
	int track_number;
	std::string file_url;
	std::vector<std::string> artist_names;
};

// 歌单
struct Playlist {
	std::string user_name;
	std::string name;
	std::string description;
	std::string cover_url;
	bool is_default;
};

struct PlaylistSong {
	std::string playlist_name;
	std::string user_name;
	std::string song_title;
	std::string album_title;
	int position;
};