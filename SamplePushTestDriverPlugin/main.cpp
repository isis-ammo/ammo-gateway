#include <iostream>
#include <string>

#include "ace/Select_Reactor.h"
#include "ace/Reactor.h"

#include "ace/OS_NS_unistd.h" 
#include "ace/Signal.h" 

//#include "SamplePushReceiver.h"
#include "GatewayConnector.h"

#include "version.h"
#include "log.h"
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

int main(int argc, char **argv) {  
  LOG_INFO("AMMO Sample Push Testdriver Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  setupLogging();
  
  //Explicitly specify the ACE select reactor; on Windows, ACE defaults
  //to the WFMO reactor, which has radically different semantics and
  //violates assumptions we made in our code
  ACE_Select_Reactor selectReactor;
  ACE_Reactor newReactor(&selectReactor);
  auto_ptr<ACE_Reactor> delete_instance(ACE_Reactor::instance(&newReactor));
  
  SigintHandler * handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);
  
  cout << "Creating gateway connector..." << endl << flush;
    
  GatewayConnector *gatewayConnector = new GatewayConnector(NULL);
  
  
  PushData pushData;
  pushData.uri = "foo";
  pushData.mimeType = "text/plain";
  pushData.data = "baz";
  gatewayConnector->pushData(pushData);
 
#ifdef OLD 
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
#endif

  return 0;
}
