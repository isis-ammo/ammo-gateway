#include "GWSecurityMgr.h"
#include "AMMO_Crypto.h"


GWSecurityMgr::GWSecurityMgr ()
{
  // Initialize the oper ids .... 
  pass_keys[std::string("operator")] = string("operator");
  printf ("\n Inside the Constructor \n");
  operator_id = "operator";
  // get the pubkey file of the phone from the device id ..
  string pubkey_phn = "public_key_phone.pem";

  string gw_pvtkey = "private_key_gateway.pem";

  // Read the public and private keys
  crp.read_public_key (pubkey_phn);

  crp.read_private_key (gw_pvtkey);
}

bool GWSecurityMgr::Authenticate (ammo::protocol::MessageWrapper &msg)
{
 
  printf ("\n Inside the Authenticate Method.");
  //LOG_DEBUG(commsHandler << " Inside the Authenticate Method...");
  ammo::protocol::AuthenticationMessage authMessage = msg.authentication_message();

  //get the phone id ... 
  string deviceId = authMessage.device_id ();


  // first decrypt the 
//  unsigned char* decr = 
//            crp.decrypt ((unsigned char*)authMessage.user_key().c_str(), authMessage.user_key().length ());

  //char* data = "operator";
  
  printf ("Operator [operator] key [%s] length [%d]\n",
            pass_keys[std::string("operator")].c_str (), 
            pass_keys["operator"].size());
  
  //cout << pass_keys[std::string("operator")] << endl;
  //printf ("strlen %d\n", strlen(data));

  bool verified = crp.verify ((unsigned char*)pass_keys[string("operator")].c_str()/*operator_id*/,
                              pass_keys["operator"].size()/*strlen(operator_id)*/, 
                               //8,
                              (unsigned char*)authMessage.user_key().c_str(),
                              authMessage.user_key().length ());

 
//  printf ("\n The key is %s\n", decr);
//  LOG_DEBUG(commsHandler << " The key is " << decr << "..");

  printf ((verified)? "True Device":"Faulty Device");

  return verified;
}

std::vector<unsigned char> GWSecurityMgr::get_gateway_sign ()
{
  printf ("\n Inside the get_gateway_sign Method.");
  //char* data = "operator";

//  return crp.sign ((unsigned char*)operator_id, strlen(operator_id));
  return crp.sign ((unsigned char*)pass_keys[string("operator")].c_str(), pass_keys[string("operator")].size());
}

std::vector<unsigned char> GWSecurityMgr::get_Server_Nonce ()
{
  std::vector<unsigned char> bytes = crp.get_random_bytes ();
  server_nonce_.assign (bytes.begin(), bytes.end());
  
  return bytes;
}

void GWSecurityMgr::set_client_nonce (string cl_nonce)
{
  client_nonce_ = cl_nonce;
}

void GWSecurityMgr::set_keyXchange (string keyX)
{
  keyXChange_ = keyX;
}
void GWSecurityMgr::set_phn_auth (string phnA)
{
  phnAuth_ =  phnA;
}

bool GWSecurityMgr::verify_phone_auth ()
{
  //string orig_data = keyXChange_ + client_nonce_;// + server_nonce_;
  string orig_data = keyXChange_ + client_nonce_ + server_nonce_;
//  string orig_data = keyXChange_ + server_nonce_;
//  string orig_data = keyXChange_;
//  string orig_data = client_nonce_;
//  string orig_data = server_nonce_;
  printf ("\n Before calling the verify function \n");

  return crp.verify ((unsigned char*)orig_data.c_str(),
                     orig_data.size(), 
                     (unsigned char*)phnAuth_.c_str(),
                     phnAuth_.length ());
}
