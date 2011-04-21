#include <iostream>
#include <string>

#include <ctime>

#include "ace/Reactor.h"
#include "GatewayConnector.h"

#include "soap/PASSPortBinding.nsmap"

#include "PassSubscriberTask.h"
#include "PassAmmmoPublisher.h"
#include "PassConfigurationManager.h"

#include "log.h"
#include "version.h"


using namespace std;

const string EVENT_MIME_TYPE = "application/vnd.edu.vu.isis.ammo.dash.event";
const string MEDIA_MIME_TYPE = "application/vnd.edu.vu.isis.ammo.dash.media";
const string EVENT_PULL_MIME_TYPE = "application/vnd.edu.vu.isis.ammo.map.object";

/* Signal handler */
static void signal_handler(int signum)
{
  std::cout << "SIGNAL (" << signum << ") caught. Terminating...." << std::endl << std::flush;;
}

int main(int argc, char **argv) {  
  LOG_INFO("AMMO PASS Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  
  // Set signal handler
  {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;

    // Register the handler for SIGINT
    sa.sa_handler = signal_handler;
    sigaction(SIGINT, &sa, 0);

    // Register the handler for SIGQUIT
    sa.sa_handler = signal_handler;
    sigaction(SIGQUIT, &sa, 0);

    // Register the handler for SIGTERM
    sa.sa_handler = signal_handler;
    sigaction(SIGTERM, &sa, 0);
  }
  
  PassConfigurationManager::getInstance();
  
  cout << "Creating gateway connector..." << endl << flush;
  
  GatewayConnector *gatewayConnector = new GatewayConnector(NULL);
  
  PassAmmmoPublisher::connector = gatewayConnector;
  
  cout << "Creating new subscriber task..." << endl << flush;
  PassSubscriberTask subscriber;
  subscriber.activate();
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
  cout << "Event loop terminated." << endl << flush;
  
  subscriber.close(0);
  cout << "Waiting for subscription server to unsubscribe..." << endl << flush;
  subscriber.wait();
  return 0;
}
