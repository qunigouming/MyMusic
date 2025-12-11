# CMakeList.txt: StorageServer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 将源代码添加到此项目的可执行文件。
add_executable (StorageServer "main.cpp"
	"Common/ConfigManager.cpp"
	"Common/ConfigManager.h"
	"Common/LogManager.cpp"
	"Common/LogManager.h"
	"Common/Singleton.h"
	"gRPC/message.grpc.pb.cc"
    "gRPC/message.grpc.pb.h"
    "gRPC/message.pb.cc"
    "gRPC/message.pb.h"
 "gRPC/StorageServiceImpl.cpp" "Common/FdfsClient.cpp" "Common/FdfsClient.h" "Common/AsioIOServicePool.h" "Common/AsioIOServicePool.cpp" "Common/global.h"
 "config.ini"
 )

# TODO: 如有需要，请添加测试并安装目标。
