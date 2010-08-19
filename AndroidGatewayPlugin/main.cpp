/**
* Work in progress:  this file currently just tests whether or not ACE, MWC, and
* Protocol Buffers are correctly installed.
*/

#include <iostream>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"

#include "protocol/Test.pb.h"

using namespace std;

int main(int argc, char **argv) {
  cout << "Hello World" << endl;
  
  Person p;
  p.set_id(5);
  p.set_name("Bob");
  
  cout << "Protocol Buffers Person: " << p.DebugString() << endl;
  
  ACE_SOCK_Connector connector;
  ACE_SOCK_Stream peer;
  ACE_INET_Addr peer_addr;
  
  if(peer_addr.set(12345, "localhost") == -1) {
    return 1;
  } else if(connector.connect(peer, peer_addr) == -1) {
    return 1;
  }
  
  string stringToSend = "Hello";
  
  peer.send_n(stringToSend.c_str(), stringToSend.length() + 1);
  
  peer.close();
}
