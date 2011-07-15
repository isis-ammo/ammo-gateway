#ifndef AMMO_CRYPTOH
#define AMMO_CRYPTOH

#include <openssl/rsa.h>
#include <openssl/pem.h>


#include <stdio.h>
#include <string>
#include <vector>

using namespace std;


/**
 * @class AMMO_Crypt
 *
 * @brief Implements the basic functions of encryption, decryption
 * sign and verify.
 *
 */

typedef unsigned char* uchar_ptr;

class AMMO_Crypt 
{

 public:
  AMMO_Crypt ();
  ~AMMO_Crypt ();

  vector<unsigned char> encrypt(uchar_ptr data, size_t data_len);

  vector<unsigned char> decrypt(uchar_ptr encr, size_t encr_len);
  
  vector<unsigned char> sign(uchar_ptr data, size_t data_len);

  bool verify (
               uchar_ptr data,
               size_t data_len,
               uchar_ptr sig,
               size_t sig_len);

/*
bool verify_wo_cryptoplus (
                          uchar_ptr data,
                          size_t data_len,
                          uchar_ptr sig,
                          size_t sig_len);
                          */

   int read_public_key (std::string pub_file);
   
   int read_private_key (std::string pvt_file);

   std::vector<unsigned char> get_random_bytes ();

   vector<unsigned char> generate_digest (const std::string algo,const std::string& data );

   string ucharVectorToString (vector<unsigned char> ucharV);

   vector<unsigned char> stringToUcharVector (string);
   
   void dump_to_file (string file, string buffer);

  private:
    
    RSA * pub_RSAp_;
    RSA * pvt_RSAp_;

    EVP_PKEY* pubkey_;
    EVP_PKEY* pvtkey_;


    char* getErrorString ();
    
   // to hex function ...
    template <typename T>
    std::string to_hex(const T& begin,const T& end);
   
   std::string to_hex(const void* buf, size_t buf_len);
};


#endif
