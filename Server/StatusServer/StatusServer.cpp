#include <iostream>
#include "global.h"
#include "ConfigManager.h"
#include "RedisManager.h"
#include "StatusServiceImpl.h"

void RunServer() {
    auto& cfg = ConfigManager::GetInstance();

    std::string server_address(cfg["StatusServer"]["Host"] + ":" + cfg["StatusServer"]["Port"]);
    StatusServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "StatusServer listening on " << server_address << std::endl;

    boost::asio::io_service io_context;

    // Set up a signal set
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&server, &io_context](const boost::system::error_code& error, int signal_number) {
        if (!error) {
            std::cout << "Signal " << signal_number << " received. Shutting down..." << std::endl;
            server->Shutdown();
            io_context.stop();
        }
    });

    // Run the io_service in a separate thread
    std::thread([&io_context]() {
        io_context.run();
    }).detach();

    // Wait for server to shutdown
    server->Wait();
}

int main()
{
    system("chcp 65001");
    try {
        RunServer();
        RedisManager::GetInstance()->Close();
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        RedisManager::GetInstance()->Close();
        return EXIT_FAILURE;
    }
}
