#pragma once
#include "global.h"
#include "Singleton.h"
#include "HttpConnection.h"

typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpTask;
class LogicSystem : public Singleton<LogicSystem>
{
	friend Singleton<LogicSystem>;
public:
	~LogicSystem() = default;
	void RegisterPost(std::string url, HttpTask task);
	void RegisterGet(std::string url, HttpTask task);
	bool HandlePost(std::string, std::shared_ptr<HttpConnection> connection);
	bool HandleGet(std::string url, std::shared_ptr<HttpConnection> connection);
private:
	LogicSystem();
	std::map<std::string, HttpTask> _post_handlers;
	std::map<std::string, HttpTask> _get_handlers;
};

