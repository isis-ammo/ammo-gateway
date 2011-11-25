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

#include "SerialServiceHandler.h"

#include "log.h"
#include "version.h"

#include "UserSwitch.inl"

using namespace std;

string gatewayAddress;
int gatewayPort;

extern int  serial_receiver( int argc, char *argv[], void ( *process_message) ( int sender, int size, char *full_buffer ) );



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
  LOG_INFO("AMMO Serial Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  dropPrivileges();
  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects
  // during write)
  ACE_Sig_Action no_sigpipe((ACE_SignalHandler) SIG_IGN);
  ACE_Sig_Action original_action;
  no_sigpipe.register_action(SIGPIPE, &original_action);
  
  SigintHandler * handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);
  
  string androidAddress = "/dev/ttyUSB0";
  
  queue<string> argumentQueue;
  for(int i=1; i < argc; i++) {
    argumentQueue.push(string(argv[i]));
  }
  
  while(!argumentQueue.empty()) {
    string arg = argumentQueue.front();
    argumentQueue.pop();
    
    if(arg == "--listenAddress" && argumentQueue.size() >= 1) {
      string param = argumentQueue.front();
      argumentQueue.pop();
      androidAddress = param;
    } else {
      LOG_FATAL("Usage: SerialGatewayPlguin [--listenPort port] [--listenAddress address]");
      LOG_FATAL("  --listenAddress address  Sets the listening address for the Serial");
      LOG_FATAL("                           interface (default /dev/ttyUSB0)");
      return 1;
    }
  }
  
  LOG_DEBUG("Creating service handler which receives and routes to gateway via the GatewayConnector");
  SerialServiceHandler *svcHandler = new SerialServiceHandler();
  svcHandler->open( (void *)(androidAddress.c_str()) );

  LOG_DEBUG("Receiving message receiver - blocking call ");
  svcHandler->receiveData();
}
