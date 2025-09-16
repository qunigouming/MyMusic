#include "Email.h"
#include <iostream>
#include "ConfigManager.h"
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/SSLManager.h>
Email::Email()
{
	auto& config = ConfigManager::GetInstance();
	m_session = std::make_unique<Poco::Net::SecureSMTPClientSession>("smtp.qq.com", 587);
	m_session->open();

	// Initialize SSL
	Poco::Net::initializeSSL();

	Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> ptrHandler = new Poco::Net::AcceptCertificateHandler(false);
	Poco::Net::Context::Ptr ptrContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", Poco::Net::Context::VERIFY_NONE, 9, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
	Poco::Net::SSLManager::instance().initializeClient(nullptr, ptrHandler, ptrContext);
	try {
		m_session->login();
		if (m_session->startTLS(ptrContext)) {
			std::cout << "TLS connection established." << std::endl;
		}
	}
	catch (Poco::Net::SMTPException& e) {
		std::cout << e.message() << std::endl;
	}
	try {
		std::cout << "Connecting to smtp.qq.com..." << config["Email"]["User"] << " " << config["Email"]["Passwd"] << std::endl;
		m_session->login(Poco::Net::SecureSMTPClientSession::AUTH_LOGIN, config["Email"]["User"], config["Email"]["Passwd"]);
	}
	catch (Poco::Net::SMTPException& e) {
		std::cout << e.message() << std::endl;
	}
}

Email::~Email()
{
    m_session->close();
    m_session.reset();
	Poco::Net::uninitializeSSL();
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
