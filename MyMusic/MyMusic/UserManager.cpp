#include "UserManager.h"

UserManager::UserManager() : _userInfo(nullptr)
{
}

UserManager::~UserManager()
{
}

void UserManager::setUserInfo(std::shared_ptr<UserInfo> userInfo)
{
    _userInfo = userInfo;
}

void UserManager::setMusicList(QList<std::shared_ptr<MusicInfo>> musicList)
{
    _musicList = musicList;
}

const QList<std::shared_ptr<MusicInfo>> UserManager::getMusicList() const
{
    return _musicList;
}

void UserManager::setToken(QString token)
{
    _token = token;
}

int UserManager::getUid()
{
    return _userInfo->uid;
}

QString UserManager::getName()
{
    return QString::fromStdString(_userInfo->name);
}

QString UserManager::getIcon()
{
    return QString::fromStdString(_userInfo->icon);
}
