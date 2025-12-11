#include "LogManager.h"
#include <mutex>
#include <filesystem>
#include <iostream>

void LogManager::InitGlog(const char* argv, std::string logPath)
{
    static std::once_flag flag;
	std::call_once(flag, [&] {
		// 检查路径是否存在
		if (!std::filesystem::exists(logPath)) {
			if (std::filesystem::create_directories(logPath)) {
				std::cerr << "Create directories failed: " << logPath;
			}
		}

		google::InitGoogleLogging(argv);

		// 在程序启动时删除旧日志
		FLAGS_logtostderr = false;
		FLAGS_alsologtostderr = true;               // 输出到stderr
		FLAGS_log_dir = logPath;
		FLAGS_max_log_size = 100;                   // 每个日志文件最大100MB
		FLAGS_stop_logging_if_full_disk = true;     // 磁盘空间不足时停止日志记录
		google::EnableLogCleaner(3);                // 保留3天内的日志
	});
}

LogManager::~LogManager()
{
	google::ShutdownGoogleLogging();
}
