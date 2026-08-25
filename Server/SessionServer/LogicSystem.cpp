#include "LogicSystem.h"
#include "RedisManager.h"
#include "ConfigManager.h"
#include "MysqlManager.h"
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <iomanip>
#include <openssl/md5.h>
#include <filesystem>
#include <ctime>
#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "UserManager.h"
#include "Server.h"
#include "SessionGrpcClient.h"
#include "StorageGrpcClient.h"
#include "LogManager.h"
#include "Common/Tools/ImgFmtInspector/ImgFmtInspector.h"

#define DEBUG_TEST_UPLOAD_FILE_FUNCTION

namespace {

// 音频 spool 参数：8MB 以内内存缓冲，超过落盘临时文件；10 分钟未活动视为断传
const int64_t SPOOL_MEM_THRESHOLD = 8 * 1024 * 1024;
const int SPOOL_STALE_SECONDS = 600;

// UploadAudio 读取回调的状态：先读内存缓冲，再读临时文件
struct SpoolReadState {
	std::string mem;
	size_t mem_off = 0;
	std::ifstream ifs;
};

// 生成会话 token（UUID 随机串，与 StatusServer 握手 token 同风格）
std::string generate_session_token() {
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	return boost::uuids::to_string(uuid);
}

} // namespace

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
	std::string out(It(tmp.begin()), It(tmp.end()));
	// transform_width 会把 '=' 填充位的残留比特也解成多余字节（每分片多 1 字节，
	// 导致上传文件每 32KB 错位），按有效字符数截断到正确长度
	size_t valid_chars = std::count_if(tmp.begin(), tmp.end(), [](char c) { return c != '='; });
	out.resize(valid_chars * 3 / 4);
	return out;
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

