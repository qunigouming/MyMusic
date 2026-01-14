#pragma once

#include <string>
#pragma warning(disable:4996)
#define GLOG_USE_GLOG_EXPORT        // 使用动态库
#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_EXPORT __declspec(dllimport)

#include <glog/logging.h>

class LogManager
{
public:
    static void InitGlog(const char* argv, std::string logPath = "./logs");     // 初始化Glog（只执行一次）
private:
    LogManager() = default;
    ~LogManager();
};