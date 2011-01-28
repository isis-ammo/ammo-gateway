#include <iostream>

#include "ace/Reactor.h"

#include "LocationStore.h"

using namespace std;

// Quick way to switch between plugin debugging, where this main() is run
// by hand, and normal operation.
#define DEBUG 0

int main(int argc, char **argv) {  
  cout << "Creating location store receiver..." << endl << flush;
  
  LocationStoreReceiver *pushReceiver = new LocationStoreReceiver();
  
#if DEBUG
	
  char *uri = const_cast<char *> ("http://battalion/company/platoon/squad.mil");
  char *mime_t = const_cast<char *> ("text/plain");
  std::vector<char> data (128, 'x');
	
  pushReceiver->onDataReceived (0, uri, mime_t, data);
	
  delete pushReceiver;
  return 0;
	
#else
	
  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
	
  gatewayConnector->registerDataInterest("text/plain", pushReceiver);
	
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
	
#endif
}
