#include "GatewayConnector.h"
#include "ace/Connector.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include <string>
#include <iostream>

using namespace std;

GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate) : connected(false), handler(NULL), delegate(delegate) {
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
    handler->setParentConnector(this);
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
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    return false;
  }
}

bool GatewayConnector::pushData(string uri, string mimeType, const string &data) {
  ammmo::gateway::protocol::GatewayWrapper msg;
  ammmo::gateway::protocol::PushData *pushMsg = msg.mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_data(data);
  
  msg.set_type(ammmo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  
  std::cout << "Sending Data Push message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    return false;
  }
}

bool GatewayConnector::registerDataInterest(string uri, DataPushReceiverListener *listener) {
  ammmo::gateway::protocol::GatewayWrapper msg;
  ammmo::gateway::protocol::RegisterDataInterest *di = msg.mutable_register_data_interest();
  di->set_uri(uri);
  
  msg.set_type(ammmo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST);
  
  std::cout << "Sending RegisterDataInterest message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    receiverListeners[uri] = listener;
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    std::cout << "Receiver listener was not registered; it won't receive any data." << std::endl << std::flush;
    return false;
  }
}

bool GatewayConnector::unregisterDataInterest(string uri) {
  return false;
}

void GatewayConnector::onAssociateResultReceived(const ammmo::gateway::protocol::AssociateResult &msg) {
  std::cout << "Got associate result of " << msg.result() << std::endl << std::flush;
  delegate->onAuthenticationResponse(this, msg.result());
}

void GatewayConnector::onPushDataReceived(const ammmo::gateway::protocol::PushData &msg) {
  string uri = msg.uri();
  string mimeType = msg.mime_type();
  vector<char> data(msg.data().begin(), msg.data().end());
  
  receiverListeners[uri]->onDataReceived(this, uri, mimeType, data);
}

//--GatewayConnectorDelegate default implementations (for optional delegate methods)

void GatewayConnectorDelegate::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  std::cout << "GatewayConnectorDelegate::onAuthenticationResponse : result = " << result << std::endl << std::flush;
}


