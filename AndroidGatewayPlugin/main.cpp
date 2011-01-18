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

#include "AndroidServiceHandler.h"

using namespace std;

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr logger(Logger::getLogger("ammo.gateway.AndroidGatewayPlugin"));

string gatewayAddress;
int gatewayPort;

int main(int argc, char **argv) {  
  GatewayConnector::setLoggerParentId("ammo.gateway.AndroidGatewayPlugin");
  NDC::push("main");
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
      LOG4CXX_FATAL(logger, "Usage: AndroidGatewayPlguin [--listenPort port] [--listenAddress address]");
      LOG4CXX_FATAL(logger, "  --listenPort port        Sets the listening port for the Android ");
      LOG4CXX_FATAL(logger, "                           interface (default 32869)");
      LOG4CXX_FATAL(logger, "  --listenAddress address  Sets the listening address for the Android");
      LOG4CXX_FATAL(logger, "                           interface (default 0.0.0.0, or all interfaces)");
      return 1;
    }
  }
  
  LOG4CXX_DEBUG(logger, "Creating acceptor...");
  
  //TODO: make interface and port number specifiable on the command line
  ACE_INET_Addr serverAddress(androidPort, androidAddress.c_str());
  
  LOG4CXX_INFO(logger, "Listening on port " << serverAddress.get_port_number() << " on interface " << serverAddress.get_host_addr());
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  ACE_Acceptor<AndroidServiceHandler, ACE_SOCK_Acceptor> acceptor(serverAddress);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  LOG4CXX_DEBUG(logger, "Starting event loop...");
  reactor->run_reactor_event_loop();
  NDC::pop();
}