void LogicSystem::SetServer(std::shared_ptr<Server> server)
{
	_server = server;
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
				LOG(INFO) << "recv message id is " << msg_node->_message->_msg_id;
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
		LOG_IF(INFO, msg_node->_message->_msg_id != ID_HEARTBEAT_REQ) << "recv message id is " << msg_node->_message->_msg_id;
		auto func = _handler.find(msg_node->_message->_msg_id);
		if (func == _handler.end()) {
			_msg_que.pop();
			LOG(INFO) << "message id [" << msg_node->_message->_msg_id << "] handler not found";
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
	_handler[ID_COLLECT_SONG_REQ] = std::bind(&LogicSystem::CollectSongHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_handler[ID_GET_COLLECT_SONG_LIST_INFO_REQ] = std::bind(&LogicSystem::GetCollectSongListHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_handler[ID_GET_COLLECT_SONG_LIST_REQ] = std::bind(&LogicSystem::GetSongListPageInfoHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::LoginHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
	Json::Value root;
	Json::Reader reader;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	LOG(INFO) << "user login uid is " << uid;
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
	// token 只允许使用一次，防止重放
	RedisManager::GetInstance()->Del(token_key);
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
    user_self_info["sex"] = user_info->sex;
    user_self_info["icon"] = user_info->icon;
    user_self_info["email"] = user_info->email;
	rspJson["user_self_info"] = user_self_info;

	// 获取所有音乐
	std::list<std::shared_ptr<MusicInfo>> music_list;
	success = MysqlManager::GetInstance()->GetAllMusicInfo(uid, music_list);
	if (!success) {
		rspJson["error"] = ErrorCodes::EtherInvalid;
		return;
	}
	for (auto& music_info : music_list) {
		Json::Value music_info_json;
		music_info_json["id"] = music_info->id;
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
	{
		// 分布式锁保证原子性
		auto lock_key = LOCK_PREFIX + uid_str;
		auto identifier = RedisManager::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
		Defer DistLockDefer([this, identifier, lock_key] {
			RedisManager::GetInstance()->releaseLock(lock_key, identifier);
		});

		// 检测用户是否在其他或本服务器登录
		std::string uid_ip_value = "";
		auto uid_ip_key = USER_IP_PREFIX + uid_str;
		bool success = RedisManager::GetInstance()->Get(uid_ip_key, uid_ip_value);
		if (success) {
			// 用户已经登录，踢出之前的登录
			auto& config = ConfigManager::GetInstance();
			auto self_name = config["SelfServer"]["Name"];
			// 若为本服务器，直接踢出
			if (uid_ip_value == server_name) {
				auto old_session = UserManager::GetInstance()->GetSession(uid);

				// 发送踢人信息
				if (old_session) {
					old_session->NotifyOffline(uid);

					_server->ClearSession(old_session->GetSessionId());
				}
			}

			// 其他服务器在登录，RPC通知其他服务器踢出连接
			KickUserReq req;
            req.set_uid(uid);
			SessionGrpcClient::GetInstance()->NotifyKickUser(uid_ip_value, req);
		}

		session->SetUserUid(uid);
		std::string ip_key = USER_IP_PREFIX + uid_str;
		RedisManager::GetInstance()->Set(ip_key, server_name);

		// 将uid与Session关联
		UserManager::GetInstance()->AddSession(uid, session);
		std::string uid_session_key = USER_SESSION_PREFIX + uid_str;
		RedisManager::GetInstance()->Set(uid_session_key, session->GetSessionId());
	}

	// 更新连接数
    RedisManager::GetInstance()->IncreaseCount(server_name);

	// 签发会话token（30分钟过期，心跳续期）——校验半边已有，签发半边此前缺失
	auto new_token = generate_session_token();
	auto session_token_key = USERTOKENPREFIX + uid_str;
	RedisManager::GetInstance()->Set(session_token_key, new_token);
	RedisManager::GetInstance()->Expire(session_token_key, TOKEN_EXPIRE_TIME);
	rspJson["token"] = new_token;
	return;
}

void LogicSystem::HeartBeatHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
	Json::Value root;
	Json::Reader reader;
	Json::Value rspJson;
	rspJson["error"] = ErrorCodes::Success;

	reader.parse(msg_data, root);
	auto uid = root["fromuid"].asInt();
	auto token = root["token"].asString();
	if (uid <= 0 || uid != session->GetUserUid() || !CheckToken(uid, token)) {
		rspJson["error"] = ErrorCodes::TokenInvalid;
		session->Send(rspJson.toStyledString(), ID_HEARTBEAT_RSP);
		return;
	}
	RedisManager::GetInstance()->Expire(USERTOKENPREFIX + std::to_string(uid), TOKEN_EXPIRE_TIME);
	LOG_EVERY_N(INFO, 30) << "user heartbeat uid is " << uid;
    session->Send(rspJson.toStyledString(), ID_HEARTBEAT_RSP);
}

void LogicSystem::UploadFileHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
    Json::Value root;
    Json::Reader reader;
	Json::Value retValue;
	Defer defer([this, &retValue, session] {
		std::string str = retValue.toStyledString();
		session->Send(str, ID_UPLOAD_FILE_RSP);
	});
    reader.parse(msg_data, root);

	// 验证token是否合法
	auto uid = session->GetUserUid();
	auto token = root["token"].asString();

	if (!CheckToken(uid, token)) {
		retValue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	try {
	auto data = root["data"].asString();
	std::string decoded = decode_base64(data);
	auto seq = root["seq"].asInt();
	auto name = root["name"].asString();
	auto total_size = root["total_size"].asInt();
	auto trans_size = root["trans_size"].asInt();

	// spool 键：uid + 清洗后的文件名（清洗防路径穿越）
	std::string safe_name = name;
	for (auto& ch : safe_name) {
		if (ch == '/' || ch == '\\' || ch == ':') ch = '_';
	}
	std::string spool_key = std::to_string(uid) + ":" + safe_name;

	std::unique_lock<std::mutex> lock(_spool_mutex);
	auto now = std::time(nullptr);

	if (seq == 1) {
		// 新上传：清扫断传的过期条目，并清理同名旧条目
		for (auto it = _spools.begin(); it != _spools.end();) {
			if (now - it->second.last_active > SPOOL_STALE_SECONDS || it->first == spool_key) {
				if (!it->second.tmp_path.empty()) {
					std::error_code ec;
					std::filesystem::remove(it->second.tmp_path, ec);
				}
				it = _spools.erase(it);
			}
			else {
				++it;
			}
		}
		SpoolEntry entry;
		entry.name = safe_name;
		entry.total = total_size;
		entry.last_active = now;
		_spools[spool_key] = std::move(entry);
	}

	auto it = _spools.find(spool_key);
	if (it == _spools.end()) {
		retValue["error"] = ErrorCodes::EtherInvalid;
		return;
	}
	SpoolEntry& entry = it->second;
	entry.last_active = now;
	entry.received += static_cast<int64_t>(decoded.size());

	// 追加分片：优先内存缓冲，超阈值落盘临时文件
	if (entry.tmp_path.empty() && static_cast<int64_t>(entry.mem.size() + decoded.size()) <= SPOOL_MEM_THRESHOLD) {
		entry.mem += decoded;
	}
	else {
		if (entry.tmp_path.empty()) {
			auto spool_dir = ConfigManager::GetInstance()["Store"]["SpoolPath"];
			if (spool_dir.empty()) {
				spool_dir = ConfigManager::GetInstance()["Store"]["MusicPath"];
			}
			// 临时文件名不能含 ':'（spool_key 的 uid 分隔符是 Windows 非法字符，曾导致崩溃）
			std::string tmp_name = std::to_string(uid) + "_" + safe_name + ".tmp";
			// 用 u8path 按 UTF-8 解释文件名，避免含非 GBK 字符（emoji 等）时 ANSI 转换抛异常
			entry.tmp_path = std::filesystem::path(spool_dir) / std::filesystem::u8path(tmp_name);
		}
		std::ofstream outfile(entry.tmp_path, std::ios::out | std::ios::binary | std::ios::app);
		if (!outfile) {
			LOG(ERROR) << "open spool file failed: " << entry.tmp_path;
			retValue["error"] = ErrorCodes::EtherInvalid;
			return;
		}
		if (!entry.mem.empty()) {
			outfile.write(entry.mem.c_str(), static_cast<std::streamsize>(entry.mem.size()));
			entry.mem.clear();
		}
		outfile.write(decoded.c_str(), static_cast<std::streamsize>(decoded.size()));
		if (!outfile) {
			LOG(ERROR) << "write spool file failed";
			retValue["error"] = ErrorCodes::EtherInvalid;
			return;
		}
		outfile.close();
	}

	LOG(INFO) << "file upload progress: " << trans_size << "/" << total_size;
    retValue["error"] = ErrorCodes::Success;
    retValue["seq"] = seq;
	retValue["name"] = name;
    retValue["total_size"] = total_size;
	retValue["trans_size"] = trans_size;

	// 最后一片：构造读取回调，gRPC 流式转发 StorageServer → FastDFS
	if (total_size == trans_size) {
		LOG(INFO) << "file upload complete, forwarding to StorageServer: " << spool_key;
		SpoolEntry finished = std::move(entry);
		_spools.erase(it);
		// 网络 IO 不在锁内进行
		lock.unlock();

		auto state = std::make_shared<SpoolReadState>();
		state->mem = std::move(finished.mem);
		if (!finished.tmp_path.empty()) {
			state->ifs.open(finished.tmp_path, std::ios::binary);
		}
		StorageGrpcClient::ReadCallback read_cb = [state](char* buf, size_t max_len) -> size_t {
			if (state->mem_off < state->mem.size()) {
				size_t got = std::min(max_len, state->mem.size() - state->mem_off);
				memcpy(buf, state->mem.data() + state->mem_off, got);
				state->mem_off += got;
				return got;
			}
			if (state->ifs.is_open()) {
				state->ifs.read(buf, static_cast<std::streamsize>(max_len));
				return static_cast<size_t>(state->ifs.gcount());
			}
			return 0;
		};

		auto response = StorageGrpcClient::GetInstance()->UploadAudio(finished.name, finished.total, read_cb);

		// 清理临时文件（先关流再删，Windows 上打开的文件无法删除）
		if (state->ifs.is_open()) {
			state->ifs.close();
		}
		if (!finished.tmp_path.empty()) {
			std::error_code ec;
			std::filesystem::remove(finished.tmp_path, ec);
		}

		if (response.error() != ErrorCodes::Success) {
			LOG(ERROR) << "UploadAudio to StorageServer failed";
			retValue["error"] = ErrorCodes::EtherInvalid;
			return;
		}
		_song.file_url = response.storage_url();
		try {
			MysqlManager::GetInstance()->getOrCreateSong(_song);
		}
		catch (std::exception& e) {
			LOG(ERROR) << "Exception: " << e.what();
			retValue["error"] = ErrorCodes::EtherInvalid;
			return;
		}
		_song.Clear();
	}
	}
	catch (std::exception& e) {
		// 任何 spool/转发异常都回错误响应，避免工作线程崩溃
		LOG(ERROR) << "UploadFileHandler exception: " << e.what();
		retValue["error"] = ErrorCodes::EtherInvalid;
	}
}

void LogicSystem::UploadMetaTypeHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
#ifndef DEBUG_TEST_UPLOAD_FILE_FUNCTION
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
		LOG(INFO) << "cover url is " << cover_url << "\t" << title;
		if (!file) {
			int errnum = errno;
			LOG(ERROR) << "open file failed" << errnum;
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

	_song.track_number = root["track"].asInt();
	_song.duration = root["duration"].asInt();

#else
    Json::Value root;
	Json::Reader reader;
	reader.parse(msg_data, root);

	Json::Value retValue;
	retValue["error"] = ErrorCodes::Success;
	Defer defer([this, &retValue, session] {
		std::string str = retValue.toStyledString();
		session->Send(str, ID_UPLOAD_META_TYPE_RSP);
	});

	// 验证token是否合法
	auto uid = session->GetUserUid();
	auto token = root["token"].asString();

	if (!CheckToken(uid, token)) {
		retValue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	// 将专辑信息存储到数据库中
	// 插入专辑
	Album album;
	album.title = root["album"].asString();
	album.artist_name = root["artists"].asString();
	std::string title = root["title"].asString();
	std::string cover_data = decode_base64(root["icon"].asString());
	if (!cover_data.empty()) {
		// 图片映射地址存储到数据库中
		std::string cover_name = album.title + "_" + album.artist_name + "_" + title;
		std::string mime_type = ImgFmtInspector::getImageMimeType(cover_data);
		UploadImageResponse response = StorageGrpcClient::GetInstance()->UploadImage(cover_name, cover_data, mime_type);
		if (response.error() != ErrorCodes::Success) {
			LOG(INFO) << "Unknown Error: Upload Image Failed!!!";
            retValue["error"] = ErrorCodes::EtherInvalid;
			return;
		}
		FileMapInfo file_map_info;
		file_map_info.storage_path = response.storage_url();
		file_map_info.mime_type = mime_type;
		file_map_info.create_id = session->GetUserUid();
		int cover_url_id = MysqlManager::GetInstance()->createFileMap(file_map_info);
		album.cover_url_id = cover_url_id;
	}
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

	_song.track_number = root["track"].asInt();
	_song.duration = root["duration"].asInt();
#endif
}

void LogicSystem::CollectSongHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
    Json::Value root;
    Json::Reader reader;
    reader.parse(msg_data, root);

    Json::Value retValue;
    retValue["error"] = ErrorCodes::Success;
    // 回显请求字段，客户端在失败时能准确回滚心形状态
    retValue["song_id"] = root["song_id"].asInt();
    retValue["status"] = root["status"].asBool();
    Defer defer([this, &retValue, session] {
		std::string str = retValue.toStyledString();
		session->Send(str, ID_COLLECT_SONG_RSP);
	});

	// 验证token是否合法
	auto uid = session->GetUserUid();
	auto token = root["token"].asString();

	if (!CheckToken(uid, token)) {
		retValue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	if (root["flag"].asBool()) {
		// 其他类型歌单
	}

	try {
		// 默认歌单处理
		if (root["status"].asBool()) {
			// 添加收藏歌曲		每次用最新添加的歌曲来当作cover
			Playlist playlist;
			playlist.name = "喜欢的音乐";
			playlist.is_default = true;
			playlist.description = "";
			playlist.user_id = session->GetUserUid();
			playlist.cover_url_id = MysqlManager::GetInstance()->getCoverUrlId(root["song_id"].asInt());
			MysqlManager::GetInstance()->getOrCreatePlaylist(playlist);			// 创建歌单

			// 添加歌曲
			PlaylistSong playlist_song;
			playlist_song.playlist_name = "喜欢的音乐";
			playlist_song.song_id = root["song_id"].asInt();
	        playlist_song.user_id = session->GetUserUid();
			playlist_song.position = 0;			// 默认添加到最后
			MysqlManager::GetInstance()->createPlaylistSong(playlist_song);
			LOG(INFO) << "收藏歌曲" << root["song_id"].asInt() << "成功";
		}
		else {
			// 取消收藏歌曲
	        MysqlManager::GetInstance()->deletePlaylistSong(session->GetUserUid(), "喜欢的音乐", root["song_id"].asInt());
	        LOG(INFO) << "取消收藏歌曲" << root["song_id"].asInt() << "成功";
		}
	}
	catch (std::exception& e) {
		LOG(ERROR) << "collect song exception: " << e.what();
		retValue["error"] = ErrorCodes::EtherInvalid;
	}
}

void LogicSystem::GetCollectSongListHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
    Json::Value root;
    Json::Reader reader;
    reader.parse(msg_data, root);

    Json::Value retValue;
    retValue["error"] = ErrorCodes::Success;
    Defer defer([this, &retValue, session] {
		std::string str = retValue.toStyledString();
		session->Send(str, ID_GET_COLLECT_SONG_LIST_INFO_RSP);
	});

	// 验证token是否合法
	auto uid = session->GetUserUid();
	auto token = root["token"].asString();

	if (!CheckToken(uid, token)) {
		retValue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	if (root["flag"].asBool()) {

	}
	
	// 默认歌单处理
	std::shared_ptr<SongListPageInfo> pageinfo = MysqlManager::GetInstance()->getSongListPageInfo(session->GetUserUid(), "喜欢的音乐");
	if (pageinfo == nullptr) {
        retValue["error"] = ErrorCodes::EtherInvalid;
		LOG(ERROR) << "获取歌单信息失败";
        return;
	}
	retValue["title"] = pageinfo->title;
	retValue["cover_url"] = pageinfo->songlist_icon;
	retValue["description"] = pageinfo->description;
	retValue["author"] = pageinfo->author;
	retValue["authorIcon"] = pageinfo->authorIcon;
	retValue["createTime"] = pageinfo->createTime;
}

void LogicSystem::GetSongListPageInfoHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
    Json::Value root;
    Json::Reader reader;
    reader.parse(msg_data, root);
    Json::Value retValue;
    retValue["error"] = ErrorCodes::Success;
    Defer defer([this, &retValue, session] {
		std::string str = retValue.toStyledString();
		session->Send(str, ID_GET_COLLECT_SONG_LIST_RSP);
	});

	// 验证token是否合法
	auto uid = session->GetUserUid();
	auto token = root["token"].asString();

	if (!CheckToken(uid, token)) {
		retValue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	MusicInfoListPtr playlistsongs = MysqlManager::GetInstance()->getPlaylistSongs(session->GetUserUid(), root["playlist_name"].asString());
	if (playlistsongs.empty()) {
        retValue["error"] = ErrorCodes::EtherInvalid;
        LOG(ERROR) << "获取歌单歌曲信息失败";
        return;
	}

    // retValue["total_count"] = static_cast<int>(playlistsongs.size());
    for (auto& song : playlistsongs) { 
		Json::Value music_info_json;
		music_info_json["id"] = song->id;
		music_info_json["title"] = song->title;
		music_info_json["album"] = song->album;
		music_info_json["song_icon"] = song->song_icon;
		music_info_json["artists"] = song->artists;
		music_info_json["duration"] = song->duration;
		music_info_json["file_url"] = song->file_url;
		music_info_json["is_like"] = song->is_like;

		retValue["music_list"].append(music_info_json);
	}
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
		userinfo->email = base_info["email"].asString();
		userinfo->sex = base_info["sex"].asInt();
		userinfo->icon = base_info["icon"].asString();
		LOG(INFO) << "user login the uid:" << userinfo->uid << " name:" << userinfo->name << " email:" << userinfo->email << " sex:" << userinfo->sex << " icon:" << userinfo->icon;
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
		redis_root["email"] = userinfo->email;
		redis_root["sex"] = userinfo->sex;
		redis_root["icon"] = userinfo->icon;
		RedisManager::GetInstance()->Set(base_key, redis_root.toStyledString());
	}
	return true;
}

bool LogicSystem::CheckToken(int uid, const std::string& token)
{
	if (uid <= 0 || token.empty()) return false;
	std::string token_key = USERTOKENPREFIX + std::to_string(uid);
	std::string token_value;
	if (!RedisManager::GetInstance()->Get(token_key, token_value))	return false;
	return token_value == token;
}
