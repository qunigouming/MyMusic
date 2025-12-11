#include <thread>
#include <atomic>
#include <csignal>

#include "Common/LogManager.h"
#include "Common/ConfigManager.h"
#include "gRPC/StorageServiceImpl.h"

std::atomic_bool shutdown_requested = false;
std::condition_variable cv;

void signal_handler(int signal)
{
	shutdown_requested = true;
	cv.notify_all();
}

int main(int argc, char* argv[])
{
	LogManager::InitGlog(argv[0]);
	auto& cfg = ConfigManager::GetInstance();
	try {
		StorageServiceImpl service;
		grpc::ServerBuilder builder;
		std::string server_address = cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"];
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        LOG(INFO) << "StorageServer listening on " << server_address;

		std::mutex mutex;
		std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

		{
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&] { return shutdown_requested.load(); });
		}

		LOG(INFO) << "Shutting down server";
		server->Shutdown();
	}
	catch (std::exception& e) {
		LOG(ERROR) << "Exception: " << e.what();
	}
	return 0;
}
