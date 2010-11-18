#include <iostream>
#include <string>

#include "ace/Reactor.h"

#include "LocationStore.h"
#include "GatewayConnector.h"

using namespace std;

int main(int argc, char **argv) {  
  cout << "Creating gateway connector..." << endl << flush;
  
  LocationStoreReceiver *pushReceiver = new LocationStoreReceiver();
  
  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
  
  gatewayConnector->registerDataInterest("text/plain", pushReceiver);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
}
