#include <iostream>
#include <string>
#include <queue>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"

#include "ace/OS_NS_unistd.h"
#include "ace/Signal.h"

#include "ace/Acceptor.h"
#include "ace/Select_Reactor.h"
#include "ace/Reactor.h"

#include "log.h"
#include "version.h"

#include "GatewayServiceHandler.h"
#include "GatewayConfigurationManager.h"
#include "GatewayCore.h"

#include "UserSwitch.inl"
#include "LogConfig.inl"

using namespace std;

//Handle SIGINT so the program can exit cleanly (otherwise, we just terminate
//in the middle of the reactor event loop, which isn't always a good thing).
class SigintHandler : public ACE_Event_Handler {
public:
  int handle_signal (int signum, siginfo_t * = 0, ucontext_t * = 0) {
    if (signum == SIGINT || signum == SIGTERM) {
      ACE_Reactor::instance()->end_reactor_event_loop();
    }
    return 0;
  }
};



int main(int argc, char **argv) {
  LOG_INFO("AMMO Gateway Core (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  dropPrivileges();
  setupLogging("GatewayCore");

  //Explicitly specify the ACE select reactor; on Windows, ACE defaults
  //to the WFMO reactor, which has radically different semantics and
  //violates assumptions we made in our code
  ACE_Select_Reactor selectReactor;
  ACE_Reactor newReactor(&selectReactor);
  auto_ptr<ACE_Reactor> delete_instance(ACE_Reactor::instance(&newReactor));
  
  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects
  // during write)
  ACE_Sig_Action no_sigpipe((ACE_SignalHandler) SIG_IGN);
  ACE_Sig_Action original_action;
  no_sigpipe.register_action(SIGPIPE, &original_action);
  
  SigintHandler * handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);
  
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
  LOG_DEBUG("Event loop terminated.");
  GatewayCore::getInstance()->terminate();
  return 0;
}
