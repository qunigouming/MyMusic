#pragma once
#include "global.h"
#include <map>
#include "Session.h"

using boost::asio::ip::tcp;

class Server : public std::enable_shared_from_this<Server>
{
public:
	Server(boost::asio::io_context& ioc, short port);
	~Server();
	void ClearSession(std::string uid);
	bool CheckVaild(std::string& uid);
	void On_Timer(const boost::system::error_code& ec);
	void StartTimer();
	void StopTimer();
private:
	void HandleAccept(std::shared_ptr<Session> new_session, const boost::system::error_code& error);
	void Accept();
	boost::asio::io_context& _ioc;
	short _port;
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<Session>> _sessions;
	std::mutex _mutex;
	boost::asio::steady_timer _timer;
};

