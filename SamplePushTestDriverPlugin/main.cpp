#include <iostream>
#include <string>

#include "ace/Reactor.h"

//#include "SamplePushReceiver.h"
#include "GatewayConnector.h"

#include "version.h"
#include "log.h"

using namespace std;
using namespace ammo::gateway;

int main(int argc, char **argv) {  
  LOG_INFO("AMMO Sample Push Testdriver Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  cout << "Creating gateway connector..." << endl << flush;
    
  GatewayConnector *gatewayConnector = new GatewayConnector(NULL);

  gatewayConnector->pushData("foo", "text/plain", "baz");
 
#ifdef OLD 
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
#endif

  return 0;
}
