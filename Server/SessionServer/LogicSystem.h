#pragma once
#include "Singleton.h"
#include "global.h"
#include <map>
#include <mutex>
#include <ctime>
#include <filesystem>
#include <unordered_map>
#include "Session.h"
#include "dataInfo.h"

typedef std::function<void(std::shared_ptr<Session>, const short& msg_id, const std::string& msg_data)> funcCallBack;
class LogicSystem : public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PustMsg(std::shared_ptr<LogicNode> msg);
	void SetServer(std::shared_ptr<Server> server);
private:
	LogicSystem();
	void Run();
	void RegisterCallBack();
	void LoginHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);			// 登录请求
	void HeartBeatHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);		// 心跳包请求
	void UploadFileHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);		// 上传文件请求
	void UploadMetaTypeHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);	// 上传文件元数据请求
	void CollectSongHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);	// 收藏歌曲请求
    void GetCollectSongListHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);		// 获取收藏歌单信息请求(一般信息)
	void GetSongListPageInfoHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);	// 获取收藏歌单列表请求(歌单歌曲)

	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	bool CheckToken(int uid, const std::string& token);

	std::thread _workThread;
	std::condition_variable _cond;
	std::mutex _mutex;
	std::atomic_bool _b_stop = false;
	std::queue<std::shared_ptr<LogicNode>> _msg_que;
	std::map<int, funcCallBack> _handler;

	std::shared_ptr<Server> _server = nullptr;

	Song _song;

	// 音频上传 spool：客户端分片暂存（内存/临时文件），完成后 gRPC 流式转发 StorageServer
	struct SpoolEntry {
		std::string mem;                    // 内存缓冲（未落盘时）
		std::filesystem::path tmp_path;     // 落盘后的临时文件路径（空表示未落盘）
		std::string name;                   // 清洗后的文件名
		int64_t total = 0;                  // total_size
		int64_t received = 0;
		std::time_t last_active = 0;
	};
	std::mutex _spool_mutex;
	std::unordered_map<std::string, SpoolEntry> _spools;   // key = "uid:清洗后文件名"
};

