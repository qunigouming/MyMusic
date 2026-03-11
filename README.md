# MyMusic 分布式音乐流媒体系统

## 项目概述

MyMusic 是一款基于分布式服务器架构的音乐播放系统，参考网易云音乐界面设计。系统采用客户端-服务器架构，客户端使用 Qt 框架开发，服务器端由多个通过 gRPC 通信的 C++ 微服务组成。

## 技术栈

### 客户端
- **UI 框架**: Qt 6.5.3 + MSVC 2019
- **多媒体处理**: FFmpeg (音频解码)、taglib (MP3 元数据解析)
- **网络通信**: HTTP (登录注册)、TCP (实时数据传输)、WebSocket
- **依赖管理**: vcpkg

### 服务器端
- **服务框架**: C++ 分布式微服务
- **通信协议**: gRPC (服务间通信)、HTTP/WebSocket (客户端接入)
- **数据存储**: MySQL + Redis
- **异步 I/O**: Boost.Asio

## 核心功能

### 客户端功能
- **用户系统**: 登录、注册、邮箱验证码验证、密码找回
- **本地音乐播放**: 基于 FFmpeg 实现音频解码，支持多种音频格式
- **在线音乐播放**: 从服务器获取音乐资源并实时播放
- **歌单管理**: 收藏歌曲、创建歌单、编辑歌单信息
- **本地音乐扫描**: 多线程扫描 + taglib 解析元数据 (标题、歌手、专辑、时长等)
- **自定义音乐列表**: 重写 QTableView、Model、Delegate、ProxyModel 实现个性化展示
- **AI 聊天功能**: 集成 AI 对话能力，支持流式输出
- **音频效果**: 均衡器调节、音频效果配置
- **界面定制**: 皮肤颜色自定义
- **系统集成**: 窗口拖拽、最小化/最大化/关闭、系统托盘

### 服务器端服务
| 服务 | 端口 | 功能 |
|------|------|------|
| GateServer | 8080 | API 网关，HTTP/WebSocket 端点，请求路由 |
| SessionServer | - | 用户会话管理、登录/登出、心跳、歌单操作 |
| VerifyServer | - | 邮箱验证码生成与验证、用户注册验证 |
| StatusServer | - | 系统监控、服务健康检查、登录计数 |
| StorageServer | - | 文件存储、图片/音频流式上传 |

## 目录结构

```
MyMusic/
├── MyMusic/                     # Qt 客户端
│   ├── MyMusic/                # 客户端源代码
│   │   ├── tableview/          # 自定义表格视图组件
│   │   ├── FFPlayer/          # FFmpeg 播放器封装
│   │   ├── Tool/               # 工具类
│   │   ├── source/             # 资源文件 (图片、字体)
│   │   └── *.cpp/*.h           # 主要业务代码
│   └── MyMusic.sln
├── Server/                     # 服务器端
│   ├── GateServer/             # API 网关
│   ├── SessionServer/           # 会话服务
│   ├── VerifyServer/           # 验证服务
│   ├── StatusServer/            # 状态服务
│   ├── StorageServer/          # 存储服务
│   └── *.sql                    # 数据库脚本
├── vcpkg_installed/             # vcpkg 依赖
├── CLAUDE.md                   # 开发指南
├── MessageDoc.md               # API 消息格式文档
└── README.md
```

## 架构图

```
客户端 (Qt)
    │
    ├─ HTTP/WebSocket ──► GateServer (8080)
    │                         │
    │                         ├─► SessionServer (gRPC)
    │                         ├─► VerifyServer (gRPC)
    │                         ├─► StatusServer (gRPC)
    │                         └─► StorageServer (gRPC)
    │                              │
    ▼                              ▼
TCP 长连接                    MySQL / Redis
```

## 构建说明

### 客户端 (Qt 6.5.3 + MSVC 2019)

```bash
# 使用 Visual Studio 打开
cd MyMusic/MyMusic
MyMusic.sln

# 或使用 CMake
cmake -B build -S . -G "Visual Studio 16 2019"
cmake --build build --config Release
```

### 服务器端

使用 Visual Studio 2019 打开相应的解决方案文件：
- `Server/GateServer/GateServer.sln`
- `Server/SessionServer/SessionServer.sln`
- `Server/VerifyServer/VerifyServer.sln`
- `Server/StatusServer/StatusServer.sln`
- `Server/StorageServer/StorageServer.sln`

### 依赖安装

项目使用 vcpkg 管理部分依赖：
```bash
vcpkg install
```

需额外安装：Boost、gRPC、MySQL Connector、libjson

## 配置说明

客户端配置 (`MyMusic/MyMusic/MyMusic/config.ini`):
```ini
[GateServer]
host=127.0.0.1
port=8080
```

GateServer 配置 (`Server/GateServer/config.ini`):
```ini
[GateServer]
Port=8080
[VerifyServer]
Host=192.168.10.150
Port=50051
[StatusServer]
Host=192.168.10.150
Port=50052
[Mysql]
Host=127.0.0.1
Port=3306
Schema=mymusic
[Redis]
Host=127.0.0.1
Port=6380
```

## 关键设计模式

- **单例模式**: ConfigManager、RedisManager、MysqlManager
- **线程池**: AsioIOServicePool 用于连接处理
- **gRPC 流**: 用于文件分片上传
- **信号/槽**: Qt 事件处理机制

## 协议文档

详细的 API 消息格式见 `MessageDoc.md`，包括：
- LOGIN_USER_REQ: 用户登录
- ID_HEARTBEAT_REQ: 心跳保活
- ID_UPLOAD_FILE_REQ: 文件上传
- ID_COLLECT_SONG_REQ: 歌曲收藏
- ID_GET_COLLECT_SONG_LIST_REQ: 获取歌单