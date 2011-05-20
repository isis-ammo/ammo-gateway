#include <iostream>
#include <string>
#include <queue>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"

#include "ace/Acceptor.h"
#include "ace/Reactor.h"

#include "AndroidServiceHandler.h"

#include "log.h"
#include "version.h"

using namespace std;

string gatewayAddress;
int gatewayPort;

int main(int argc, char **argv) {
  LOG_INFO("AMMO Android Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects
  // during write)
  {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    // Register the handler for SIGINT
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);
  }
  
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
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  ACE_Acceptor<AndroidServiceHandler, ACE_SOCK_Acceptor> acceptor(serverAddress);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  LOG_DEBUG("Starting event loop...");
  reactor->run_reactor_event_loop();
  LOG_DEBUG("Event loop terminated.");
}
