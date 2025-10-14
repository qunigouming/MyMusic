#include "LogManager.h"
#pragma warning(disable:4996)
#define GLOG_USE_GLOG_EXPORT        // 使用动态库
#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_EXPORT __declspec(dllimport)

#include <glog/logging.h>
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
