#include <iostream>
#include <string>

#include <ctime>

#include "log.h"

#include "ace/Reactor.h"

#include "TigrPushReceiver.h"
#include "GatewayConnector.h"

#include "soap/ContentServiceSoapBinding.nsmap"

#include "TigrConfigurationManager.h"

using namespace std;

const string EVENT_MIME_TYPE = "application/vnd.edu.vu.isis.ammo.collector.event";
const string MEDIA_MIME_TYPE = "application/vnd.edu.vu.isis.ammo.collector.media";
const string EVENT_PULL_MIME_TYPE = "application/vnd.edu.vu.isis.ammo.map.object";
const string REPORT_MIME_TYPE = "application/vnd.edu.vu.isis.ammo.report.report_base";

/**
  Data Interest is expressed when the service subscribes to messages of a type.
  Pull Interest expresses the ability to service pull requests.
*/
int main(int argc, char **argv) {  
  TigrConfigurationManager *config = TigrConfigurationManager::getInstance();
  
  LOG_DEBUG("Creating gateway connector...");
  
  TigrPushReceiver *pushReceiver = new TigrPushReceiver();
  
  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
  
  LOG_DEBUG("Registering interest in " << EVENT_MIME_TYPE);
  gatewayConnector->registerDataInterest(EVENT_MIME_TYPE, pushReceiver);
  
  LOG_DEBUG("Registering interest in " << MEDIA_MIME_TYPE);
  gatewayConnector->registerDataInterest(MEDIA_MIME_TYPE, pushReceiver);
  
  LOG_DEBUG("Registering interest in " << EVENT_PULL_MIME_TYPE);
  gatewayConnector->registerPullInterest(EVENT_PULL_MIME_TYPE, pushReceiver);

  LOG_DEBUG("Registering interest in " << REPORT_MIME_TYPE);
  gatewayConnector->registerDataInterest(REPORT_MIME_TYPE, pushReceiver);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  LOG_DEBUG("Starting event loop...");
  reactor->run_reactor_event_loop();
}
