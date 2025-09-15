#include "MysqlManager.h"

bool MysqlManager::GetAllMusicInfo(MusicInfoListPtr& music_list_info)
{
    return _dao.GetAllMusicInfo(music_list_info);
}

std::shared_ptr<UserInfo> MysqlManager::GetUserInfo(const int& uid)
{
    return _dao.GetUserInfo(uid);
}

std::shared_ptr<UserInfo> MysqlManager::GetUserInfo(const std::string& name)
{
    return _dao.GetUserInfo(name);
}

int MysqlManager::getUserInfo()
{
    return _dao.getUserInfo();
}

int MysqlManager::getOrCreateArtist(const std::string& artist_name)
{
    return _dao.getOrCreateArtist(artist_name);
}

int MysqlManager::getOrCreateAlbum(const Album& album)
{
    return _dao.getOrCreateAlbum(album);
}

int MysqlManager::getOrCreateSong(const Song& song)
{
    return _dao.getOrCreateSong(song);
}

void MysqlManager::createSongArtistIfNotExists(int song_id, int artist_id)
{
    _dao.createSongArtistIfNotExists(song_id, artist_id);
}

int MysqlManager::getOrCreatePlaylist(const Playlist& playlist)
{
    return _dao.getOrCreatePlaylist(playlist);
}

void MysqlManager::createPlaylistSong(const PlaylistSong& ps)
{
    _dao.createPlaylistSong(ps);
}
