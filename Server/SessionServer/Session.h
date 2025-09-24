#pragma once
#include "global.h"
#include <boost\asio.hpp>
#include <boost/beast.hpp>
#include <boost\beast\http.hpp>
#include <queue>
#include "MessageNode.h"


#define HEARTBEAT_EXPIRE_TIME 60
#define TIMER_INTERVAL 60

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Server;
class LogicSystem;

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* server);
	~Session();
	tcp::socket& GetSocket();
	std::string& GetSessionId();
	void SetUserUid(int uid);
	int GetUserUid();
	void Start();
	void Send(char* msg, short max_len, short msg_id);
	void Send(std::string msg, short msg_id);
	void Close();
	void AsyncReadHead(int total_len);
	void AsyncReadBody(int total_len);
	void NotifyOffline(int uid);
	bool IsHeartbeatExpired(std::time_t& now);
	void UpdateHeartbeat();
	void DealExceptionSession();

private:
	//read maxlength to node
	void AsyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler);
	void AsyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t)> handler);
	void HandleWrite(const boost::system::error_code& error, std::shared_ptr<Session> self);
	bool _b_close = false;
	tcp::socket _socket;
	std::string _session_id;
	char _data[MAX_LENGTH];
	Server* _server;
	std::queue<std::shared_ptr<SendNode>> _send_que;
	std::mutex _send_mutex;
	std::shared_ptr<RecvNode> _recv_msg_node;
	bool _b_head_parse = false;
	std::shared_ptr<MessageNode> _recv_head_node;
	int _user_uid = 0;
	std::atomic<time_t> _last_heartbeat;
	std::mutex _session_mutex;
};

class LogicNode {
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> message): _session(session), _message(message) {}

private:
	std::shared_ptr<Session> _session;
	std::shared_ptr<RecvNode> _message;
};

