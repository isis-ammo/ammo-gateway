#include "GatewayConnector.h"
#include "ace/Connector.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include <string>
#include <iostream>

using namespace std;

GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate) : connected(false), handler(NULL) {
  ACE_INET_Addr serverAddress(12475, "127.0.0.1");
  connector = new ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector>();
  int status = connector->connect(handler, serverAddress);
  if(status == -1) {
    std::cout << "connection failed" << std::endl << std::flush;
    std::cout << "errno: " << errno << std::endl << std::flush;
    std::cout << "error: " << strerror(errno) << std::endl << std::flush;
    connected = false;
  } else {
    connected = true;
  }
  if(handler == NULL) {
    std::cout << "Handler not created by ACE_Connector" << std::endl << std::flush;
  } else {
    std::cout << "Gateway service handler created by ACE_Connector" << std::endl << std::flush;
  }
}

GatewayConnector::~GatewayConnector() {
  std::cout << "Deleting GatewayConnector()" << std::endl << std::flush;
  handler->close();
  connector->close();
  delete connector;
}
  
bool GatewayConnector::associateDevice(string device, string user, string key) {
  ammmo::gateway::protocol::GatewayWrapper msg;
  ammmo::gateway::protocol::AssociateDevice *associateMsg = msg.mutable_associate_device();
  associateMsg->set_device(device);
  associateMsg->set_user(user);
  associateMsg->set_key(key);
  
  msg.set_type(ammmo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_DEVICE);
  
  std::cout << "Sending Associate Device message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
  }
  
  return true;
}

bool GatewayConnector::pushData(string uri, string mimeType, vector<char> &data) {
  return false;
}

bool GatewayConnector::registerDataInterest(string uri, DataPushReceiverListener *listener) {
  return false;
}

bool GatewayConnector::unregisterDataInterest(string uri) {
  return false;
}
