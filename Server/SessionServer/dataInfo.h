#pragma once
#include <string>

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