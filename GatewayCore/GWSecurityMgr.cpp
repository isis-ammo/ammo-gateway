#include "GWSecurityMgr.h"
#include "AMMO_Crypto.h"
#include "GatewaySecHandler.h"

const int GWSecurityMgr::PRE_MASTER_LENGTH = 48;

GWSecurityMgr::GWSecurityMgr (const char* gatewayId, GatewaySecHandler *handler):gatewayId_(gatewayId), handler_(handler)
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
  printf ("\n The Gateway ID is [%s]\n", gatewayId);
}

//bool GWSecurityMgr::Authenticate (ammo::protocol::MessageWrapper &msg)
bool GWSecurityMgr::Authenticate (AuthMessage &msg)
{
 
  printf ("\n Inside the Authenticate Method.");

  if (msg.type == AuthMessage::CLIENT_NONCE)
  {
    
    set_client_nonce (msg.message);
    set_device_id(msg.device_id);

  //  LOG_TRACE(commsHandler << "Got the Client Nonce ");
    cout << "Got the Client Nonce " << endl;

    std::vector<unsigned char> nonce = get_Server_Nonce ();
    std::string sigStr; 
    sigStr.assign (nonce.begin (), nonce.end ());

    cout << "Server Nonce Length is " << endl;

    AuthMessage outmsg;
    outmsg.type = AuthMessage::SERVER_NONCE;
    outmsg.message = sigStr;

    cout << " Returning from Authenticate..." << endl;
    handler_->sendMessage(outmsg);

  } 
  else if (msg.type == AuthMessage::CLIENT_KEYXCHANGE)
  {
    set_keyXchange (msg.message);

  } 
  else if (msg.type == AuthMessage::CLIENT_PHNAUTH)
  {

    string phnAuth = msg.message;

    cout << "Got the Phone Auth" << endl;

    set_phn_auth (phnAuth);

    bool ver = verify_phone_auth ();
    
    cout << "Verified " << ((ver)? "OK": "FAILED") << endl;

    if (ver)
    {
      generate_master_secret ();
    }
    else
      return false;
  
  } 
  else if (msg.type == AuthMessage::CLIENT_FINISH) 
  {

    cout << "Got the CLIENT FINISH MSG" << endl;

    string clnt_fin = msg.message;

    if(verify_client_finish (clnt_fin))
    {
      cout << "Client Finish Verified OK" << endl;

      // Client Finish verified ok ... so now send the gateway finish ...

//      LOG_DEBUG(commsHandler << " Returning from Authenticate after sending Gateway Finish...");
      cout << " Returning from Authenticate after sending Gateway Finish..." << endl;

      AuthMessage outmsg;
      outmsg.type = AuthMessage::SERVER_FINISH;
      outmsg.message = get_server_finish ();
      
      handler_->sendMessage(outmsg);

      handler_->authenticationComplete ();

    }else {
      //LOG_TRACE(commsHandler << "Client Finish Verified False"); 
      cout << "Client Finish Verified False" << endl;

      // Client Finish verified false ... so now send, auth false ...
      return false;
    }
  }  

  return true;
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
  //server_nonce_ = "Roy";
  //std::vector<unsigned char> bytes (server_nonce_.begin(), server_nonce_.end());
  
  return bytes;
}

void GWSecurityMgr::set_client_nonce (string cl_nonce)
{
  client_nonce_ = cl_nonce;
}

void GWSecurityMgr::set_keyXchange (string keyX)
{
  keyXChange_ = keyX;

  extract_pre_master();
}

void GWSecurityMgr::extract_pre_master ()
{
  // decrypt the keyXchange 
  /*
  pre_master_ = crp.ucharVectorToString(
                              crp.decrypt ((unsigned char*)keyXChange_.c_str(), 
                              keyXChange_.size()));
                              */
  vector<unsigned char> uchar = crp.decrypt ((unsigned char*)keyXChange_.c_str(), keyXChange_.size());

  pre_master_.assign(uchar.begin(), uchar.begin() + PRE_MASTER_LENGTH); 
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

void GWSecurityMgr::generate_master_secret()
{

  // the master secret used nested digest ...
  // the first digest is 
  // SHA-256('A'||S||client_nonce||server_nonce)
  // S is the premaster secret
  
//  string content;// = "A";
//  content += pre_master_;
  string content = pre_master_;
  content += client_nonce_;
  content += server_nonce_;
  
  string first_level = 
              crp.ucharVectorToString(crp.generate_digest ("SHA256", content));

  crp.dump_to_file("master_secret", pre_master_);

  // the second level wiil have another message digest ... 
  // SHA-256 (S||first_level)

  content = pre_master_ + first_level;

  master_secret_ = crp.ucharVectorToString(crp.generate_digest ("SHA256", content));

//  crp.dump_to_file("master_secret", first_level);
}

string GWSecurityMgr::regenerate_phone_finish ()
{
  // PhoneFinish = SHA-256(master_secret||pad2||
  //    SHA-256(handshake_msg||sender||master_secret||pad1))

  // handshake_msg = phoneauth + keyxchange + client_nonce + server_nonce 

  // create the handshake msg 
  string handshake_msg = phnAuth_ + keyXChange_ + client_nonce_ + server_nonce_;

  // add the other stuff 
  string content = handshake_msg + deviceId_ + master_secret_; // need to add pad1 here ....

  string first_level = crp.ucharVectorToString(crp.generate_digest("SHA256", content));

  // no create the outer level 
  content = master_secret_ + first_level;

  phone_finish_ = crp.ucharVectorToString(crp.generate_digest ("SHA256", content));

  return phone_finish_;
}

void GWSecurityMgr::set_device_id (string device)
{
  deviceId_ = device;
}

bool GWSecurityMgr::verify_client_finish(string clientFin)
{
  string client_finish = regenerate_phone_finish();

  return (clientFin == client_finish);
}


string GWSecurityMgr::get_server_finish ()
{
  // create the handshake msg 
  string handshake_msg = phnAuth_ + keyXChange_ + client_nonce_ + server_nonce_;

  // add the other stuff 
  string content = handshake_msg + gatewayId_ + master_secret_; // need to add pad1 here ....

  string first_level = crp.ucharVectorToString(crp.generate_digest("SHA256", content));

  // no create the outer level 
  content = master_secret_ + first_level;

  phone_finish_ = crp.ucharVectorToString(crp.generate_digest ("SHA256", content));

  return phone_finish_;
}
