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
