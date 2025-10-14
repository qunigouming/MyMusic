#include "ConfigManager.h"
#include "GateService.h"
#include "LogManager.h"

int main(int argc, char* argv[]) {
    LogManager::InitGlog(argv[0]);
	auto& cfgMgr = ConfigManager::GetInstance();
	std::string gate_port = cfgMgr["GateServer"]["Port"];
	std::cout << gate_port << std::endl;
	unsigned short port = static_cast<unsigned short>(atoi(gate_port.c_str()));
	try {
		boost::asio::io_context ioc{ 1 };
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number) {
			if (error)	return;
			ioc.stop();
		});
		std::make_shared<GateService>(ioc, port)->Start();
		std::cout << "GateServer listen on port" << port << std::endl;
		ioc.run();
	}
	catch (std::exception& exp) {
		std::cout << "Error: " << exp.what() << std::endl;
		return EXIT_FAILURE;
	}
}