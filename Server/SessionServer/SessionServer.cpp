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

bool b_stop = false;
std::condition_variable cond_quit;
std::mutex mtx_quit;

int main()
{
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

        // 处理退出信号
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context, pool](const boost::system::error_code&, int) {
            io_context.stop();
            pool->Stop();
        });

        io_context.run();

        pointer_server->StopTimer();
        return 0;
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
