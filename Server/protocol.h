#ifndef PROTOCOL_H
#define PROTOCOL_H


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

enum class REGISTTYPE{
    HAVEUSER,                   //拥有用户
    EQUAL_USERNAME,             //用户名相同
    OK                          //完毕
};

enum class LOGINTYPE{
    NOTUSER,                    //没有该用户
    LOGIN,                      //用户在线
    OK                          //完毕
};

#endif // PROTOCOL_H
