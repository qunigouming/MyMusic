#pragma once

#include <string>
#include <QString>
#include <QVector>

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
    QString name;
};

struct Album {
    QString title;
    QString artist_name;
    QString release_date;
    QString cover_url;
    QString description;
};

struct Song {
    QString title;
    QString album_title;
    int duration;
    int track_number;
    QString file_url;
    QVector<QString> artist_names;
};

// 歌单
struct Playlist {
    QString user_name;
    QString name;
    QString description;
    QString cover_url;
    bool is_default;
};

struct PlaylistSong {
    QString playlist_name;
    QString user_name;
    QString song_title;
    QString album_title;
    int position;
};