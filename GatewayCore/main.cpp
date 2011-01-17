#include <iostream>
#include <string>
#include <queue>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"

#include "ace/Acceptor.h"
#include "ace/Reactor.h"

#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>

#include "GatewayServiceHandler.h"
#include "GatewayConfigurationManager.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr logger(Logger::getLogger("edu.vu.isis.ammo.gateway.Core"));

int main(int argc, char **argv) {  
  NDC::push("main");
  LOG4CXX_INFO(logger, "GatewayCore starting");
  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects
  // during write)
  {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    // Register the handler for SIGINT
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);
  }
  
  GatewayConfigurationManager *config = GatewayConfigurationManager::getInstance();
  
  LOG4CXX_DEBUG(logger, "Creating acceptor...");
  
  //TODO: make interface and port number specifiable on the command line
  ACE_INET_Addr serverAddress(config->getGatewayPort(), config->getGatewayInterface().c_str());
  
  LOG4CXX_INFO(logger, "Listening on port " << serverAddress.get_port_number() << " on interface " << serverAddress.get_host_addr());
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  ACE_Acceptor<GatewayServiceHandler, ACE_SOCK_Acceptor> acceptor(serverAddress);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  LOG4CXX_DEBUG(logger, "Starting event loop...");
  reactor->run_reactor_event_loop();
  NDC::pop();
}
