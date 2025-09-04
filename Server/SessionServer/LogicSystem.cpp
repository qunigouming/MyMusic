#include "LogicSystem.h"
#include "RedisManager.h"
#include "ConfigManager.h"
#include "MysqlManager.h"

LogicSystem::~LogicSystem()
{
	_b_stop = true;
	_cond.notify_one();
	_workThread.join();
}

void LogicSystem::PustMsg(std::shared_ptr<LogicNode> msg)
{
	std::unique_lock<std::mutex> lock(_mutex);
	_msg_que.push(msg);
	if (_msg_que.size() == 1) {
		lock.unlock();
		_cond.notify_one();
	}
}

LogicSystem::LogicSystem()
{
	RegisterCallBack();
	_workThread = std::thread(&LogicSystem::Run, this);
}

void LogicSystem::Run()
{
	for (;;) {
		std::unique_lock<std::mutex> lock(_mutex);
		//wait resource push into message queue
		while (_msg_que.empty() && !_b_stop) _cond.wait(lock);
		if (_b_stop) {
			//handling remains request
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				std::cout << "recv message id is " << msg_node->_message->_msg_id << std::endl;
				auto func = _handler.find(msg_node->_message->_msg_id);
				if (func == _handler.end()) {
					_msg_que.pop();
					continue;
				}
				func->second(msg_node->_session, msg_node->_message->_msg_id, std::string(msg_node->_message->_data, msg_node->_message->_total_len));
				_msg_que.pop();
			}
			break;
		}

		auto msg_node = _msg_que.front();
		std::cout << "recv message id is " << msg_node->_message->_msg_id << std::endl;
		auto func = _handler.find(msg_node->_message->_msg_id);
		if (func == _handler.end()) {
			_msg_que.pop();
			std::cout << "message id [" << msg_node->_message->_msg_id << "] handler not found" << std::endl;
			continue;
		}
		func->second(msg_node->_session, msg_node->_message->_msg_id, std::string(msg_node->_message->_data, msg_node->_message->_total_len));
		_msg_que.pop();
	}
}

void LogicSystem::RegisterCallBack()
{
	_handler[MSG_USER_LOGIN_REQ] = std::bind(&LogicSystem::LoginHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_handler[ID_HEARTBEAT_REQ] = std::bind(&LogicSystem::HeartBeatHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::LoginHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
	Json::Value root;
	Json::Reader reader;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "user login uid is " << uid << "user token is " << token << std::endl;
	Json::Value rspJson;
	Defer defer([this, &rspJson, session] {
		std::string str = rspJson.toStyledString();
		session->Send(str, MSG_USER_LOGIN_RSP);
	});

	// 验证token
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisManager::GetInstance()->Get(token_key, token_value);
	if (!success) {
		rspJson["error"] = ErrorCodes::UidInvalid;
		return;
	}
	if (token_value != token) {
		rspJson["error"] = ErrorCodes::TokenInvalid;
		return;
	}
	rspJson["error"] = ErrorCodes::Success;

	// 获取用户初始登录信息
	std::string base_key = USER_BASE_INFO + uid_str;
	auto user_info = std::make_shared<UserInfo>();
	success = GetBaseInfo(base_key, uid, user_info);
	if (!success) {
        rspJson["error"] = ErrorCodes::UidInvalid;
		return;
	}
	Json::Value user_self_info;
	user_self_info["uid"] = user_info->uid;
	user_self_info["name"] = user_info->name;
    user_self_info["pwd"] = user_info->pwd;
    user_self_info["sex"] = user_info->sex;
    user_self_info["icon"] = user_info->icon;
    user_self_info["email"] = user_info->email;
	rspJson["user_self_info"] = user_self_info;

	// 获取所有音乐
	std::list<std::shared_ptr<MusicInfo>> music_list;
	success = MysqlManager::GetInstance()->GetAllMusicInfo(music_list);
	if (!success) {
		rspJson["error"] = ErrorCodes::EtherInvalid;
		return;
	}
	for (auto& music_info : music_list) {
		Json::Value music_info_json;
        music_info_json["title"] = music_info->title;
		music_info_json["album"] = music_info->album;
        music_info_json["song_icon"] = music_info->song_icon;
        music_info_json["artists"] = music_info->artists;
        music_info_json["duration"] = music_info->duration;
        music_info_json["file_url"] = music_info->file_url;
        music_info_json["is_like"] = music_info->is_like;

        rspJson["music_list"].append(music_info_json);
	}

	//connection add to redis
	auto server_name = ConfigManager::GetInstance()["SelfServer"]["Name"];
	int con_count = 0;
	auto redis_res = RedisManager::GetInstance()->HGet(LOGINCOUNT, server_name);		//check connection count in redis
	if (!redis_res.empty()) con_count = std::stoi(redis_res);
	++con_count;
	RedisManager::GetInstance()->HSet(LOGINCOUNT, server_name, std::to_string(con_count));
	return;
}

void LogicSystem::HeartBeatHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
	Json::Value root;
	Json::Reader reader;
	reader.parse(msg_data, root);
	auto uid = root["fromuid"].asInt();
	std::cout << "user heartbeat uid is " << uid << std::endl;
	Json::Value rspJson;
    rspJson["error"] = ErrorCodes::Success;
    session->Send(rspJson.toStyledString(), ID_HEARTBEAT_RSP);
}

bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	std::string info_str = "";
	bool b_base = RedisManager::GetInstance()->Get(base_key, info_str);
	// Redis中有数据，从Redis中获取
	if (b_base) {
		Json::Reader reader;
		Json::Value base_info;
		reader.parse(info_str, base_info);
		userinfo->uid = base_info["uid"].asInt();
		userinfo->name = base_info["name"].asString();
		userinfo->pwd = base_info["pwd"].asString();
		userinfo->email = base_info["email"].asString();
		userinfo->sex = base_info["sex"].asInt();
		userinfo->icon = base_info["icon"].asString();
		std::cout << "user login the uid:" << userinfo->uid << " name:" << userinfo->name << " pwd:" << userinfo->pwd << " email:" << userinfo->email << " sex:" << userinfo->sex << " icon:" << userinfo->icon << std::endl;
	}
	else {
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = MysqlManager::GetInstance()->GetUserInfo(userinfo->uid);
		if (!user_info)	return false;
		userinfo = user_info;

		// 将数据库数据写入缓存
		Json::Value redis_root;
		redis_root["uid"] = userinfo->uid;
		redis_root["name"] = userinfo->name;
		redis_root["pwd"] = userinfo->pwd;
		redis_root["email"] = userinfo->email;
		redis_root["sex"] = userinfo->sex;
		redis_root["icon"] = userinfo->icon;
		RedisManager::GetInstance()->Set(base_key, redis_root.toStyledString());
	}
	return true;
}

bool LogicSystem::GetAllMusicInfo(std::list<std::shared_ptr<MusicInfo>>& music_list_info)
{
	return MysqlManager::GetInstance()->GetAllMusicInfo(music_list_info);
}
