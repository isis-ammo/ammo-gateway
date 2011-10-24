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
#include "ace/Reactor.h"

#include "log.h"
#include "version.h"

#include "GatewayServiceHandler.h"
#include "GatewayConfigurationManager.h"
#include "GatewayCore.h"

#include <ace/OS_NS_pwd.h>
#include <ace/OS_NS_unistd.h>

using namespace std;

const char *AMMO_USER = "ammo-gateway";

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

void dropPrivileges() {
#if !defined(ACE_LACKS_PWD_FUNCTIONS) || !defined(ACE_LACKS_GETUID) || !defined(ACE_LACKS_SETUID)
  if(ACE_OS::getuid() == 0) {
    struct passwd *user = ACE_OS::getpwnam(AMMO_USER);
    if(user != NULL) {
      int result = ACE_OS::setgid(user->pw_gid); //have to switch gids first; a non-root user can't switch its own gid, apparently
      if(result == 0) {
        result = ACE_OS::setuid(user->pw_uid);
        if(result == 0) {
          LOG_TRACE("Successfully switched to ammo-gateway user.");
        } else {
          LOG_WARN("setuid failed when switching to daemon user " << AMMO_USER << " " << ACE_OS::last_error());
        }
      } else {
        LOG_WARN("setgid failed when switching to daemon user " << AMMO_USER << ACE_OS::last_error());
      }
    } else {
      LOG_DEBUG("Daemon user " << AMMO_USER << " could not be found.");
    }
  } else {
    LOG_DEBUG("Already running as an unprivileged user; not attempting to change to ammo-gateway");
  }
#endif
}

int main(int argc, char **argv) {
  LOG_INFO("AMMO Gateway Core (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  dropPrivileges();
  
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
