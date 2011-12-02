#include <iostream>
#include <string>
#include <queue>

#include <dns_sd.h>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"

#include "ace/OS_NS_unistd.h"
#include "ace/Signal.h"

#include "ace/Acceptor.h"
#include "ace/Reactor.h"
#include "ace/Select_Reactor.h"

#include "AndroidServiceHandler.h"

#include "log.h"
#include "version.h"

#include "UserSwitch.inl"

using namespace std;

string gatewayAddress;
int gatewayPort;

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

/**
* Dependency:  libavahi-compat-libdnssd-dev
*/
void cb(DNSServiceRef sdRef,
        DNSServiceFlags flags,
        DNSServiceErrorType errorCode,
        const char *serviceName,
        const char *regtype,
        const char *replyDomain,
        void *context) {
  printf("called!\n");
}

void registerMdnsService(DNSServiceRef &sdRef, uint16_t portNumber) {
  LOG_DEBUG("Registering service with MDNS responder...");
  DNSServiceRegister(&sdRef,               //sdRef (an *uninitialized* DNSServiceRef instance)
                     0,                   //flags (0 uses default name conflict behavior--  add a number to end of name)
                     0,                   //interfaceIndex (0 accepts default)
                     NULL,                //name (NULL uses computer name)
                     "_ammogateway._tcp", //regtype (_servicename._transport
                     NULL,                //domain (NULL uses default domain)
                     NULL,                //host (NULL uses default name)
                     htons(portNumber),   //port (port number we're listening on, in network order
                     0,                   //txtLen (length of txt record: must be zero if txtRecord is null)
                     0,                   //txtRecord (NULL or properly formatted DNS txt record)
                     cb,                  //DNSSServiceRegisterReply callBack (function to be called on success or failure)
                     0);                  //context (application context pointer to be passed to callback function)
  LOG_DEBUG("Waiting for response from MDNS responder...");
  DNSServiceErrorType error = DNSServiceProcessResult(sdRef);
  if(error == kDNSServiceErr_NoError) {
    LOG_INFO("Successfully registered service.");
  } else {
    LOG_ERROR("DNS-SD service registration failed...  error " << error);
  }
}

void unregisterMdnsService(DNSServiceRef &sdRef) {
  LOG_DEBUG("Destroying DNS service reference...")
  DNSServiceRefDeallocate(sdRef);
}

int main(int argc, char **argv) {
  LOG_INFO("AMMO Android Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  dropPrivileges();

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
  
  string androidAddress = "0.0.0.0";
  int androidPort = 32869;
  
  queue<string> argumentQueue;
  for(int i=1; i < argc; i++) {
    argumentQueue.push(string(argv[i]));
  }
  
  while(!argumentQueue.empty()) {
    string arg = argumentQueue.front();
    argumentQueue.pop();
    
    if(arg == "--listenPort" && argumentQueue.size() >= 1) {
      string param = argumentQueue.front();
      argumentQueue.pop();
      androidPort = atoi(param.c_str());
    } else if(arg == "--listenAddress" && argumentQueue.size() >= 1) {
      string param = argumentQueue.front();
      argumentQueue.pop();
      androidAddress = param;
    } else {
      LOG_FATAL("Usage: AndroidGatewayPlguin [--listenPort port] [--listenAddress address]");
      LOG_FATAL("  --listenPort port        Sets the listening port for the Android ");
      LOG_FATAL("                           interface (default 32869)");
      LOG_FATAL("  --listenAddress address  Sets the listening address for the Android");
      LOG_FATAL("                           interface (default 0.0.0.0, or all interfaces)");
      return 1;
    }
  }
  
  LOG_DEBUG("Creating acceptor...");
  
  //TODO: make interface and port number specifiable on the command line
  ACE_INET_Addr serverAddress(androidPort, androidAddress.c_str());
  
  LOG_INFO("Listening on port " << serverAddress.get_port_number() << " on interface " << serverAddress.get_host_addr());
  
  DNSServiceRef sdRef;
  registerMdnsService(sdRef, serverAddress.get_port_number());
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  ACE_Acceptor<AndroidServiceHandler, ACE_SOCK_Acceptor> acceptor(serverAddress);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  LOG_DEBUG("Starting event loop...");
  reactor->run_reactor_event_loop();
  LOG_DEBUG("Event loop terminated.");
  unregisterMdnsService(sdRef);
}
