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
	void LoginHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);
	void HeartBeatHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);
	void UploadFileHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);
	void UploadMetaTypeHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);
	void CollectSongHandler(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);

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

