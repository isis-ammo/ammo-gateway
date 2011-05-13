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
  string mime_t ("application/vnd.edu.vu.isis.ammo.dash.event");
  string origin_user ("gi.joe@usarmy.mil");

  Json::Value value;
  value["uuid"] = "3q;4lkj34t:8b";
  value["mediaCount"] = 3;
  value["displayName"] = "Do you know who I am?";
  value["categoryId"] = "famous names";
  value["title"] = "lord mayor";
  value["description"] = "fat cat";
  value["longitude"] = 122.45;
  value["latitude"] = 67.89;
  value["createdDate"] = 1020000000;
  value["modifiedDate"] = 1030000000;
  
  std::string styled_string (value.toStyledString ());
  
  std::vector<char> data (styled_string.length ());
  ACE_OS::memcpy (data.get_allocator ().address (*data.begin ()),
                  styled_string.c_str (),
                  styled_string.length ());
	
  pushReceiver->onDataReceived (0, uri, mime_t, data, origin_user);
  
  value["longitude"] = 123.56;
  styled_string = value.toStyledString ();
  data.resize (styled_string.length ());
	
  ACE_OS::memcpy (data.get_allocator ().address (*data.begin ()),
                  styled_string.c_str (),
                  styled_string.length ());
	
  pushReceiver->onDataReceived (0, uri, mime_t, data, origin_user);
  
  std::string requestUid ("requestUid");
  std::string pluginId ("pluginId");
  std::string query (",,,,");
  std::string projection (",,,,,,,,123.11,,,,,,");
  
  pushReceiver->onDataReceived (0, requestUid, pluginId, mime_t, query, projection, 0, 0, false);
  
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
