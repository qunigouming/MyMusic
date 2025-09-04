#pragma once
#include <iostream>
#include <json/value.h>
#include <json/reader.h>
#include <json/json.h>
#include <functional>
#include <boost\asio.hpp>
#include <memory>
#include <mutex>

//data define length
#define MAX_LENGTH 1024 * 1024 * 2
#define HEAD_TOTAL_LEN 4				//head data total length
#define HEAD_ID_LEN 2					//head's id data length
#define HEAD_DATA_LEN 2					//head's data length
#define MAX_RECV_QUEUE 1024*1024				//maximum receive length
#define MAX_SEND_QUEUE 1024					//maximum send length

//custom prefix string
#define USERTOKENPREFIX "utoken_"
#define LOGINCOUNT		"logincount"
#define USER_BASE_INFO	"ubaseinfo_"
#define USER_SESSION_PREFIX "usession_"
#define LOCK_PREFIX "lock_"
#define LOCK_COUNT "lockcount"

// �ֲ�ʽ���ĳ���ʱ��
#define LOCK_TIME_OUT 10
// �ֲ�ʽ��������ʱ��
#define ACQUIRE_TIME_OUT 5

enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,  //Json��������
	RPCFailed = 1002,  //RPC�������
	VarifyExpired = 1003, //��֤�����
	VarifyCodeErr = 1004, //��֤�����
	UserExist = 1005,       //�û��Ѿ�����
	PasswdErr = 1006,    //�������
	EmailNotMatch = 1007,  //���䲻ƥ��
	PasswdUpFailed = 1008,  //��������ʧ��
	PasswdInvalid = 1009,   //�������ʧ��
	TokenInvalid = 1010,   //TokenʧЧ
	UidInvalid = 1011,  //uid��Ч
	EtherInvalid = 0x7777,  //Ether����
};

class Defer {
public:
	Defer(std::function<void()> func) : _func(func) {}

	~Defer() {
		_func();
	}

private:
	std::function<void()> _func;
};

//User request message id
enum MESSAGE_REQ_IDS {
	MSG_USER_LOGIN_REQ = 0x2001,	//User login
    ID_HEARTBEAT_REQ = 0x2002,		// Heartbeat request
};

//User response message id
enum MSSAGE_RSP_IDS {
	MSG_USER_LOGIN_RSP = 0x5001,		//User login backsourcing
	ID_HEARTBEAT_RSP = 0x5002,			// Heartbeat response
};