#pragma once

#include <string>

class LogManager
{
public:
    static void InitGlog(const char* argv, std::string logPath = "./logs");     // 初始化Glog（只执行一次）
private:
    LogManager() = default;
    ~LogManager();
};