#include <iostream>
#include <string>

#include <ctime>

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

int main(int argc, char **argv) {  
  TigrConfigurationManager *config = TigrConfigurationManager::getInstance();
  
  cout << "Creating gateway connector..." << endl << flush;
  
  TigrPushReceiver *pushReceiver = new TigrPushReceiver();
  
  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
  
  cout << "Registering interest in " << EVENT_MIME_TYPE << endl << flush;
  gatewayConnector->registerDataInterest(EVENT_MIME_TYPE, pushReceiver);
  
  cout << "Registering interest in " << MEDIA_MIME_TYPE << endl << flush;
  gatewayConnector->registerDataInterest(MEDIA_MIME_TYPE, pushReceiver);
  
  cout << "Registering interest in " << EVENT_PULL_MIME_TYPE << endl << flush;
  gatewayConnector->registerPullInterest(EVENT_PULL_MIME_TYPE, pushReceiver);

  // TAB ADDED
  cout << "Registering interest in " << REPORT_MIME_TYPE << endl << flush;
  gatewayConnector->registerDataInterest(REPORT_MIME_TYPE, pushReceiver);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
}