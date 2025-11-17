# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

set(CMAKE_TOOLCHAIN_FILE "/home/mian/GitProject/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE PATH "Vcpkg toolchain file")

project ("StorageServer" VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
#find_package(PkgConfig REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

#pkg_check_modules(GLOG REQUIRED libglog)

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
)
