#include "Email.h"
#include <iostream>
#include "ConfigManager.h"
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/SSLManager.h>
#include "LogManager.h"
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
			LOG(INFO) << "TLS connection established.";
		}
	}
	catch (Poco::Net::SMTPException& e) {
		LOG(INFO) << e.message();
	}
	try {
		LOG(INFO) << "Connecting to smtp.qq.com..." << config["Email"]["User"] << " " << config["Email"]["Passwd"];
		m_session->login(Poco::Net::SecureSMTPClientSession::AUTH_LOGIN, config["Email"]["User"], config["Email"]["Passwd"]);
	}
	catch (Poco::Net::SMTPException& e) {
		LOG(INFO) << e.message();
	}
}

Email::~Email()
{
    m_session->close();
    m_session.reset();
	Poco::Net::uninitializeSSL();
}

void Email::sendVerifyCode(std::string recipient, std::string verify_code, VerifyPurpose purpose)
{
	auto& config = ConfigManager::GetInstance();
	Poco::Net::MailMessage message;
	message.setSender(config["Email"]["User"]);
	message.addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, recipient));
	if (purpose == VerifyPurpose::RESET_PASSWORD) {
		message.setSubject("MyMusic 重置密码验证码");
		message.setContent("您正在进行 MyMusic 密码重置，验证码为：" + verify_code + "。10分钟有效，如非本人操作请忽略。");
	}
	else if (purpose == VerifyPurpose::REGISTER) {
		message.setSubject("MyMusic 注册验证码");
		message.setContent("您正在进行 MyMusic 注册，验证码为：" + verify_code + "。10分钟有效。");
	}
	else {
        LOG(ERROR) << "Invalid VerifyPurpose";
	}
    m_session->sendMessage(message);
    LOG(INFO) << "Genarate Verify code: " << verify_code << " send to:" << recipient;
}
