#pragma once
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/StringPartSource.h>
#include <memory>
class Email
{
public:
	Email();
	~Email();

	void sendVerifyCode(std::string recipient, std::string verify_code, bool is_reset = false);
private:
	std::unique_ptr<Poco::Net::SecureSMTPClientSession> m_session;
};

