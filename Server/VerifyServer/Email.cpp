#include "Email.h"
#include <iostream>
#include "ConfigManager.h"
Email::Email()
{
	auto& config = ConfigManager::GetInstance();
	m_session = std::make_unique<Poco::Net::SecureSMTPClientSession>("smtp.qq.com", 587);

	try {
		m_session->open();
		if (m_session->startTLS()) {
			std::cout << "TLS connection established." << std::endl;
		}
	}
	catch (Poco::Net::SMTPException& e) {
		std::cout << e.message() << std::endl;
	}
	try {
		std::cout << "Connecting to smtp.qq.com..." << config["Email"]["User"] << " " << config["Email"]["Passwd"] << std::endl;
		m_session->login(Poco::Net::SMTPClientSession::AUTH_LOGIN, config["Email"]["User"], config["Email"]["Passwd"]);
	}
	catch (Poco::Net::SMTPException& e) {
		std::cout << e.message() << std::endl;
	}
}

Email::~Email()
{
    m_session->close();
    m_session.reset();
}

void Email::sendVerifyCode(std::string recipient, std::string verify_code)
{
	auto& config = ConfigManager::GetInstance();
	Poco::Net::MailMessage message;
	message.setSender(config["Email"]["User"]);
	message.addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, recipient));
	message.setSubject("MyMusic");
	message.setContent("Your verify code is " + verify_code);
    m_session->sendMessage(message);
    std::cout << "Genarate Verify code: " << verify_code << " send to:" << recipient << std::endl;
}
