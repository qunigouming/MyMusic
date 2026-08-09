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
- **文件存储**: FastDFS (封面图片) + 本地磁盘 (音频文件)

## 核心功能

### 客户端功能
- **用户系统**: 登录、注册、邮箱验证码验证、密码找回
- **本地音乐播放**: 基于 FFmpeg 实现音频解码，支持多种音频格式
- **在线音乐播放**: 从服务器获取音乐资源并实时播放
- **歌单管理**: 收藏歌曲、创建歌单、编辑歌单信息
- **本地音乐扫描**: 多线程扫描 + taglib 解析元数据 (标题、歌手、专辑、时长等)
- **自定义音乐列表**: 重写 QTableView/Model/Delegate/ProxyModel，delegate 自绘渲染（图标、文本、悬停功能按钮，无 widget 编辑器），支持排序与批量加载
- **AI 聊天功能**: 集成 AI 对话能力，支持流式输出
- **音频效果**: 均衡器调节、音频效果配置
- **界面定制**: 皮肤颜色自定义
- **系统集成**: 窗口拖拽、最小化/最大化/关闭、系统托盘

### 服务器端服务
| 服务 | 端口 | 功能 |
|------|------|------|
| GateServer | HTTP 8080 | API 网关，HTTP/WebSocket 端点，请求路由 |
| SessionServer | TCP 8090 / gRPC 50055 | 用户会话管理、登录/登出、心跳、歌单操作、文件上传（多实例时每实例端口不同） |
| VerifyServer | gRPC 50051 | 邮箱验证码生成与验证、用户注册验证 |
| StatusServer | gRPC 50052 | 系统监控、服务健康检查、登录计数、SessionServer 分配 |
| StorageServer | gRPC 50100 | 文件存储、图片流式上传（对接 FastDFS） |

### 会话认证
- **双 token 机制**: 登录前由 GateServer 签发一次性握手 token（防重放，登录校验后即焚）；登录成功后服务器签发会话 token（Redis `utoken_<uid>`，TTL 30 分钟）
- **全链路校验**: 心跳、收藏、上传、歌单等所有请求均携带会话 token，服务器逐 handler 校验，失效返回 `TokenInvalid(1010)`
- **心跳续期**: 客户端每 10 秒心跳，服务器校验通过后刷新 token 过期时间；断线/超时清理时删除 token

## 目录结构

```
MyMusic/
├── MyMusic/                     # Qt 客户端
│   ├── MyMusic/                # 客户端源代码
│   │   ├── tableview/          # 自定义表格视图组件（delegate 自绘）
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
│   └── resoure/conf/           # 部署配置模板
│       ├── nginx/              # nginx 配置 (mymusic.conf / music.conf)
│       └── fastdfs/            # FastDFS 配置 (tracker/storage/client)
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
    │                         ├─► SessionServer (TCP 8090 + gRPC 50055)
    │                         ├─► VerifyServer (gRPC 50051)
    │                         ├─► StatusServer (gRPC 50052)
    │                         └─► StorageServer (gRPC 50100)
    │                              │
    ▼                              ▼
TCP 长连接                    MySQL / Redis / FastDFS
```

## 部署架构

### 主机名映射方案

所有配置文件使用**主机名**而非 IP，IP 变更时只需修改各机器的 `hosts` 文件，配置与数据库无需改动。

| 主机名 | 指向 | 说明 |
|--------|------|------|
| `gate.local` | 本机 | GateServer (HTTP 8080) |
| `session1.local` / `session2.local` | 各 SessionServer 机器 | SessionServer 实例 |
| `status.local` / `verify.local` | 本机 | StatusServer / VerifyServer |
| `mysql.local` / `redis.local` | 本机 | MySQL / Redis |
| `storage.local` | Linux 机器 | FastDFS + nginx（音乐与封面） |

Windows hosts 示例（所有服务在同一台 Windows 机器时，本机服务用 `127.0.0.1` 最稳）：

```
127.0.0.1       gate.local
127.0.0.1       session1.local
127.0.0.1       status.local
127.0.0.1       verify.local
127.0.0.1       mysql.local
127.0.0.1       redis.local
192.168.186.128 storage.local
```

### 混合部署拓扑

