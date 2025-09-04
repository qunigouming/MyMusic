#pragma once

#include "global.h"

class GateService : public std::enable_shared_from_this<GateService>
{
public:
	GateService(boost::asio::io_context& ioc, unsigned short& port);
	void Start();
private:
	tcp::acceptor _acceptor;
	boost::asio::io_context& _ioc;
};

