#include <iostream>
#include <string>
#include <queue>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"

#include "ace/Acceptor.h"
#include "ace/Reactor.h"

#include "log.h"
#include "version.h"

#include "GatewayServiceHandler.h"
#include "GatewayConfigurationManager.h"
#include "GatewayCore.h"

using namespace std;
int main(int argc, char **argv) {
  LOG_INFO("AMMO Gateway Core (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects
  // during write)
#ifndef _WIN32 //TODO:  Use the correct ACE paradigm for handling signals so both Win32 and *nix work correctly
  {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // Register the handler for SIGINT
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);
  }
#endif
  
  GatewayConfigurationManager *config = GatewayConfigurationManager::getInstance();
  
  LOG_DEBUG("Creating acceptor...");
  
  //TODO: make interface and port number specifiable on the command line
  ACE_INET_Addr serverAddress(config->getGatewayPort(), config->getGatewayInterface().c_str());
  
  LOG_INFO("Listening on port " << serverAddress.get_port_number() << " on interface " << serverAddress.get_host_addr());
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  ACE_Acceptor<GatewayServiceHandler, ACE_SOCK_Acceptor> acceptor(serverAddress);
  
  //Initializes the cross-gateway connections
  GatewayCore::getInstance()->initCrossGateway();
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  LOG_DEBUG("Starting event loop...");
  reactor->run_reactor_event_loop();
  
  return 0;
}
