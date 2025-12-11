# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

set(CMAKE_TOOLCHAIN_FILE "/home/mian/GitProject/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE PATH "Vcpkg toolchain file")

project ("StorageServer" VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)

find_package(Threads REQUIRED)

if(DEFINED ENV{MY_INSTALL_DIR})
    set(GRPC_ROOT "$ENV{MY_INSTALL_DIR}")
    message(STATUS "Using MY_INSTALL_DIR: ${GRPC_ROOT}")
else()
    set(GRPC_ROOT "/home/mian/.local")
    message(STATUS "Using default gRPC path: ${GRPC_ROOT}")
endif()

set(CMAKE_PREFIX_PATH "${GRPC_ROOT}")
set(protobuf_MODULE_COMPATIBLE ON)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(glog CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# 查找fastcommon库
find_library(FASTCOMMON_LIB NAMES fastcommon
    PATHS /usr/lib64 /usr/local/lib /usr/local/lib64
    NO_DEFAULT_PATH
)

# 查找fdfsclient库
find_library(FDFSCLIENT_LIB NAMES fdfsclient
    PATHS /usr/lib64 /usr/local/lib /usr/local/lib64
    NO_DEFAULT_PATH
)

# 查找头文件路径
find_path(FASTCOMMON_INCLUDE_DIR logger.h
    PATHS /usr/include/fastcommon
)

find_path(FDFSCLIENT_INCLUDE_DIR fdfs_client.h
    PATHS /usr/include/fastdfs
)

message(STATUS "work directory: ${CMAKE_SOURCE_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/StorageServer)

#if(NOT FASTCOMMON_LIB OR NOT FDFSCLIENT_LIB)
#    message(FATAL_ERROR "未找到FastDFS客户端库，请确保已安装fastcommon和libfdfsclient")
#endif()

#if(NOT FASTCOMMON_INCLUDE_DIR OR NOT FDFSCLIENT_INCLUDE_DIR)
#    message(FATAL_ERROR "未找到FastDFS头文件")
#endif()

include_directories(${FASTCOMMON_INCLUDE_DIR})
include_directories(${FDFSCLIENT_INCLUDE_DIR})

get_filename_component(ms_proto "./StorageServer/gRPC/message.proto" ABSOLUTE)

# 包含子项目。
add_subdirectory ("StorageServer")

target_include_directories("StorageServer" PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${GRPC_ROOT}/include         # 添加gRPC包含路径
    ${Protobuf_INCLUDE_DIRS}     # 添加Protobuf包含路径
    ${gRPC_INCLUDE_DIRS}         # 添加gRPC包含路径（如果定义了）
)

target_link_libraries("StorageServer" PRIVATE
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    glog::glog
    Boost::filesystem
    ${FDFSCLIENT_LIB} 
    ${FASTCOMMON_LIB}
    pthread
    dl
)
