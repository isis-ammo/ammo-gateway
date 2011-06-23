/**
 * \file pbkdf2.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A PBKDF2 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/hash/pbkdf2.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

template <typename T>
std::string to_hex(const T& begin, const T& end)
{
	std::ostringstream oss;

	for (T i = begin; i != end; ++i)
	{
		oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*i);
	}

	return oss.str();
}

std::string to_hex(const void* buf, size_t buf_len)
{
	return to_hex(static_cast<const unsigned char*>(buf), static_cast<const unsigned char*>(buf) + buf_len);
}

void pbkdf2(const std::string& name, const std::string& password, const std::string& salt, unsigned int iterations)
{
	try
	{
		cryptoplus::hash::message_digest_algorithm algorithm(name);

		std::vector<unsigned char> key = cryptoplus::hash::pbkdf2<unsigned char>(password.c_str(), password.size(), salt.c_str(), salt.size(), algorithm, iterations);
		std::cout << name << ": " << to_hex(key.begin(), key.end()) << std::endl;
	}
	catch (cryptoplus::error::cryptographic_exception& ex)
	{
		std::cerr << name << ": " << ex.what() << std::endl;
	}
}

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "PBKDF2 sample" << std::endl;
	std::cout << "=============" << std::endl;
	std::cout << std::endl;

	const std::string password = "this is a very long and secret key";
	const std::string salt = "a salt value";
	const unsigned int iterations = 1000;

	std::cout << "Password: " << password << std::endl;
	std::cout << "Salt: " << salt << std::endl;
	std::cout << "Iterations: " << iterations << std::endl;
	std::cout << std::endl;

#if OPENSSL_VERSION_NUMBER >= 0x01000000
	pbkdf2("MD5", password, salt, iterations);
	pbkdf2("MD4", password, salt, iterations);
#endif
	pbkdf2("SHA1", password, salt, iterations);
#if OPENSSL_VERSION_NUMBER >= 0x01000000
	pbkdf2("SHA", password, salt, iterations);
	pbkdf2("SHA224", password, salt, iterations);
	pbkdf2("SHA256", password, salt, iterations);
	pbkdf2("SHA384", password, salt, iterations);
	pbkdf2("SHA512", password, salt, iterations);
	pbkdf2("MDC2", password, salt, iterations);
	pbkdf2("whirlpool", password, salt, iterations);
	pbkdf2("RIPEMD160", password, salt, iterations);
#endif

	return EXIT_SUCCESS;
}
