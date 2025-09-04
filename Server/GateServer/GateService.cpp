#include "GateService.h"
#include "AsioIOServicePool.h"
#include "HttpConnection.h"

GateService::GateService(boost::asio::io_context& ioc, unsigned short& port) : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
}

void GateService::Start()
{
	auto self = shared_from_this();
	auto& ioc_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(ioc_context);
	_acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
		try {
			//错误处理
			if (ec) {
				self->Start();
				return;
			}
			//启动新接收
			new_con->Start();
			self->Start();
		}
		catch (std::exception& exp) {
			std::cout << "acceptor exception is " << exp.what() << std::endl;
			self->Start();
		}
	});
}
