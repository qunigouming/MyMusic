#include "VerifyServiceImpl.h"
#include "ConfigManager.h"
#include <boost/asio.hpp>
#include <thread>
#include "RedisManager.h"
#include "Email.h"
#include "LogManager.h"

void RunServer() {
	auto& cfg = ConfigManager::GetInstance();

	std::string server_address(cfg["VerifyServer"]["Host"] + ":" + cfg["VerifyServer"]["Port"]);
	VerifyServiceImpl service;
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
	LOG(INFO) << "VerifyServer listening on " << server_address;

	boost::asio::io_context io_service;
	boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
	signals.async_wait([&server, &io_service](const boost::system::error_code& error, int signal_number) {
		if (!error) {
			LOG(INFO) << "Signal " << signal_number << " received. Shutting down...";
			server->Shutdown();
			io_service.stop();
		}
	});

	std::thread([&io_service]() {
		io_service.run();
	}).detach();

	server->Wait();
}

int main(int argc, char* argv[]) {
	LogManager::InitGlog(argv[0]);
	try {
		RunServer();
		RedisManager::GetInstance()->Close();
	} catch (std::exception& e) {
		LOG(ERROR) << "Exception: " << e.what();
		RedisManager::GetInstance()->Close();
		return EXIT_FAILURE;
	}
}