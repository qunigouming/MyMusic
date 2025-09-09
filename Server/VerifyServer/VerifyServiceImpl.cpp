#include "VerifyServiceImpl.h"
#include "Email.h"
#include <boost/random.hpp>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "global.h"
#include "RedisManager.h"

std::string generateVerifyCode()
{
	static boost::random::mt19937 gen(std::time(0));
	boost::random::uniform_int_distribution<> dist(0, 9);

	std::stringstream ss;
	for (int i = 0; i < 6; ++i) {
		ss << dist(gen);
	}
	return ss.str();
}

Status VerifyServiceImpl::GetVerifyCode(ServerContext* context, const GetVerifyReq* request, GetVerifyRsp* response)
{
    std::string email = request->email();

	std::string verify_code;
	bool get_success = RedisManager::GetInstance()->Get(CODEPREFIX + email, verify_code);
	if (!get_success) {
		// 未获取到验证码，说明验证码过期或未被创建，直接生成一个
		verify_code = generateVerifyCode();
		std::cout << "generate verify code: " << verify_code << std::endl;
		// 存储验证码
		bool res = RedisManager::GetInstance()->Set(CODEPREFIX + email, verify_code);
        if (!res) {
			std::cout << "RedisManager::Set failed" << std::endl;
            response->set_error(ErrorCodes::OtherError);
            return Status::OK;
        }
		res = RedisManager::GetInstance()->Expire(CODEPREFIX + email, 60 * 10);
		if (!res) {
            std::cout << "RedisManager::Expire failed" << std::endl;
            response->set_error(ErrorCodes::OtherError);
            return Status::OK;
		}
	}

	Email email_sender;
	email_sender.sendVerifyCode(email, verify_code);
	std::cout << "send verify code to " << email << std::endl;
	// 设置回复内容
	response->set_email(email);
	response->set_code(verify_code);
	response->set_error(ErrorCodes::Success);
    return Status::OK;
}