- **Windows 机器**：运行 GateServer、SessionServer、VerifyServer、StatusServer、StorageServer、MySQL、Redis
- **Linux 机器**：运行 FastDFS（tracker + storage）与 nginx，对外提供音乐/封面文件的 HTTP 访问

### nginx 职责（配置见 `Server/resoure/conf/nginx/`）

- **Linux `mymusic.conf`**（当前生效）：
  - `location /audio/` → 音乐文件（`alias /home/main/music/`）
  - `location /group1/M00` → FastDFS 封面图片（`ngx_fastdfs_module`）
  - RTMP 1935（直播流，预留）
- **Windows `music.conf`**：旧版本遗留，当前无任何请求命中，可停用删除

### 多 SessionServer 实例

- **StatusServer** 通过 `[SessionServers]` 逗号列表 + `[SessionServerN]` 段维护实例表，`/get_server` 时分配一个给客户端（当前固定返回第一个，负载均衡逻辑已预留注释）
- **SessionServer** 通过 `[PeerServer]` 配置对端实例，跨服务器踢下线走 gRPC
- Redis 中 `uip_<uid>` 按实例名（`sessionserver1` 等）记录用户所在服务器，**不依赖 IP**
- 每实例的 TCP 端口（客户端连接）与 RPC 端口（互踢）必须唯一

### 换 IP 的迁移步骤

1. 修改各机器 `hosts` 文件并同步分发
2. 重启所有服务（gRPC channel 与连接池在启动时建立）
3. 数据库 URL 替换（见「数据库 URL 存储」）
4. FastDFS 侧：修改 `bind_addr` / `tracker_server` / nginx 配置（配置模板见 `Server/resoure/conf/fastdfs/`）

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
host=gate.local
port=8080
```

GateServer 配置 (`Server/GateServer/config.ini`):
```ini
[GateServer]
Port=8080
[VerifyServer]
Host=verify.local
Port=50051
[StatusServer]
Host=status.local
Port=50052
[Mysql]
Host=mysql.local
Port=3306
User=root
Passwd=123
Schema=mymusic
[Redis]
Host=redis.local
Port=6380
Passwd=123456
```

SessionServer 配置 (`Server/SessionServer/config.ini`，多实例示例):
```ini
[StatusServer]
Host=status.local
Port=50052
[SelfServer]
Name=sessionserver1            ; 实例唯一标识（Redis 中按此记录）
Host=0.0.0.0
Port=8090                      ; 客户端 TCP 端口，每实例不同
RPCPort=50055                  ; 互踢 RPC 端口，每实例不同
[PeerServer]
Servers=sessionserver2         ; 只列对端实例
[sessionserver2]
Name=sessionserver2
Host=session2.local            ; 对端用主机名
Port=50056                     ; 对端的 RPCPort
[StorageServer]
Host=storage.local
Port=50100
[Mysql]
Host=mysql.local
Port=3306
User=root
Passwd=123
Schema=MyMusic
[Redis]
Host=redis.local
Port=6380
Passwd=123456
[Store]
CoverPath=D:/Procedure/COS/Picture/
RemotePath=http://storage.local/audio/    ; 音频 URL 前缀，会写入 song.file_url
MusicPath=D:/Procedure/COS/Music/         ; 音频文件落盘目录（需同步到 Linux /home/main/music/）
```

StatusServer 配置 (`Server/StatusServer/config.ini`):
```ini
[StatusServer]
Host=0.0.0.0
Port=50052
[Redis]
Host=redis.local
Port=6380
Passwd=123456
[SessionServers]               ; 实例列表（已由 ChatServer 改名）
Name=SessionServer1,SessionServer2
[SessionServer1]
Name=SessionServer1
Host=session1.local
Port=8090
[SessionServer2]
Name=SessionServer2
Host=session2.local
Port=50056
```

## 安全设计

- **一次性握手 token**：登录前 GateServer 签发（UUID），存入 Redis `utoken_<uid>`，登录时校验后立即删除，防止重放攻击
- **会话 token**：登录成功后签发，Redis `utoken_<uid>`（TTL 30 分钟），客户端所有请求携带，服务器各 handler 校验
- **心跳续期**：心跳校验通过后刷新 TTL；TCP 断线（60 秒无消息）清理时同步删除 token
- **连接绑定**：TCP 登录后 session 与 uid 绑定，请求中的 uid 与会话不一致即拒绝（防伪造他人身份）

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
