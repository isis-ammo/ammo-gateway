#include "AMMO_Crypto.h"
#include <openssl/err.h>

#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>


#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/hash/message_digest_context.hpp>
#include <cryptoplus/pkey/pkey.hpp>
#include <cryptoplus/error/error_strings.hpp>
#include <cryptoplus/random/random.hpp>


AMMO_Crypt::AMMO_Crypt ()
{
  ERR_load_crypto_strings();  
}

uchar_ptr AMMO_Crypt::encrypt (uchar_ptr data, size_t data_len)
{
  //enc must point to RSA_size(rsa) bytes of memory. See man RSA_public_encrypt 
  uchar_ptr enc = new unsigned char[RSA_size(pub_RSAp_)];

  //need to check this
  //  flen must be less than RSA_size(rsa) - 11 for the PKCS #1 v1.5 based padding modes

  int enc_len = RSA_public_encrypt(
                                   data_len,
                                   data,
                                   enc,
                                   pub_RSAp_, 
                                   RSA_PKCS1_PADDING);

  if (enc_len == -1)
  {
    printf("Badness has occured! encryption failed\n");
    
    char* err = getErrorString ();

    cout << err  << endl;

    delete err;

    return NULL;
  }
  else
  {
    printf("Encrypted the message OK! = \n%s\n", enc);
  }

  return enc; // sender should delete the buffer 
}

uchar_ptr AMMO_Crypt::decrypt (
                                uchar_ptr encr,
                                size_t encr_len)
{
  printf ("The encrypted msg length is %d", encr_len);

  uchar_ptr decr = new unsigned char [RSA_size(pvt_RSAp_)*2];// need  to set 500 to RSA_size ()

  memset (decr, 0, RSA_size(pvt_RSAp_)*2);

  if (RSA_private_decrypt(encr_len, encr, decr, pvt_RSAp_, RSA_PKCS1_PADDING) != -1)
  {
    printf("\nMessage decrypted to : %s\n", decr);
  }
  else
  {
    printf("Badness has occured! decryption failed\n");

    char* err = getErrorString ();
    cout << err  << endl;
    delete err;

    return NULL;
  }

  return decr; // the calling function should delete this memory
}

int AMMO_Crypt::read_public_key (string pub_file)
{
  FILE *keyfile = fopen(pub_file.c_str (), "r");

  pub_RSAp_ = PEM_read_RSA_PUBKEY(keyfile, NULL, 0, NULL);
  
  if (pub_RSAp_ == NULL)
  {
    printf("Public Key: Badness has occured! Did not read key file\n");
    return -1;
  }
  else
  {
    printf("Opened the key file OK!\n");
    return 0;
  }
}



int AMMO_Crypt::read_private_key (string pvt_file)
{
  FILE *keyfile = fopen(pvt_file.c_str (), "r");

  pvt_RSAp_ = PEM_read_RSAPrivateKey(keyfile, NULL, NULL, NULL);

  if (pvt_RSAp_ == NULL)
  {
    printf("Private Key: Badness has occured! Did not read key file\n");
    return -1;
  }
  else
  {
    printf("Opened the key file OK!\n");
    return 0;
  }
}

char* AMMO_Crypt::getErrorString ()
{
  long err = ERR_get_error ();

  char buf[1024];
  memset(buf, 0 , 1024);
  ERR_error_string(err, buf);

  return buf;
}

bool AMMO_Crypt::verify (
			 uchar_ptr data,
			 size_t data_len,
			 uchar_ptr sig,
			 size_t sig_len)
{
  cryptoplus::crypto_initializer crypto_initializer;
  cryptoplus::algorithms_initializer algorithms_initializer;
  cryptoplus::error::error_strings_initializer error_strings_initializer;

  std::cout << "Message digest signature sample" << std::endl;
  std::cout << "===============================" << std::endl;
  std::cout << std::endl;

  try
    {
      std::cout << "Data: " << data << std::endl;
      
      cryptoplus::pkey::rsa_key rsa_key = 
	cryptoplus::pkey::rsa_key::take_ownership (pub_RSAp_);

      cryptoplus::pkey::pkey pkey = 
	cryptoplus::pkey::pkey::from_rsa_key(rsa_key);

      std::cout << "Checking that the type is correct: " << (pkey.is_rsa() ? "OK" : "FAILURE") << std::endl;
      

      cryptoplus::hash::message_digest_algorithm algorithm("SHA1");

      cryptoplus::hash::message_digest_context ctx2;

      ctx2.verify_initialize(algorithm);
      //		ctx2.verify_update(data.c_str(), data.size());
      ctx2.verify_update(data, data_len);
      //		bool verification = ctx2.verify_finalize(&signature[0], signature.size(), pkey);
      bool verification = ctx2.verify_finalize(sig, sig_len, pkey);

      std::cout << "Verification: " << (verification ? "OK" : "FAILED") << std::endl;

      return verification;
    }
  catch (cryptoplus::error::cryptographic_exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }


}

std::vector<unsigned char> AMMO_Crypt::sign (uchar_ptr data, size_t data_len)
{
  cryptoplus::crypto_initializer crypto_initializer;
  cryptoplus::algorithms_initializer algorithms_initializer;
  cryptoplus::error::error_strings_initializer error_strings_initializer;
  try
    {
      std::cout << "Data: " << data << std::endl;

      cryptoplus::pkey::rsa_key rsa_key = 
        cryptoplus::pkey::rsa_key::take_ownership (pvt_RSAp_);

      cryptoplus::pkey::pkey pkey = 
        cryptoplus::pkey::pkey::from_rsa_key(rsa_key);

      std::cout << "Checking that the type is correct: " << (pkey.is_rsa() ? "OK" : "FAILURE") << std::endl;
      cryptoplus::hash::message_digest_algorithm algorithm("SHA1");
      cryptoplus::hash::message_digest_context ctx;

      ctx.sign_initialize(algorithm);
//      ctx.sign_update(data.c_str(), data.size());
      ctx.sign_update(data, data_len);
      std::vector<unsigned char> signature = ctx.sign_finalize<unsigned char>(pkey);
      
      std::cout << "Signature: print"  << std::endl;
      std::cout << "Signature length is : " << signature.size() << std::endl;
//      std::cout << "Signature: " << to_hex(signature.begin(), signature.size()) << std::endl;
      
      return signature;
    }
  catch (cryptoplus::error::cryptographic_exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
}

std::vector<unsigned char> AMMO_Crypt::get_random_bytes ()
{
  std::vector<unsigned char> bytes = 
       cryptoplus::random::get_random_bytes<unsigned char>(16);
  std::cout << "Random bytes: " << to_hex(bytes.begin(), bytes.end()) << std::endl;  

  return bytes;
}

template <typename T>
std::string AMMO_Crypt::to_hex(const T& begin,const T& end)
{
  std::ostringstream oss;

  for (T i = begin; i != end; ++i)
  {
    oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*i);
  }

  return oss.str();
}

std::string AMMO_Crypt::to_hex(const void* buf, size_t buf_len)
{
  //return to_hex(static_cast<const unsigned char*>(buf), static_cast<const unsigned char*>(buf) + buf_len);
}
