#include "LogManager.h"
#include <mutex>
#include <filesystem>
#include <iostream>

void LogManager::InitGlog(const char* argv, std::string logPath)
{
    static std::once_flag flag;
    std::call_once(flag, [&] {
        // 检查路径是否存在
        std::error_code ec;
        if (!std::filesystem::exists(logPath)) {
            if (!std::filesystem::create_directories(logPath, ec)) {
                std::cerr << "Create directories failed: " << logPath
                    << " Error: " << ec.message() << std::endl;
                return;
            }
        }
        google::InitGoogleLogging(argv);
        // 在程序启动时删除旧日志
        FLAGS_logtostderr = false;
        FLAGS_alsologtostderr = true;               // 输出到stderr
        FLAGS_colorlogtostderr = true;

        // Use SetLogDestination instead of FLAGS_log_dir
        std::string infoPrefix = logPath + "\\MusicApp_INFO_";
        std::string warnPrefix = logPath + "\\MusicApp_WARN_";
        std::string errorPrefix = logPath + "\\MusicApp_ERROR_";
        std::string fatalPrefix = logPath + "\\MusicApp_FATAL_";

        google::SetLogDestination(google::GLOG_INFO, infoPrefix.c_str());
        google::SetLogDestination(google::GLOG_WARNING, warnPrefix.c_str());
        google::SetLogDestination(google::GLOG_ERROR, errorPrefix.c_str());
        google::SetLogDestination(google::GLOG_FATAL, fatalPrefix.c_str());
        FLAGS_max_log_size = 100;                   // 每个日志文件最大100MB
        FLAGS_stop_logging_if_full_disk = true;     // 磁盘空间不足时停止日志记录
        
        google::EnableLogCleaner(30);                // 保留30天内的日志

        std::atexit(google::ShutdownGoogleLogging);
        LOG(INFO) << "===========================================";
        LOG(INFO) << "   Log System Initialized Successfully     ";
        LOG(INFO) << "===========================================";
    });
}

LogManager::~LogManager()
{
	// google::ShutdownGoogleLogging();
}
