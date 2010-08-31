/**
* Work in progress:  this file currently just tests whether or not ACE, MWC, and
* Protocol Buffers are correctly installed.
*/

#include <iostream>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"

#include "ace/Acceptor.h"

#include "ace/Reactor.h"

#include "AndroidServiceHandler.h"

using namespace std;

int main(int argc, char **argv) {
  cout << "ACE_MESSAGE_QUEUE TEST" << endl;
  
  ACE_Message_Queue<ACE_NULL_SYNCH> *queue = new ACE_Message_Queue<ACE_NULL_SYNCH>();
  
  unsigned int messageSize = 5;
  char *messageToSend = new char[messageSize];
  unsigned int messageChecksum = 79;
  
  ACE_Message_Block *messageSizeBlock = new ACE_Message_Block(sizeof(messageSize));
  queue->enqueue_tail(messageSizeBlock);
  
  cout << "Creating acceptor..." << endl;
  
  //TODO: make interface and port number specifiable on the command line
  ACE_INET_Addr serverAddress(32869, "0.0.0.0");
  
  cout << "Listening on port " << serverAddress.get_port_number() << " on interface " << serverAddress.get_host_addr() << endl;
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  ACE_Acceptor<AndroidServiceHandler, ACE_SOCK_Acceptor> acceptor(serverAddress);
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  cout << "Starting event loop..." << endl << flush;
  reactor->run_reactor_event_loop();
}
