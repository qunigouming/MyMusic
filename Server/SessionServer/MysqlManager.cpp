#include "MysqlManager.h"

bool MysqlManager::GetAllMusicInfo(int user_id, MusicInfoListPtr& music_list_info)
{
    return _dao.GetAllMusicInfo(user_id, music_list_info);
}

std::shared_ptr<UserInfo> MysqlManager::GetUserInfo(const int& uid)
{
    return _dao.GetUserInfo(uid);
}

std::shared_ptr<UserInfo> MysqlManager::GetUserInfo(const std::string& name)
{
    return _dao.GetUserInfo(name);
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

std::string MysqlManager::getCoverUrl(int song_id)
{
    return _dao.getCoverUrl(song_id);
}

std::string MysqlManager::getSongTitle(int song_id)
{
    return _dao.getSongTitle(song_id);
}

bool MysqlManager::deletePlaylistSong(int playlist_id, int song_id)
{
    if (_dao.deletePlaylistSong(playlist_id, song_id)) {
        return _dao.updatePlaylistSongPosition(playlist_id);
    }
    return false;
}

bool MysqlManager::deletePlaylistSong(int user_id, const std::string& playlist_name, int song_id)
{
    // 获取歌单id
    int playlist_id = _dao.getPlaylistId(user_id, playlist_name);

    // 删除歌单中的歌曲
    if (playlist_id > 0) {
        if (_dao.deletePlaylistSong(playlist_id, song_id)) {
            return _dao.updatePlaylistSongPosition(playlist_id);
        }
    }
    return false;
}
