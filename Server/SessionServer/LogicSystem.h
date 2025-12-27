#pragma once
#include "Singleton.h"
#include "global.h"
#include <map>
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

	std::thread _workThread;
	std::condition_variable _cond;
	std::mutex _mutex;
	std::atomic_bool _b_stop = false;
	std::queue<std::shared_ptr<LogicNode>> _msg_que;
	std::map<int, funcCallBack> _handler;

	std::shared_ptr<Server> _server = nullptr;

	Song _song;
};

