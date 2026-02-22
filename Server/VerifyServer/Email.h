#pragma once
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/StringPartSource.h>
#include <memory>

enum class VerifyPurpose {
	REGISTER = 0,
	RESET_PASSWORD = 1
};

class Email
{
public:
	Email();
	~Email();

	void sendVerifyCode(std::string recipient, std::string verify_code, VerifyPurpose purpose);
private:
	std::unique_ptr<Poco::Net::SecureSMTPClientSession> m_session;
};

