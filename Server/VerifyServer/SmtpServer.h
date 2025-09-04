#pragma once
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// SMTP �Ự��
class SmtpSession : public std::enable_shared_from_this<SmtpSession> {
public:
    SmtpSession(boost::asio::io_context& io_context);

    tcp::socket& socket();

    void start();

private:
    void handle_write(const boost::system::error_code& error,
        size_t bytes_transferred);

    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred);

    tcp::socket socket_;
    boost::asio::streambuf buffer_;
};

// SMTP ��������
class SmtpServer {
public:
    SmtpServer(short port);
    void Start();

private:
    void start_accept();

    void handle_accept(std::shared_ptr<SmtpSession> new_session, const boost::system::error_code& error);

    boost::asio::io_context _ioc_context;
    tcp::acceptor acceptor_;
};

