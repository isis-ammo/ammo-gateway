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

using namespace std;

string gatewayAddress;
int gatewayPort;

int main(int argc, char **argv) {  
  gatewayAddress = "127.0.0.1";
  gatewayPort = 12475;
  
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
      cout << "Usage: AndroidGatewayPlguin [--listenPort port] [--listenAddress address]" << endl;
      cout << endl;
      cout << "  --listenPort port        Sets the listening port for the Android " << endl;
      cout << "                           interface (default 12475)" << endl;
      cout << "  --listenAddress address  Sets the listening address for the Android" << endl;
      cout << "                           interface (default 0.0.0.0, or all interfaces)" << endl << flush;
      return 1;
    }
  }
  
  cout << "Creating acceptor..." << endl;
  
  //TODO: make interface and port number specifiable on the command line
  ACE_INET_Addr serverAddress(androidPort, androidAddress.c_str());
  
  cout << "Listening on port " << serverAddress.get_port_number() << " on interface " << serverAddress.get_host_addr() << endl;
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  ACE_Acceptor<AndroidServiceHandler, ACE_SOCK_Acceptor> acceptor(serverAddress);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
}
