#include <iostream>
#include <string>

#include "ace/Reactor.h"

#include "ace/OS_NS_unistd.h" 
#include "ace/Signal.h" 

#include "SpotPushReceiver.h"
#include "GatewayConnector.h"

#include "log.h"
#include "version.h"

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
  LOG_INFO("AMMO Spot Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  
  SigintHandler * handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);
  
  cout << "Creating gateway connector..." << endl << flush;
  
  SpotPushReceiver *pushReceiver = new SpotPushReceiver();
  
  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
  
  gatewayConnector->registerDataInterest("application/vnd.edu.vu.isis.ammo.report.report_base", pushReceiver);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
  
  return 0;
}
