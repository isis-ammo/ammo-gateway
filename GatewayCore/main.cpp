#include <iostream>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"

#include "ace/Acceptor.h"

#include "ace/Reactor.h"

#include "GatewayServiceHandler.h"

using namespace std;

int main(int argc, char **argv) {  
  cout << "Creating acceptor..." << endl;
  
  //TODO: make interface and port number specifiable on the command line
  ACE_INET_Addr serverAddress(12475, "0.0.0.0");
  
  cout << "Listening on port " << serverAddress.get_port_number() << " on interface " << serverAddress.get_host_addr() << endl;
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  ACE_Acceptor<GatewayServiceHandler, ACE_SOCK_Acceptor> acceptor(serverAddress);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
}
