# 介绍
1. 一个分布式服务器的音乐播放器项目，服务器仿照llfcchat，客户端已实现登录功能，注册功能，播放本地音乐功能。
2. 音乐列表通过重写QTableView，model, delegate, proxymodel,headview来显示期望的样式。
3. 本地音乐使用一个多线程文件扫描工具来扫描指定目录下的所有.mp3格式的媒体文件，封装使用taglib来获取mp3文件的tag。
4. 播放器基于FFmpeg来播放音乐，输出用Qt的IODevice来输出声音(后续可能会添加视频解码功能)。
