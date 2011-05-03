#include <iostream>
#include <string>

#include "ace/Reactor.h"

#include "SamplePushReceiver.h"
#include "GatewayConnector.h"

#include "log.h"
#include "version.h"

using namespace std;
using namespace ammo::gateway;

int main(int argc, char **argv) {  
  LOG_INFO("AMMO Sample Push Receiver Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  
  cout << "Creating gateway connector..." << endl << flush;
  
  SamplePushReceiver *pushReceiver = new SamplePushReceiver();
  
  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
  
  gatewayConnector->registerDataInterest("text/plain", pushReceiver);
  
  for(;;) {
    gatewayConnector->pushData("someUrl", "sometype", "someData");
  }
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
  
  return 0;
}
