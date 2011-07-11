#ifndef GWSECURITYMGRH
#define GWSECURITYMGRH

#include "protocol/AmmoMessages.pb.h"
#include "AMMO_Crypto.h"

#include <iostream>

using namespace std;

#include<map>

using namespace std;

class GatewaySecHandler;

struct AuthMessage 
{

  enum Type 
  {
    CLIENT_NONCE,
    SERVER_NONCE,
    CLIENT_KEYXCHANGE,
    CLIENT_PHNAUTH,
    CLIENT_FINISH,
    SERVER_FINISH
  };

  Type type;
  string message;
  string device_id;
  string user_id;
};

class GWSecurityMgr 
{

  public:

    static const int PRE_MASTER_LENGTH;

//  this is for the demo only
    char * operator_id;

    // This function checks the message and if it is 
    // genuine then sends back a return msg
    // If it is not then returns null
//    bool Authenticate (ammo::protocol::MessageWrapper &msg);
    bool Authenticate (AuthMessage &msg);

    std::vector<unsigned char> get_gateway_sign ();

    GWSecurityMgr (const char* gatewayId, GatewaySecHandler *handler);
    
    std::vector<unsigned char> get_Server_Nonce ();
    
    void set_client_nonce (string cl_nonce);

    void set_keyXchange (string keyX);
    
    void set_phn_auth (string phnA);

    bool verify_phone_auth ();

    void set_device_id (string device);
    
    void generate_master_secret();

    string regenerate_phone_finish ();

    bool verify_client_finish (string);
    
    string get_server_finish ();

  private:
    // The Crypto object providing basic 
    // cryptographic functions such as encrypt, decrypt, sign,
    // verify ...
   AMMO_Crypt crp; 

   map<string, string> pass_keys;
//   vector<int> previous

   string client_nonce_;

   string keyXChange_;

   string phnAuth_;

   string server_nonce_;
   
   string pre_master_;

   string master_secret_;

   void extract_pre_master ();

   string deviceId_;

   string phone_finish_;

   string gatewayId_;

   GatewaySecHandler * handler_;
};


#endif
