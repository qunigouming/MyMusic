#pragma once
#include <string>
#include <boost\asio.hpp>
#include "global.h"

class LogicSystem;

class MessageNode
{
public:
	MessageNode(unsigned short len) : _total_len(len) {
		_data = new char[_total_len + 1];
		_data[_total_len] = '\0';
	}

	~MessageNode() {
		std::cout << "destory MessageNode" << std::endl;
		delete[] _data;
	}

	void Clear() {
		memset(_data, 0, _total_len);
		_current_len = 0;
	}

	unsigned short _current_len = 0;
	unsigned short _total_len;
	char* _data;
};

class RecvNode : public MessageNode {
	friend class LogicSystem;
public:
	RecvNode(unsigned short max_len, short msg_id) : MessageNode(max_len), _msg_id(msg_id) {}
private:
	short _msg_id;
};

class SendNode : public MessageNode {
	friend class LogicSystem;
public:
	SendNode(const char* msg, short max_len, short msg_id) : MessageNode(max_len + HEAD_TOTAL_LEN), _msg_id(msg_id) {
		short msg_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
		memcpy(_data, &msg_id_host, HEAD_ID_LEN);							//copy message id(type) to node;
		short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
		memcpy(_data + HEAD_ID_LEN, &max_len_host, HEAD_DATA_LEN);			//copy message length to node;
		memcpy(_data + HEAD_TOTAL_LEN, msg, max_len);			//copy message to node
	}
private:
	short _msg_id;
};

