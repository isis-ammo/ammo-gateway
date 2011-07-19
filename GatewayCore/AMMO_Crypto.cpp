#include "AMMO_Crypto.h"
#include <openssl/err.h>
#include <openssl/rand.h>

#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>


AMMO_Crypt::AMMO_Crypt ():
            pubkey_(NULL),
            pvtkey_(NULL)
{
  ERR_load_crypto_strings();  
  OpenSSL_add_all_algorithms();
}

AMMO_Crypt::~AMMO_Crypt ()
{
  if (pubkey_ != NULL)
  {
    EVP_PKEY_free (pubkey_);
    EVP_cleanup ();
  }

  if (pvtkey_ != NULL)
  {
    EVP_PKEY_free (pvtkey_);
    EVP_cleanup ();
  }
}

vector<unsigned char> AMMO_Crypt::encrypt (uchar_ptr data, size_t data_len)
{
  //enc must point to RSA_size(rsa) bytes of memory. See man RSA_public_encrypt 
  uchar_ptr enc = new unsigned char[RSA_size(pub_RSAp_)];

  vector<unsigned char> encV;
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

    return encV;
  }
  else
  {
    printf("Encrypted the message OK! = \n%s\n", enc);
  }

  encV.assign(enc, enc+RSA_size(pub_RSAp_));

//  return enc; // sender should delete the buffer 
  return encV;
}

vector<unsigned char> AMMO_Crypt::decrypt (
                                uchar_ptr encr,
                                size_t encr_len)
{
  printf ("The encrypted msg length is %d", encr_len);

  uchar_ptr decr = new unsigned char [RSA_size(pvt_RSAp_)*2];// need  to set 500 to RSA_size ()

  memset (decr, 0, RSA_size(pvt_RSAp_)*2);
  
  vector<unsigned char> decrV;

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

    return decrV;
  }

  //return decr; // the calling function should delete this memory
  decrV.assign(decr, decr+RSA_size(pub_RSAp_)*2);

//  return enc; // sender should delete the buffer 
  return decrV;
}

int AMMO_Crypt::read_public_key (string pub_file)
{
  FILE *keyfile = fopen(pub_file.c_str (), "r");
  
  if (keyfile == NULL)
  {
    printf("No Public Key Found for Phone!\n");
    return -1;
  }

  pub_RSAp_ = PEM_read_RSA_PUBKEY(keyfile, NULL, 0, NULL);
  
  if (pub_RSAp_ == NULL)
  {
    printf("Public Key: Badness has occured! Did not read key file\n");
    return -1;
  }
  else
  {
    printf("Opened the key file OK!\n");

    // assign the EVP key 
    pubkey_ = EVP_PKEY_new ();
    EVP_PKEY_set1_RSA(pubkey_, pub_RSAp_);

    std::cout << "Checking that the type is correct: " << ((EVP_PKEY_type(pubkey_->type) == EVP_PKEY_RSA) ? "OK" : "FAILURE") << std::endl;
    return 0;
  }
}



