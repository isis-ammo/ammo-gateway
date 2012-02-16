#include <iostream>
#include <string>
#include <ctime>

#include "ace/Reactor.h"
#include "ace/Select_Reactor.h"

#include "ace/OS_NS_unistd.h" 
#include "ace/Signal.h" 

#include "LdapPushReceiver.h"
#include "GatewayConnector.h"
#include "LdapConfigurationManager.h"

#include "GatewayLdapConstants.h"

#include "log.h"
#include "version.h"

#include "UserSwitch.inl"
#include "LogConfig.inl"

using namespace std;
using namespace ammo::gateway;

//Handle SIGINT so the program can exit cleanly (otherwise, we just terminate
//in the middle of the reactor event loop, which isn't always a good thing).
class SigintHandler : public ACE_Event_Handler {
public:
  int handle_signal (int signum, siginfo_t * = 0, ucontext_t * = 0) {
    if (signum == SIGINT || signum == SIGTERM) {
      ACE_Reactor::instance()->end_reactor_event_loop();
    }
    return 0;
  }
};

int main(int argc, char **argv) 
{
  dropPrivileges();
  setupLogging("LdapGatewayPlugin");
  LOG_FATAL("=========");
  LOG_FATAL("AMMO LDAP Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  
  //Explicitly specify the ACE select reactor; on Windows, ACE defaults
  //to the WFMO reactor, which has radically different semantics and
  //violates assumptions we made in our code
  ACE_Select_Reactor selectReactor;
  ACE_Reactor newReactor(&selectReactor);
  auto_ptr<ACE_Reactor> delete_instance(ACE_Reactor::instance(&newReactor));
  
  SigintHandler * handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);
  
  // Create important objects:
  LdapConfigurationManager::getInstance();
  LOG_DEBUG("Creating gateway connector...");
  LdapPushReceiver *pushReceiver = new LdapPushReceiver();
  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
  
  // Register data interests with Gateway Core:
  LOG_DEBUG("Registering interest in " << CONTACT_MIME_TYPE);
  gatewayConnector->registerDataInterest(CONTACT_MIME_TYPE, pushReceiver);
  
  LOG_DEBUG("Registering interest in " << CONTACT_PULL_MIME_TYPE);
  gatewayConnector->registerPullInterest(CONTACT_PULL_MIME_TYPE, pushReceiver);
  
  // Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  LOG_DEBUG("Starting event loop...");
  reactor->run_reactor_event_loop();
  
  return 0;
}
