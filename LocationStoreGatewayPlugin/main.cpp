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
	
  string uri ("http://battalion/company/platoon/squad.mil");
  string mime_t ("text/plain");
  string origin_user ("gi.joe@usarmy.mil");
  std::vector<char> data (128, 'x');
	
  pushReceiver->onDataReceived (0, uri, mime_t, data, origin_user);
	
  delete pushReceiver;
	
#else
	
  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
	
  gatewayConnector->registerDataInterest("text/plain", pushReceiver);
	
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
	
#endif

  return 0;
}
