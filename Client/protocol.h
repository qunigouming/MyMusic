#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PASSWORDMINSIZE    4           //密码最短长度

//数据传输枚举
enum class TcpMSGType{
    REGIST_REQUEST,             //注册请求
    REGIST_REPLY,               //注册回复
    LOGIN_REQUEST,              //登录请求
    LOGIN_REPLY,                //登录回复
    MUSICOPEN_REQUEST,          //音乐播放请求
    MUSICOPEN_REPLY,            //音乐播放回复
    MUSICLIST_REQUEST,          //音乐列表请求
    MUSICLIST_REPLY             //音乐列表回复
};

//注册判断枚举
enum class REGISTTYPE{
    HAVEUSER,                   //拥有用户
    EQUAL_USERNAME,             //用户名相同
    OK                          //完毕
};

//登录判断枚举
enum class LOGINTYPE{
    NOTUSER,                    //没有该用户
    LOGIN,                      //用户在线
    OK                          //完毕
};

//列表播放模式
enum class LISTPLAYER{
    LOOP,                       //列表循环
    SINGLELOOP,                 //单曲循环
    RANDOM                      //随机播放
};

#endif // PROTOCOL_H
