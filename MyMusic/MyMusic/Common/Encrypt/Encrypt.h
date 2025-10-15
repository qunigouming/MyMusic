#pragma once
#include <string>
#include <iostream>

class Encrypt
{
public:
	Encrypt(const std::string& password);
	Encrypt() = default;
	~Encrypt() = default;

	std::string getPasswordHash();
	std::string getPasswordSalt();
	bool VerifyPassword(const std::string& password, const std::string& storeEncodedSalt, const std::string& storeEncodedKey);

	// 使用密码和盐值计算密码哈希值
	static std::string ComputeHashWithSalt(const std::string& password, const std::string& saltBase64);

	std::string AES_Encrypt(const std::string& plainText);
    std::string AES_Decrypt(const std::string& cipherText);
private:
	std::string _password_salt;		// 加密盐值
	std::string _password_hash;		// 密码哈希值
	int _iterations = 10;			// 迭代次数

	const std::string KAES_KEY = "qgr&YrAzE!Gv$}R6";
};

