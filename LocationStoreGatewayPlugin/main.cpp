#include <iostream>

#include "ace/Reactor.h"

#include "log.h"
#include "version.h"

#include "LocationStore.h"
#include "LocationStoreConfigManager.h"

#include "ace/Time_Value.h"
#include "ace/OS_NS_time.h"
#include "ace/Date_Time.h"

using namespace std;
using namespace ammo::gateway;

// Quick way to switch between plugin debugging, where this main() is run
// by hand, and normal operation.
#define DEBUG 0

int main (int argc, char **argv)
{
  LOG_INFO("AMMO Location Store Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  
  LOG_DEBUG ("Creating location store receiver...");
  
  LocationStoreReceiver *pushReceiver = new LocationStoreReceiver ();
  
#if DEBUG
	
  string uri ("http://battalion/company/platoon/squad.mil");
  string mime_t ("text/plain");
  string origin_user ("gi.joe@usarmy.mil");
  std::vector<char> data (128, 'x');
	
  pushReceiver->onDataReceived (0, uri, mime_t, data, origin_user);
	
  delete pushReceiver;
	
#else
	
  GatewayConnector *gatewayConnector = new GatewayConnector (pushReceiver);
	
  LocationStoreConfigManager *config =
	LocationStoreConfigManager::getInstance (pushReceiver, gatewayConnector);
	
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance ();
  LOG_DEBUG ("Starting event loop...");
  reactor->run_reactor_event_loop ();
	
#endif

  return 0;
}
