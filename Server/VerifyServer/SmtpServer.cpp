#include "SmtpServer.h"
#include <iostream>
#include <functional>
#include <string>
#include <sstream>

SmtpServer::SmtpServer(short port) : acceptor_(_ioc_context, tcp::endpoint(tcp::v4(), port))
{
    start_accept();

}

void SmtpServer::Start()
{
    _ioc_context.run();
}

void SmtpServer::start_accept()
{
    auto new_session = std::make_shared<SmtpSession>(acceptor_.get_executor().context());
    acceptor_.async_accept(new_session->socket(),
        std::bind(&SmtpServer::handle_accept, this, new_session,
            boost::asio::placeholders::error));
}

void SmtpServer::handle_accept(std::shared_ptr<SmtpSession> new_session, const boost::system::error_code& error)
{
    if (!error) {
        new_session->start();
    }

    start_accept();
}

SmtpSession::SmtpSession(boost::asio::io_context& io_context)
    : socket_(io_context) {}

tcp::socket& SmtpSession::socket() {
    return socket_;
}

void SmtpSession::start() {
    // 发送欢迎消息
    std::string response = "220 localhost SMTP Server Ready\r\n";
    boost::asio::async_write(socket_, boost::asio::buffer(response),
        std::bind(&SmtpSession::handle_write, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void SmtpSession::handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        // 开始读取客户端命令
        boost::asio::async_read_until(socket_, buffer_, '\n',
            std::bind(&SmtpSession::handle_read, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}

void SmtpSession::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error) {
        std::istream is(&buffer_);
        std::string line;
        std::getline(is, line);
        // 去除换行符
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.erase(line.length() - 1);
        }

        std::cout << "Received command: " << line << std::endl;

        // 简单的命令处理
        std::string response;
        if (line.substr(0, 4) == "HELO") {
            response = "250 Hello, glad to meet you\r\n";
        }
        else if (line.substr(0, 4) == "QUIT") {
            response = "221 Bye\r\n";
        }
        else {
            response = "500 Command not recognized\r\n";
        }

        boost::asio::async_write(socket_, boost::asio::buffer(response),
            std::bind(&SmtpSession::handle_write, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}
