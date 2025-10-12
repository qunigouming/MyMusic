#pragma once

#include "dataInfo.h"
#include "Singleton.h"
#include <memory>
#include <QObject>
#include <QList>

class UserManager : public QObject, public Singleton<UserManager>, public std::enable_shared_from_this<UserManager>
{
    Q_OBJECT
    friend class Singleton<UserManager>;
public:
    ~UserManager();
    void setUserInfo(std::shared_ptr<UserInfo> userInfo);
    std::shared_ptr<UserInfo> getUserInfo();
    void setMusicList(QList<std::shared_ptr<MusicInfo>> musicList);
    const QList<std::shared_ptr<MusicInfo>> getMusicList() const;

    void setToken(QString token);
    int getUid();
    QString getName();
    QString getIcon();
private:
    UserManager();
    std::shared_ptr<UserInfo> _userInfo;
    QList<std::shared_ptr<MusicInfo>> _musicList;
    std::shared_ptr<SongListPageInfo> _songListPageInfo = nullptr;
    QString _token;
};

