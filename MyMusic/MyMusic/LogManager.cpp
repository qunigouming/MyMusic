#include "LogManager.h"
#include <mutex>

void LogManager::InitGlog(const char* argv, std::string logPath)
{
    static std::once_flag flag;
    std::call_once(flag, [&] {
        google::InitGoogleLogging(argv);

        // 在程序启动时删除旧日志
        FLAGS_logtostderr = false;
        FLAGS_alsologtostderr = false;
        FLAGS_log_dir = logPath;
        google::EnableLogCleaner(3);
    });
}

LogManager::~LogManager()
{
	google::ShutdownGoogleLogging();
}
