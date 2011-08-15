#include "AMMO_Crypto.h"
#include "log.h"
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
  }

  if (pvtkey_ != NULL)
  {
    EVP_PKEY_free (pvtkey_);
  }
  
  EVP_cleanup ();
  ERR_free_strings();
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
    LOG_ERROR("Badness has occured! encryption failed");
    
    char* err = getErrorString ();

    LOG_ERROR(err);

    delete err;

    return encV;
  }
  else
  {
    LOG_TRACE("Encrypted the message OK!");
  }

  encV.assign(enc, enc+RSA_size(pub_RSAp_));

  delete [] enc;

//  return enc; // sender should delete the buffer 
  return encV;
}

vector<unsigned char> AMMO_Crypt::decrypt (
                                uchar_ptr encr,
                                size_t encr_len)
{
  LOG_TRACE("The encrypted msg length is " << encr_len);

  uchar_ptr decr = new unsigned char [RSA_size(pvt_RSAp_)*2];// need  to set 500 to RSA_size ()

  memset (decr, 0, RSA_size(pvt_RSAp_)*2);
  
  vector<unsigned char> decrV;

  if (RSA_private_decrypt(encr_len, encr, decr, pvt_RSAp_, RSA_PKCS1_PADDING) != -1)
  {
    LOG_TRACE("Message decrypted");
  }
  else
  {
    LOG_ERROR("Badness has occured! decryption failed");

    char* err = getErrorString ();
    LOG_ERROR(err);
    delete err;

    return decrV;
  }

  decrV.assign(decr, decr+RSA_size(pub_RSAp_)*2);

  delete [] decr;

//  return enc; // sender should delete the buffer 
  return decrV;
}

int AMMO_Crypt::read_public_key (string pub_file)
{
  

  FILE *keyfile = fopen(pub_file.c_str (), "r");
  
  LOG_TRACE("Pub key path: " << pub_file.c_str());
  
  if (keyfile == NULL)
  {
    LOG_ERROR("No Public Key Found for Phone!");
    return -1;
  }

  pub_RSAp_ = PEM_read_RSA_PUBKEY(keyfile, NULL, 0, NULL);

  fclose(keyfile);
  
  if (pub_RSAp_ == NULL)
  {
    LOG_ERROR("Public Key: Badness has occured! Did not read key file");
    return -1;
  }
  else
  {
    LOG_TRACE("Opened the key file OK!");

    // assign the EVP key 
    pubkey_ = EVP_PKEY_new ();
    
    if (pubkey_ == NULL)
    {
      LOG_FATAL("Cannot allocate memory for EVP_PKEY_new");

      return -1;
    }

    if(!EVP_PKEY_set1_RSA(pubkey_, pub_RSAp_))
    {
      LOG_FATAL("Error on EVP_PKEY_set1RSA");
      return -1;
    }

    LOG_TRACE("Checking that the type is correct: " 
              << ((EVP_PKEY_type(pubkey_->type) == EVP_PKEY_RSA) ? "OK" : "FAILURE"));

    return 0;
  }
}



int AMMO_Crypt::read_private_key (string pvt_file)
{
  FILE *keyfile = fopen(pvt_file.c_str (), "r");

  LOG_TRACE("Pvt key path : " << pvt_file.c_str());

  if (keyfile == NULL)
  {
    LOG_TRACE("No Private Key Found for Phone!");
    return -1;
  }

  pvt_RSAp_ = PEM_read_RSAPrivateKey(keyfile, NULL, NULL, NULL);

  if (pvt_RSAp_ == NULL)
  {
    LOG_ERROR("Private Key: Badness has occured! Did not read key file");
    return -1;
  }
  else
  {
    LOG_TRACE("Opened the key file OK!\n");
    // assign the EVP key 
    pvtkey_ = EVP_PKEY_new ();
    
    if (pvtkey_ == NULL)
    {
      LOG_FATAL("Cannot allocate memory for EVP_PKEY_new");
      return -1;
    }

    if (!EVP_PKEY_set1_RSA(pvtkey_, pvt_RSAp_))
    {
      LOG_ERROR("Fatal Error: Error on EVP_PKEY_set1RSA");
      return -1;
    }

    LOG_TRACE("Checking that the type is correct: " << ((EVP_PKEY_type(pvtkey_->type) == EVP_PKEY_RSA) ? "OK" : "FAILURE"));
    return 0;
  }
}

char* AMMO_Crypt::getErrorString ()
{
  //TODO:  Don't return buf directly; this code will crash (you can't return
  //       a pointer to a local variable)
  long err = ERR_get_error ();

  if (err == 0) // no error present
    return NULL;

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
  //      cryptoplus::hash::message_digest_algorithm algorithm("SHA1");
  const EVP_MD* messageDigestPtr = EVP_get_digestbyname("SHA1");

  if (messageDigestPtr == NULL)
  {
    LOG_ERROR("Error in EVP_get_digestbyname: Cannot Verify");
    return false;
  }

  //      cryptoplus::hash::message_digest_context ctx;
  EVP_MD_CTX ctx;
  EVP_MD_CTX_init(&ctx);

  LOG_TRACE("Calling Initialize");

  //      ctx2.verify_initialize(algorithm);
  EVP_VerifyInit_ex(&ctx, messageDigestPtr, NULL/*engine*/);

  LOG_TRACE("After Initialize" );

  //      ctx2.verify_update(data, data_len);
  EVP_VerifyUpdate(&ctx, data, data_len);

  LOG_TRACE("After update");
  //      bool verification = ctx2.verify_finalize(sig, sig_len, pkey);

  int result = EVP_VerifyFinal(&ctx, sig, sig_len, pubkey_);

  EVP_MD_CTX_cleanup(&ctx);

  if (result < 0)
    LOG_ERROR("Error in Verify Final");

  LOG_TRACE("Verification: " << ((result == 1) ? "OK" : "FAILED"));


  return (result == 1);

}




std::vector<unsigned char> AMMO_Crypt::sign (uchar_ptr data, size_t data_len)
{
  //      cryptoplus::hash::message_digest_algorithm algorithm("SHA1");
  const EVP_MD* messageDigestPtr = EVP_get_digestbyname("SHA1");
  
  if (messageDigestPtr == NULL)
  {
    LOG_ERROR("Error in EVP_get_digestbyname: Error in Signing");
  }

  //      cryptoplus::hash::message_digest_context ctx;
  EVP_MD_CTX ctx;
  EVP_MD_CTX_init(&ctx);

  LOG_TRACE("Calling Initialize");

  //      ctx.sign_initialize(algorithm);
  EVP_SignInit_ex(&ctx, messageDigestPtr, NULL/*engine*/);

  LOG_TRACE("After Initialize");

  //      ctx.sign_update(data, data_len);
  EVP_SignUpdate (&ctx, data, data_len);

  LOG_TRACE("After update");
  //      std::vector<unsigned char> signature = ctx.sign_finalize<unsigned char>(pkey);
  LOG_TRACE("Signature: print" );

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
    const EVP_MD* messageDigestPtr = EVP_get_digestbyname(algo.c_str());

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
    
    EVP_MD_CTX_cleanup(&ctx);
    
    vector<unsigned char> msg_digest(dgst, dgst + len);

    return msg_digest;
}

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
  LOG_TRACE("Writing to file");
  ofstream out (file.c_str(), ios::binary);
  out.write(buffer.data(), buffer.size());
  out.close ();
}
