#include "Session.h"
#include "Server.h"
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "LogicSystem.h"
#include "RedisManager.h"

Session::Session(boost::asio::io_context& ioc, Server* server) : _socket(ioc), _server(server)
{
	boost::uuids::uuid uid = boost::uuids::random_generator()();
	_session_id = boost::uuids::to_string(uid);
	_recv_head_node = std::make_shared<MessageNode>(HEAD_TOTAL_LEN);
	_last_heartbeat = std::time(nullptr);
}

Session::~Session()
{
	std::cout << "Session destruct" << std::endl;
}

tcp::socket& Session::GetSocket()
{
	return _socket;
}

std::string& Session::GetUid()
{
	return _session_id;
}

void Session::SetUserUid(int uid)
{
	_user_uid = uid;
}

void Session::Start()
{
	AsyncReadHead(HEAD_TOTAL_LEN);
}

void Session::Send(char* msg, short max_len, short msg_id)
{
	std::lock_guard<std::mutex> lock(_send_mutex);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SEND_QUEUE) {
		std::cout << "session: " << _session_id << " send queue fulled, size is " << MAX_SEND_QUEUE << std::endl;
		return;
	}
	_send_que.push(std::make_shared<SendNode>(msg, max_len, msg_id));
	//if send queue size less then 0 that will send message by socket.
	if (send_que_size > 0) return;
	auto& msgNode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgNode->_data, msgNode->_total_len),
		std::bind(&Session::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void Session::Send(std::string msg, short msg_id)
{
	std::lock_guard<std::mutex> lock(_send_mutex);
	int send_que_size = _send_que.size();
	//dispose queue overlength
	if (send_que_size > MAX_SEND_QUEUE) {
		std::cout << "session: " << _session_id << " send queue fulled, size is " << MAX_SEND_QUEUE << std::endl;
		return;
	}
	_send_que.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msg_id));
	//if send queue size less then 0 that will send message by socket.
	if (send_que_size > 0) return;
	auto& msgNode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgNode->_data, msgNode->_total_len),
		std::bind(&Session::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void Session::Close()
{
	_socket.close();
	_b_close = true;
}

void Session::AsyncReadHead(int total_len)
{
	auto self = shared_from_this();
	AsyncReadFull(HEAD_TOTAL_LEN, [self](const boost::system::error_code& error, std::size_t byte_transfered) {
		try {
			if (error) {
				std::cout << "handle read failed, error is: " << error.what() << std::endl;
				self->Close();
				self->DealExceptionSession();
				return;
			}
			if (byte_transfered < HEAD_TOTAL_LEN) {
				std::cout << "read length not match, read [" << byte_transfered << "], total [" << HEAD_TOTAL_LEN << "]" << std::endl;
				self->Close();
				self->_server->ClearSession(self->_session_id);
				return;
			}

			if (!self->_server->CheckVaild(self->_session_id)) {
				self->Close();
				return;
			}

			// 拷贝头部数据
			self->_recv_head_node->Clear();
			memcpy(self->_recv_head_node->_data, self->_data, byte_transfered);
			short msg_id = 0;
			// 拷贝MsgId
			memcpy(&msg_id, self->_recv_head_node->_data, HEAD_ID_LEN);
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << std::endl;
			if (msg_id > MAX_LENGTH) {
				std::cout << "invalid msg_id is " << msg_id << std::endl;
				self->_server->ClearSession(self->_session_id);
				return;
			}
			// 拷贝消息长度
			unsigned short msg_len = 0;
			memcpy(&msg_len, self->_recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << std::endl;

			if (msg_len > MAX_LENGTH) {
				std::cout << "invalid msg_len is" << msg_len << std::endl;
				self->_server->ClearSession(self->_session_id);
				return;
			}
			self->_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
			self->AsyncReadBody(msg_len);
		}
		catch (std::exception& exp) {
			std::cout << "Exception code is: " << exp.what() << std::endl;
		}
	});
}

void Session::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();
	AsyncReadFull(total_len, [self, total_len](const boost::system::error_code& error, std::size_t bytes_transfered) {
		try {
			if (error) {
				std::cout << "handle read failed, error is " << error.what() << std::endl;
				self->Close();
				self->DealExceptionSession();
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "read length not match, read [" << bytes_transfered << "], total [" << total_len << "]" << std::endl;
				self->Close();
				self->_server->ClearSession(self->_session_id);
				return;
			}

			if (!self->_server->CheckVaild(self->_session_id)) {
				self->Close();
				return;
			}

			memcpy(self->_recv_msg_node->_data, self->_data, bytes_transfered);
			self->_recv_msg_node->_current_len += bytes_transfered;
			self->_recv_msg_node->_data[self->_recv_msg_node->_total_len] = '\0';
			std::cout << "receive data is " << self->_recv_msg_node->_data << std::endl;

			self->UpdateHeartbeat();
			//此处将消息投递到逻辑队列中
			LogicSystem::GetInstance()->PustMsg(std::make_shared<LogicNode>(self,self->_recv_msg_node));
			//继续监听头部接受事件
			self->AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& exp) {
			std::cout << "Read body error is:" << exp.what() << std::endl;
		}
	});
}

void Session::NotifyOffline(int uid)
{
	Json::Value retValue;
	retValue["error"] = ErrorCodes::Success;
	retValue["uid"] = uid;

	std::string return_str = retValue.toStyledString();
	Send(return_str, ID_NOTIFY_OFF_LINE_REQ);
	return;
}

bool Session::IsHeartbeatExpired(std::time_t& now)
{
	double diff_sec = std::difftime(now, _last_heartbeat);
	if (diff_sec > HEARTBEAT_EXPIRE_TIME) {
		std::cout << "Heartbeat expired, session id is " << _session_id << std::endl;
		return true;
	}
	return false;
}

void Session::UpdateHeartbeat()
{
	time_t now = std::time(nullptr);
	_last_heartbeat = now;
}

void Session::DealExceptionSession()
{
	auto self = shared_from_this();
	auto uid_str = std::to_string(_user_uid);
	auto lock_key = LOCK_PREFIX + uid_str;
	auto identifier = RedisManager::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	Defer defer([this, self, identifier, lock_key]() {
		_server->ClearSession(_session_id);
		RedisManager::GetInstance()->releaseLock(lock_key, identifier);
	});

	if (identifier.empty()) return;
	std::string redis_session_id = "";
	bool success = RedisManager::GetInstance()->Get(USER_SESSION_PREFIX + uid_str, redis_session_id);
	if (!success) return;

	if (redis_session_id != _session_id) {
		// 有客户在其他服务器登录
		return;
	}

	RedisManager::GetInstance()->Del(USER_SESSION_PREFIX + uid_str);
    RedisManager::GetInstance()->Del(USERTOKENPREFIX + uid_str);
}

void Session::AsyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	memset(_data, 0, MAX_LENGTH);		//eliminate buffer space
	AsyncReadLen(0, maxLength, handler);
}

void Session::AsyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len),
		[read_len, total_len, handler, self](const boost::system::error_code& error, std::size_t byte_transfered) {
		if (error) {
			handler(error, read_len + byte_transfered);
			return;
		}
		//reception data complete
		if (read_len + byte_transfered >= total_len) {
			handler(error, read_len + byte_transfered);
			return;
		}
		//dispose remaining data
		self->AsyncReadLen(read_len + byte_transfered, total_len, handler);
	});
}

void Session::HandleWrite(const boost::system::error_code& error, std::shared_ptr<Session> shared_self)
{
	try {
		auto self = shared_from_this();
		if (!error) {
			std::lock_guard<std::mutex> lock(_send_mutex);
			_send_que.pop();
			if (!_send_que.empty()) {
				auto& msgNode = _send_que.front();
				boost::asio::async_write(_socket, boost::asio::buffer(msgNode->_data, msgNode->_total_len),
					std::bind(&Session::HandleWrite, this, std::placeholders::_1, shared_self));
			}
		}
		else {
			std::cout << "Handle write failed, Error code is: " << error.what() << std::endl;
			Close();
			DealExceptionSession();
		}
	}
	catch (std::exception& exp) {
		std::cout << "Exception code is: " << exp.what() << std::endl;
	}
}
