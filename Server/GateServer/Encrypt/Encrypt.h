#pragma once
#include <string>
#include <iostream>

#include <cryptopp/sha.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/secblock.h>

class Encrypt
{
public:
	Encrypt(const std::string& password);
	Encrypt() = default;
	~Encrypt() = default;

	std::string getPasswordHash();
    std::string getPasswordSalt();
	bool VerifyPassword(const std::string& password, const std::string& storeEncodedSalt, const std::string& storeEncodedKey);
	static bool VerifyHashes(const std::string& storedHashBase64, const std::string& receivedHashBase64);

	// 使用密码和盐值计算密码哈希值
	static std::string ComputeHashWithSalt(const std::string& password, const std::string& saltBase64);
private:
	std::string _password_salt;		// 加密盐值
	std::string _password_hash;		// 密码哈希值
	int _iterations = 10;			// 迭代次数
};

