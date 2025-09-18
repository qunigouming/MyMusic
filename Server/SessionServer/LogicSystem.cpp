#include "LogicSystem.h"
#include "RedisManager.h"
#include "ConfigManager.h"
#include "MysqlManager.h"
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <fstream>
#include <sstream>
#include <codecvt>

std::string decode_base64(const std::string& val) {
	if (val.empty())	return "";
	using namespace boost::archive::iterators;
	using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;

	// 复制输入字符串以便修改
	std::string tmp = val;

	// 计算需要添加的填充字符数量
	size_t pad_chars = (4 - tmp.size() % 4) % 4;
	tmp.append(pad_chars, '=');

	// 替换 URL 安全的 Base64 字符（如果存在）
	std::replace(tmp.begin(), tmp.end(), '-', '+');
	std::replace(tmp.begin(), tmp.end(), '_', '/');

	// 解码
	return std::string(It(tmp.begin()), It(tmp.end()));
}

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
	_handler[ID_LOGIN_USER_REQ] = std::bind(&LogicSystem::LoginHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_handler[ID_HEARTBEAT_REQ] = std::bind(&LogicSystem::HeartBeatHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_handler[ID_UPLOAD_FILE_REQ] = std::bind(&LogicSystem::UploadFileHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_handler[ID_UPLOAD_META_TYPE_REQ] = std::bind(&LogicSystem::UploadMetaTypeHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
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
		session->Send(str, ID_LOGIN_USER_RSP);
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

void LogicSystem::UploadFileHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
    Json::Value root;
    Json::Reader reader;
    reader.parse(msg_data, root);

	// 将文件保存
	auto data = root["data"].asString();
	Json::Value retValue;
	Defer defer([this, &retValue, session] {
		std::string str = retValue.toStyledString();
		session->Send(str, ID_UPLOAD_FILE_RSP);
	});
	std::string decoded = decode_base64(data);
	auto seq = root["seq"].asInt();
	auto name = root["name"].asString();
	auto total_size = root["total_size"].asInt();
	auto trans_size = root["trans_size"].asInt();
	auto file_path = ConfigManager::GetInstance()["Store"]["MusicPath"] + name;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wide_path = converter.from_bytes(file_path);
	std::cout << "file path is " << file_path << std::endl;
	std::ofstream outfile;
	if (seq == 1) {
		outfile.open(wide_path, std::ios::out | std::ios::binary | std::ios::trunc);
	}
	else {
        outfile.open(wide_path, std::ios::out | std::ios::binary | std::ios::app);
	}

	if (!outfile) {
		std::cerr << "open file failed" << std::endl;
		return;
	}
	outfile.write(decoded.c_str(), decoded.size());
	if (!outfile) {
		std::cerr << "write file failed" << std::endl;
		return;
	}

	outfile.close();
	std::cout << "file upload success" << std::endl;
    retValue["error"] = ErrorCodes::Success;
    retValue["seq"] = seq;
	retValue["name"] = name;
    retValue["total_size"] = total_size;
	retValue["trans_size"] = trans_size;
	std::cout << "file upload progress: " << trans_size << "/" << total_size << std::endl;
	if (total_size == trans_size) {
		std::cout << "file upload complete" << std::endl;
		_song.file_url = ConfigManager::GetInstance()["Store"]["RemotePath"] + name;
		try {
			MysqlManager::GetInstance()->getOrCreateSong(_song);
		}
		catch (std::exception& e) {
			std::cerr << "Exception: " << e.what() << std::endl;
		}
		_song.Clear();
	}
}

void LogicSystem::UploadMetaTypeHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
	// 将数据转存到数据库中
	Json::Value root;
    Json::Reader reader;
    reader.parse(msg_data, root);

	Json::Value retValue;
    retValue["error"] = ErrorCodes::Success;
    Defer defer([this, &retValue, session] {
		std::string str = retValue.toStyledString();
		session->Send(str, ID_UPLOAD_META_TYPE_RSP);
	});

	// 插入专辑
	Album album;
	album.title = root["album"].asString();
	album.artist_name = root["artists"].asString();
	std::string title = root["title"].asString();
	// 将图片数据存储到磁盘中
	std::string cover_data = decode_base64(root["icon"].asString());
	std::string cover_url;
	if (!cover_data.empty()) {	
		cover_url = ConfigManager::GetInstance()["Store"]["CoverPath"] + title + ".png";
		// 转换为宽字符处理，后续可能考虑用其他库处理
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wide_path = converter.from_bytes(cover_url);
		std::ofstream file;
		file.open(wide_path, std::ios::out | std::ios::binary | std::ios::trunc);
		std::cout << "cover url is " << cover_url << "\t" << title << std::endl;
		if (!file) {
			int errnum = errno;
			std::cerr << "open file failed" << errnum << std::endl;
			retValue["error"] = ErrorCodes::EtherInvalid;
			return;
		}
		file.write(cover_data.c_str(), cover_data.size());
		file.close();
	}
    album.cover_url = cover_url;
    album.description = root["description"].asString();
    album.release_date = root["release_date"].asString();
	MysqlManager::GetInstance()->getOrCreateAlbum(album);

	// 保存歌曲相关信息，以备后续插入歌曲
	_song.title = root["title"].asString();
	_song.album_title = root["album"].asString();

	std::string artists = root["artists"].asString();
	std::istringstream ss(artists);
	std::string artist;
	while (std::getline(ss, artist, '/')) _song.artist_names.push_back(artist);

	_song.track_number = std::stoi(root["track"].asString());
	_song.duration = std::stoi(root["duration"].asString());
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
		user_info = MysqlManager::GetInstance()->GetUserInfo(uid);
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
