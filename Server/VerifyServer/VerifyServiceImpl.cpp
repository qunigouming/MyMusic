#include "VerifyServiceImpl.h"
#include "Email.h"
#include <boost/random.hpp>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "global.h"
#include "RedisManager.h"
#include "LogManager.h"

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
    bool is_reset = false;
    const std::string reset_prefix = "reset:";
    if (email.rfind(reset_prefix, 0) == 0) {
        is_reset = true;
        email = email.substr(reset_prefix.length());
    }

    const std::string redis_key = (is_reset ? RESET_CODEPREFIX : CODEPREFIX) + email;

	std::string verify_code;
	bool get_success = RedisManager::GetInstance()->Get(redis_key, verify_code);
	if (!get_success) {
		// 未获取到验证码，说明验证码过期或未被创建，直接生成一个
		verify_code = generateVerifyCode();
		LOG(INFO) << "generate verify code: " << verify_code;
		// 存储验证码
		bool res = RedisManager::GetInstance()->Set(redis_key, verify_code);
        if (!res) {
			LOG(ERROR) << "RedisManager::Set failed";
            response->set_error(ErrorCodes::OtherError);
            return Status::OK;
        }
		res = RedisManager::GetInstance()->Expire(redis_key, 60 * 10);
		if (!res) {
            LOG(ERROR) << "RedisManager::Expire failed";
            response->set_error(ErrorCodes::OtherError);
            return Status::OK;
		}
	}

	Email email_sender;
	email_sender.sendVerifyCode(email, verify_code, is_reset);
	LOG(INFO) << "send verify code to " << email;
	// 设置回复
容
	response->set_email(email);
	response->set_code(verify_code);
	response->set_error(ErrorCodes::Success);
    return Status::OK;
}
