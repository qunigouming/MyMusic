#include <iostream>
#include "LogicSystem.h"
#include <thread>
#include <mutex>
#include "global.h"
#include "RedisManager.h"
#include "MysqlManager.h"
#include "ConfigManager.h"
#include "AsioIOServicePool.h"
#include "Server.h"
#include <grpcpp/grpcpp.h>
#include "SessionServiceImpl.h"
#include "LogManager.h"

bool b_stop = false;
std::condition_variable cond_quit;
std::mutex mtx_quit;

int main(int argc, char* argv[])
{
    LogManager::InitGlog(argv[0]);
    system("chcp 65001");
    auto& cfg = ConfigManager::GetInstance();
    auto server_name = cfg["SelfServer"]["Name"];
    try {
        auto pool = AsioIOServicePool::GetInstance();
        // 重置登录数
        RedisManager::GetInstance()->HSet(LOGINCOUNT, server_name, "0");
        Defer defer([server_name]() {
            RedisManager::GetInstance()->HDel(LOGINCOUNT, server_name);
            RedisManager::GetInstance()->Close();
        });

        boost::asio::io_context io_context;
        auto port_str = cfg["SelfServer"]["Port"];
        auto pointer_server = std::make_shared<Server>(io_context, std::stoi(port_str));

        pointer_server->StartTimer();

        // 构建Session端联系的GrpcServer
        std::string server_address(cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"]);
        SessionServiceImpl service;
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        service.RegisterServer(pointer_server);

        // 启动rpc服务
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        LOG(INFO) << "RPC Server listening on " << server_address;

        std::thread grpc_server_thread([&server] {
            server->Wait();
        });

        // 处理退出信号
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context, pool, &server](const boost::system::error_code&, int) {
            io_context.stop();
            pool->Stop();
            server->Shutdown();
        });

        // 注册给LogicSystem方便管理连接
        LogicSystem::GetInstance()->SetServer(pointer_server);
        io_context.run();

        grpc_server_thread.join();

        pointer_server->StopTimer();
        return 0;
    }
    catch (std::exception& e) {
        LOG(ERROR) << "Exception: " << e.what();
    }
}