int AMMO_Crypt::read_private_key (string pvt_file)
{
  FILE *keyfile = fopen(pvt_file.c_str (), "r");

  if (keyfile == NULL)
  {
    printf("No Private Key Found for Phone!\n");
    return -1;
  }

  pvt_RSAp_ = PEM_read_RSAPrivateKey(keyfile, NULL, NULL, NULL);

  if (pvt_RSAp_ == NULL)
  {
    printf("Private Key: Badness has occured! Did not read key file\n");
    return -1;
  }
  else
  {
    printf("Opened the key file OK!\n");
    // assign the EVP key 
    pvtkey_ = EVP_PKEY_new ();
    EVP_PKEY_set1_RSA(pvtkey_, pvt_RSAp_);

    std::cout << "Checking that the type is correct: " << ((EVP_PKEY_type(pvtkey_->type) == EVP_PKEY_RSA) ? "OK" : "FAILURE") << std::endl;
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
/*
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

      std::cout << "Calling Initialize" << std::endl;
      ctx2.verify_initialize(algorithm);
      //		ctx2.verify_update(data.c_str(), data.size());
      std::cout << "After Initialize" << std::endl;
      ctx2.verify_update(data, data_len);
      //		bool verification = ctx2.verify_finalize(&signature[0], signature.size(), pkey);
      std::cout << "After update" << std::endl;
      bool verification = ctx2.verify_finalize(sig, sig_len, pkey);

      std::cout << "Verification: " << (verification ? "OK" : "FAILED") << std::endl;

      return verification;
    }
  catch (cryptoplus::error::cryptographic_exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }


}
*/
//bool AMMO_Crypt::verify_wo_cryptoplus (
bool AMMO_Crypt::verify (
                        uchar_ptr data,
                        size_t data_len,
                        uchar_ptr sig,
                        size_t sig_len)
{

  std::cout << "Message digest signature sample without cryptoplus" << std::endl;
  std::cout << "===============================" << std::endl;
  std::cout << std::endl;

  std::cout << "Data: " << data << std::endl;

  //      cryptoplus::hash::message_digest_algorithm algorithm("SHA1");
  const EVP_MD* messageDigestPtr = EVP_get_digestbyname("SHA1");

  //      cryptoplus::hash::message_digest_context ctx;
  EVP_MD_CTX ctx;
  EVP_MD_CTX_init(&ctx);

  std::cout << "Calling Initialize" << std::endl;

  //      ctx2.verify_initialize(algorithm);
  EVP_VerifyInit_ex(&ctx, messageDigestPtr, NULL/*engine*/);

  std::cout << "After Initialize" << std::endl;

  //      ctx2.verify_update(data, data_len);
  EVP_VerifyUpdate(&ctx, data, data_len);

  std::cout << "After update" << std::endl;
  //      bool verification = ctx2.verify_finalize(sig, sig_len, pkey);

  int result = EVP_VerifyFinal(&ctx, sig, sig_len, pubkey_);

  if (result < 0)
    cout << "Error in Verify Final" << endl;

  std::cout << "Verification: " << ((result == 1) ? "OK" : "FAILED") << std::endl;


  return (result == 1);

}




std::vector<unsigned char> AMMO_Crypt::sign (uchar_ptr data, size_t data_len)
{
  std::cout << "Data: " << data << std::endl;

  //      cryptoplus::hash::message_digest_algorithm algorithm("SHA1");
  const EVP_MD* messageDigestPtr = EVP_get_digestbyname("SHA1");

  //      cryptoplus::hash::message_digest_context ctx;
  EVP_MD_CTX ctx;
  EVP_MD_CTX_init(&ctx);

  std::cout << "Calling Initialize" << std::endl;

  //      ctx.sign_initialize(algorithm);
  EVP_SignInit_ex(&ctx, messageDigestPtr, NULL/*engine*/);

  std::cout << "After Initialize" << std::endl;

  //      ctx.sign_update(data, data_len);
  EVP_SignUpdate (&ctx, data, data_len);

  std::cout << "After update" << std::endl;
  //      std::vector<unsigned char> signature = ctx.sign_finalize<unsigned char>(pkey);
  std::cout << "Signature: print"  << std::endl;

  unsigned int sig_sz = EVP_PKEY_size (pvtkey_);
  std::vector<unsigned char> signature (sig_sz);

  EVP_SignFinal(&ctx, static_cast<unsigned char*>(&signature[0]), &sig_sz, pvtkey_);
  //      std::cout << "Signature length is : " << signature.size() << std::endl;
  //      std::cout << "Signature: " << to_hex(signature.begin(), signature.size()) << std::endl;

  EVP_MD_CTX_cleanup(&ctx);
  return signature;
}

std::vector<unsigned char> AMMO_Crypt::get_random_bytes ()
{
  std::vector<unsigned char> rand(16);
  RAND_bytes(&rand[0], rand.size ());
  return rand;
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

vector<unsigned char> AMMO_Crypt::generate_digest (const std::string algo, const std::string& data )
{
/*
  cryptoplus::crypto_initializer crypto_initializer;
  cryptoplus::algorithms_initializer algorithms_initializer;
  cryptoplus::error::error_strings_initializer error_strings_initializer;
*/

    //cryptoplus::hash::message_digest_algorithm algorithm(algo);
    const EVP_MD* messageDigestPtr = EVP_get_digestbyname(algo.c_str());

    //cryptoplus::hash::message_digest_context ctx;
    
    EVP_MD_CTX ctx;
    EVP_MD_CTX_init(&ctx);

//    ctx.initialize(algorithm);
    EVP_DigestInit_ex(&ctx, messageDigestPtr, NULL/*engine*/);

    //ctx.update(data.c_str(), data.size());
    EVP_DigestUpdate(&ctx, data.c_str(), data.size());

//    vector<unsigned char> message_digest = ctx.finalize<unsigned char>();
    const EVP_MD* md = EVP_MD_CTX_md(&ctx);

    size_t msg_sz = EVP_MD_size(md);

    unsigned char *dgst = new unsigned char[msg_sz];

    unsigned int len;
    
    EVP_DigestFinal_ex(&ctx, dgst, &len);
    
    //  std::cout << name << ": " << to_hex(message_digest.begin(), message_digest.end()) << std::endl;

    vector<unsigned char> msg_digest(dgst, dgst + len);

    return msg_digest;
}

/*
vector<unsigned char> AMMO_Crypt::generate_digest_wo_CP (const std::string algo, const std::string& data )
{
  cryptoplus::crypto_initializer crypto_initializer;
  cryptoplus::algorithms_initializer algorithms_initializer;
  cryptoplus::error::error_strings_initializer error_strings_initializer;

  try
  {
    cryptoplus::hash::message_digest_algorithm algorithm(algo);
    const EVP_MD* messageDigestPtr = EVP_get_digestbyname(algo.c_str());

    cryptoplus::hash::message_digest_context ctx;

    ctx.initialize(algorithm);
    ctx.update(data.c_str(), data.size());
    vector<unsigned char> message_digest = ctx.finalize<unsigned char>();
    //  std::cout << name << ": " << to_hex(message_digest.begin(), message_digest.end()) << std::endl;

    return message_digest;
  }
  catch (cryptoplus::error::cryptographic_exception& ex)
  {
    std::cerr << algo << ": " << ex.what() << std::endl;
  }
}

  */
string AMMO_Crypt::ucharVectorToString (vector<unsigned char> ucharV)
{
      std::string sigStr; 
      sigStr.assign (ucharV.begin (), ucharV.end ());

      return sigStr;
}

vector<unsigned char> AMMO_Crypt::stringToUcharVector (string str)
{
  vector<unsigned char> ucharV;
  ucharV.assign (str.begin(), str.end());

  return ucharV;
}

void AMMO_Crypt::dump_to_file (string file, string buffer)
{
  std::cout << "Writing to file" << std::endl;
  ofstream out (file.c_str(), ios::binary);
  out.write(buffer.data(), buffer.size());
  out.close ();
}
