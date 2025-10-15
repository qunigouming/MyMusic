#include "Encrypt.h"
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/sha.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/secblock.h>

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

std::string Encrypt::AES_Encrypt(const std::string& plainText)
{
	using namespace CryptoPP;
	// 生成随机IV
	AutoSeededRandomPool rng;
	byte iv[AES::BLOCKSIZE];
    rng.GenerateBlock(iv, sizeof(iv));

	// 加密
	std::string cipherText;
	CBC_Mode<AES>::Encryption enc;
    enc.SetKeyWithIV(reinterpret_cast<const byte*>(KAES_KEY.data()), KAES_KEY.size(), iv);
	StringSource ss(plainText, true, new StreamTransformationFilter(enc, new StringSink(cipherText)));

	std::string combined = std::string((char*)iv, sizeof(iv)) + cipherText;

	std::string encoded;
	StringSource base64_ss(
		combined,
		true,
		new Base64Encoder(
			new StringSink(encoded),
			false // 不添加换行
		)
	);

	return encoded;
}

std::string Encrypt::AES_Decrypt(const std::string& cipherText)
{
	// Base64 解码
	std::string combined;
	StringSource(
		cipherText,
		true,
		new Base64Decoder(
			new StringSink(combined)
		)
	);

	// 分离 IV 和密文
	byte iv[AES::BLOCKSIZE];
	memcpy(iv, combined.data(), sizeof(iv));
	std::string encrypted = combined.substr(sizeof(iv));

	// 解密
	std::string plaintext;
	CBC_Mode<AES>::Decryption dec;
	dec.SetKeyWithIV((const byte*)KAES_KEY.data(), KAES_KEY.size(), iv);

	StringSource ss(
		encrypted,
		true,
		new StreamTransformationFilter(
			dec,
			new StringSink(plaintext)
		)
	);

	return plaintext;
}
