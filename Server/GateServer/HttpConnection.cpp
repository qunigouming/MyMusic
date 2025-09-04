#include "HttpConnection.h"
#include "LogicSystem.h"

HttpConnection::HttpConnection(boost::asio::io_context& ioc) : _socket(ioc)
{

}

void HttpConnection::Start()
{
	auto self = shared_from_this();
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t byte_transferred) {
		try {
			if (ec) {
				std::cout << "http read error is " << ec.what() << std::endl;
				return;
			}

			boost::ignore_unused(byte_transferred);
			self->HandleRequest();
			self->CheckDeadline();			// 开始检查超时
		}
		catch (std::exception& exp) {
			std::cout << "exception is " << exp.what() << std::endl;
		}
	});
}

void HttpConnection::CheckDeadline()
{
	auto self = shared_from_this();
	deadline_.async_wait([self](beast::error_code ec) {
		if (!ec)	self->_socket.close(ec);
	});
}

void HttpConnection::HandleRequest()
{
	_response.version(_request.version());
	_response.keep_alive(false);			//设置为短连接
	//处理Get请求
	if (_request.method() == http::verb::get) {
		PreParseGetParam();
		bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());
		//错误处理
		if (!success) {
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/plain");
			beast::ostream(_response.body()) << "url not found\r\n";
			WriteResponse();
			return;
		}
		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");
		WriteResponse();
		return;
	}

	//处理Post请求
	if (_request.method() == http::verb::post) {
		bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());
		//错误处理
		if (!success) {
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/plain");
			beast::ostream(_response.body()) << "url not found\r\n";
			WriteResponse();
			return;
		}
		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");
		WriteResponse();
		return;
	}

}

void HttpConnection::WriteResponse()
{
	auto self = shared_from_this();
	_response.content_length(_response.body().size());
	http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t byte_transferred) {
		self->_socket.shutdown(tcp::socket::shutdown_send, ec);
		self->deadline_.cancel();			//重置等待
	});
}

unsigned char ToHex(unsigned char x)
{
	return x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z')	y = x - 'a' + 10;
	else if (x >= '0' && x <= '9')	y = x - '0';
	else assert(0);
	return y;
}

std::string UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; ++i) {
		if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') || (str[i] == '.') || (str[i] == '~'))		//数字或字母组成
			strTemp += str[i];
		else if (str[i] == ' ')		//空字符
			strTemp += '+';
		else {
			//其他字符需加%且高四位与低四位分别转为16进制
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] & 0x0F);
		}
	}
	return strTemp;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; ++i) {
		if (str[i] == '+')	strTemp += ' ';
		else if (str[i] == '%') {
			assert(i + 2 < length);
			unsigned char hight = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += hight * 16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}

void HttpConnection::PreParseGetParam()
{
	auto url = _request.target();
	//查找参数开始的位置
	auto query_pos = url.find('?');
	if (query_pos == std::string::npos) {
		_get_url = url;
		return;
	}

	_get_url = url.substr(0, query_pos);
	std::string query_string = url.substr(query_pos + 1);
	std::string key;
	std::string value;
	size_t pos = 0;
	while ((pos = query_string.find('&')) != std::string::npos) {
		auto pair = query_string.substr(0, pos);
		size_t eq_pos = pair.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
			value = UrlDecode(pair.substr(eq_pos + 1));
			_get_params[key] = value;
		}
		query_string.erase(0, pos + 1);
	}
	// 处理最后一个参数对（如果没有 & 分隔符）  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(query_string.substr(0, eq_pos));
			value = UrlDecode(query_string.substr(eq_pos + 1));
			_get_params[key] = value;
		}
	}
}
