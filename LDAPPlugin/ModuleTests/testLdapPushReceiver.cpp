#include <iostream>
#include <string>

#include <ctime>

//#include "ace/Reactor.h"

//#include "LdapPushReceiver.h"
//#include "GatewayConnector.h"

#include "LdapConfigurationManager.h"


using namespace std;

const string CONTACT_MIME_TYPE = "ammo/edu.vu.isis.ammo.launcher.contact";
const string CONTACT_PULL_MIME_TYPE = "ammo/edu.vu.isis.ammo.launcher.contact_pull";


int main(int argc, char **argv) {  
  LdapConfigurationManager::getInstance();
  
  cout << "Creating gateway connector..." << endl << flush;
  
  //  LdapPushReceiver *pushReceiver = new LdapPushReceiver();
  
  //  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
  
  //  cout << "Registering interest in " << CONTACT_MIME_TYPE << endl << flush;
  //  gatewayConnector->registerDataInterest(CONTACT_MIME_TYPE, pushReceiver);
  
  //  cout << "Registering interest in " << CONTACT_PULL_MIME_TYPE << endl << flush;
  //  gatewayConnector->registerPullInterest(CONTACT_PULL_MIME_TYPE, pushReceiver);
  
  //  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  //  ACE_Reactor *reactor = ACE_Reactor::instance();
  //  cout << "Starting event loop..." << endl << flush;
  //  reactor->run_reactor_event_loop();
  
  return 0;
}
