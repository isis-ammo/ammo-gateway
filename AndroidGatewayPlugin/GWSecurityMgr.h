#ifndef GWSECURITYMGRH
#define GWSECURITYMGRH

#include "protocol/AmmoMessages.pb.h"
#include "AMMO_Crypto.h"

#include<map>

using namespace std;

class GWSecurityMgr {

  public:

//  this is for the demo only
    char * operator_id;

    // This function checks the message and if it is 
    // genuine then sends back a return msg
    // If it is not then returns null
    bool Authenticate (ammo::protocol::MessageWrapper &msg);

    std::vector<unsigned char> get_gateway_sign ();

    GWSecurityMgr ();

  private:
    // The Crypto object providing basic 
    // cryptographic functions such as encrypt, decrypt, sign,
    // verify ...
   AMMO_Crypt crp; 

   map<string, string> pass_keys;
//   vector<int> previous
};


#endif
