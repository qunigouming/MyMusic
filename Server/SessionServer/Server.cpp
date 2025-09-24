#include "Server.h"
#include "AsioIOServicePool.h"
#include "ConfigManager.h"
#include "RedisManager.h"
#include "UserManager.h"

Server::Server(boost::asio::io_context& ioc, short port) : _ioc(ioc), _port(port), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)), _timer(ioc, std::chrono::seconds(TIMER_INTERVAL))
{
	std::cout << "Server Start success, listen on port: " << _port << std::endl;
	Accept();
}

Server::~Server()
{
	std::cout << "Server are destructed on port:" << _port << std::endl;
}

void Server::ClearSession(std::string session_uid)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_sessions.find(session_uid) != _sessions.end()) {
		auto uid = _sessions[session_uid]->GetUserUid();

		UserManager::GetInstance()->RemoveSession(uid, session_uid);
	}

	_sessions.erase(session_uid);
}

bool Server::CheckVaild(std::string& uid)
{
	std::lock_guard<std::mutex> lock(_mutex);
    if (_sessions.find(uid) != _sessions.end()) {
        return true;
    }
	return false;
}

void Server::On_Timer(const boost::system::error_code& ec)
{
	if (ec) {
        std::cout << "Timer error: " << ec.message() << std::endl;
		return;
	}
	std::vector<std::shared_ptr<Session>> expiredSessions;
	int sessionCount = 0;
	std::map<std::string, std::shared_ptr<Session>> session_copy;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		session_copy = _sessions;
	}

	time_t now = std::time(nullptr);
	for (auto session : session_copy) {
		auto b_expired = session.second->IsHeartbeatExpired(now);
		if (b_expired) {
			session.second->Close();
			// 收集过期的session
            expiredSessions.push_back(session.second);
			continue;
		}
		sessionCount++;
	}

	// 设置session数量
	auto& cfg = ConfigManager::GetInstance();
	auto self_name = cfg["SelfServer"]["Name"];
	auto count_str = std::to_string(sessionCount);
	RedisManager::GetInstance()->HSet(LOGINCOUNT, self_name, count_str);

	// 处理过期的session
	for (auto& session : expiredSessions) {
		session->DealExceptionSession();
	}

	// 再次设置定时器
	_timer.expires_after(std::chrono::seconds(TIMER_INTERVAL));
	_timer.async_wait([this](const boost::system::error_code& ec) {
		On_Timer(ec);
	});
}

void Server::StartTimer()
{
	auto self(shared_from_this());
	_timer.async_wait([self](boost::system::error_code ec) {
		self->On_Timer(ec);
	});
}

void Server::StopTimer()
{
	_timer.cancel();
}

void Server::HandleAccept(std::shared_ptr<Session> new_session, const boost::system::error_code& error)
{
	if (!error) {
		new_session->Start();
		std::lock_guard<std::mutex> lock(_mutex);
		_sessions.insert(std::make_pair(new_session->GetSessionId(), new_session));
	}
	else {
		std::cout << "Server acceptance of handling has failed: " << error.what() << std::endl;
	}
	Accept();
}

void Server::Accept()
{
	auto& ioc = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<Session> new_session = std::make_shared<Session>(ioc, this);
	_acceptor.async_accept(new_session->GetSocket(), std::bind(&Server::HandleAccept, this, new_session, std::placeholders::_1));
}




