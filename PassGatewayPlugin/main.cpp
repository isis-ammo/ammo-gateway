#include <iostream>
#include <string>
#include <ctime>
#include <sstream>

#include "ace/Signal.h" 
#include "ace/Reactor.h"

#include "log.h"
#include "version.h"

#include "soap/PASSPortBinding.nsmap"

#include "PassSubscriberTask.h"
#include "PassAmmmoPublisher.h"
#include "PassConfigurationManager.h"
#include "PassGatewayReceiver.h"

using namespace std;
using namespace ammo::gateway;

// If this function is called in main() for testing, it must come
// AFTER the existing call to PassConfigurationManager:;getInstance().
void testPublish (void)
{
  static int id = 0;
  ostringstream json;
  json << "{";
  json << "\"lid\": \"" << "0" << "\", ";
  json << "\"userid\": \"" << "TestID#" << id++ << "\", ";
  json << "\"unitid\": \"" << "0" << "\", ";
  json << "\"name\": \"" << "TestName#" << id << "\", ";
  json << "\"lat\": \"" << "55.3276" << "\", ";
  json << "\"lon\": \"" << "87.1298" << "\",";
  json << "\"created\": \"" << time(0) << "\", ";
  json << "\"modified\": \"" << "0" << "\"";
  json << "}";
  
  PushData pd;
  pd.uri = "my_uri";
  pd.mimeType = "ammo/com.aterrasys.nevada.locations";
  pd.data = json.str ();

  PassConfigurationManager *cfg_mgr =
    PassConfigurationManager::getInstance ();
    
  cfg_mgr->getReceiver ()->onPushDataReceived (0, pd);
}

// Handle SIGINT so the program can exit cleanly (otherwise, we just terminate
// in the middle of the reactor event loop, which isn't always a good thing).
class SigintHandler : public ACE_Event_Handler {
public:
  int
  handle_signal (int signum, siginfo_t * = 0, ucontext_t * = 0)
  {
    if (signum == SIGINT
        || signum == SIGTERM 
        || signum == SIGQUIT 
        || signum == SIGTSTP)
      {
        ACE_Reactor::instance ()->end_reactor_event_loop ();
      }
    
    return 0;
  }
};

int
main (int /* argc */, char ** /* argv */)
{  
  LOG_INFO ("AMMO PASS Gateway Plugin ("
            << VERSION
            << " built on "
            << __DATE__
            << " at "
            << __TIME__
            << ")");
  
  SigintHandler * handleExit = new SigintHandler ();
  ACE_Reactor::instance ()->register_handler (SIGINT, handleExit);
  ACE_Reactor::instance ()->register_handler (SIGTERM, handleExit);
  ACE_Reactor::instance ()->register_handler (SIGQUIT, handleExit);
  ACE_Reactor::instance ()->register_handler (SIGTSTP, handleExit);
  
  LOG_DEBUG ("Creating Pass gateway receiver...");
  
  PassGatewayReceiver *receiver = new PassGatewayReceiver ();
  
  LOG_DEBUG ("Creating gateway connector...");
  
  GatewayConnector *gatewayConnector = new GatewayConnector (receiver);

  // This must be the first call to getInstance(), so the receiver
  // and connector can be initialized when the singleton is created.
  PassConfigurationManager *config =
	  PassConfigurationManager::getInstance (receiver, gatewayConnector);
	  
  // Nothing further is done with 'config' since everything happens
  // in the constructor. This macro avoids the 'unused' warning.  
  ACE_UNUSED_ARG (config);
  
  // Connector is thread-safe so we can share one for
  // both incoming and outgoing.
  PassAmmmoPublisher::connector = gatewayConnector;
  
  LOG_DEBUG ("Creating new subscriber task...");

  // Spawn the subscriber task.
  PassSubscriberTask subscriber;
  subscriber.activate ();
  
  // Get the process-wide ACE_Reactor
  // (the one the acceptor should have registered with).
  ACE_Reactor *reactor = ACE_Reactor::instance ();
  LOG_DEBUG ("Starting event loop...");
  
  reactor->run_reactor_event_loop ();
  LOG_DEBUG ("Event loop terminated.");

  subscriber.close (0);
  LOG_DEBUG ("Waiting for subscription server to unsubscribe...");
  subscriber.wait ();

  return 0;
}
