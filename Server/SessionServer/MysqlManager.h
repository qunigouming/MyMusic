#pragma once

#include "MysqlDao.h"
#include "Singleton.h"
#include <vector>

class MysqlManager : public Singleton<MysqlManager>
{
	friend class Singleton<MysqlManager>;
public:
	~MysqlManager() {}
	bool GetAllMusicInfo(MusicInfoListPtr& music_list_info);
	std::shared_ptr<UserInfo> GetUserInfo(const int& uid);
	std::shared_ptr<UserInfo> GetUserInfo(const std::string& name);

private:
	MysqlManager() {}
	MySqlDao _dao;
};

