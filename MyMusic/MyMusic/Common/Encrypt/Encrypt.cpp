#include "Encrypt.h"

using namespace CryptoPP;

Encrypt::Encrypt(const std::string& password)
{
	SecByteBlock derivedKey(32);

	// 生成随机盐
	AutoSeededRandomPool rng;
	SecByteBlock randomSalt(16);
	rng.GenerateBlock(randomSalt, randomSalt.size());

	// 将随机盐转换为Base64
	StringSource ss_salt(randomSalt, randomSalt.size(), true, new Base64Encoder(new StringSink(_password_salt), false));
	std::cout << "Generated Salt (Base64): " << _password_salt << std::endl;

	// 生成密钥
	PKCS5_PBKDF2_HMAC<SHA256> pbkdf2;

	try {
		pbkdf2.DeriveKey(derivedKey, derivedKey.size(), 0,
			(const byte*)password.data(), password.size(),
			randomSalt, randomSalt.size(),
			_iterations);
	}
	catch (const CryptoPP::Exception& e) {
		std::cerr << "Key derivation failed: " << e.what() << std::endl;
		return;
	}

	// 将密钥转换为Base64
	StringSource ss_key(derivedKey, derivedKey.size(), true, new Base64Encoder(new StringSink(_password_hash), false));
	std::cout << "Derived Hash (Base64): " << _password_hash << std::endl;
}

std::string Encrypt::getPasswordHash()
{
	return _password_hash;
}

std::string Encrypt::getPasswordSalt()
{
	return _password_salt;
}

bool Encrypt::VerifyPassword(const std::string& password, const std::string& storeEncodedSalt, const std::string& storeEncodedKey)
{
	try {
		SecByteBlock storedKey(32);
		SecByteBlock storedSalt(16);

		// 将Base64解码
		std::string decodedSalt, decodedKey;
		StringSource ss_salt(storeEncodedSalt, true, new Base64Decoder(new StringSink(decodedSalt)));
		StringSource ss_key(storeEncodedKey, true, new Base64Decoder(new StringSink(decodedKey)));

		// 将 std::string 转换为 SecByteBlock 或直接使用指针
		const byte* saltPtr = reinterpret_cast<const byte*>(decodedSalt.data());
		size_t saltSize = decodedSalt.size();
		const byte* storedKeyPtr = reinterpret_cast<const byte*>(decodedKey.data());
		size_t storedKeySize = decodedKey.size();

		SecByteBlock newDerivedKey(32);
		PKCS5_PBKDF2_HMAC<SHA256> pbkdf2;
		pbkdf2.DeriveKey(newDerivedKey, newDerivedKey.size(), 0,
			(const byte*)password.data(), password.size(),
			saltPtr, saltSize,
			_iterations);
		return VerifyBufsEqual(newDerivedKey, storedKeyPtr, storedKeySize);
	}
	catch (const std::exception& e) {
		std::cerr << "Verification failed: " << e.what() << std::endl;
		return false;
	}
}

std::string Encrypt::ComputeHashWithSalt(const std::string& password, const std::string& saltBase64)
{
	try {
		// 解码Base64 salt
		std::string decodedSalt;
		StringSource ss_salt(saltBase64, true, new Base64Decoder(new StringSink(decodedSalt)));

		SecByteBlock derivedKey(32);
		PKCS5_PBKDF2_HMAC<SHA256> pbkdf2;
		pbkdf2.DeriveKey(derivedKey, derivedKey.size(), 0,
			(const byte*)password.data(), password.size(),
			(const byte*)decodedSalt.data(), decodedSalt.size(),
			10);

		std::string encodedHash;
		StringSource ss_key(derivedKey, derivedKey.size(), true, new Base64Encoder(new StringSink(encodedHash), false));
		return encodedHash;
	}
	catch (const std::exception& e) {
		std::cerr << "ComputeHashWithSalt failed: " << e.what() << std::endl;
		return "";
	}
}
