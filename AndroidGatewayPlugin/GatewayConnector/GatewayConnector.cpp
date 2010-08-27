#include "GatewayConnector.h"
#include "ace/Connector.h"
#include <string>

using namespace std;

GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate) {
  ACE_INET_Addr serverAddress("localhost", 12475);
  connector = new ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector>();
  //connector->connect(&handler, serverAddress);
}

GatewayConnector::~GatewayConnector() {
  delete connector;
}
  
bool GatewayConnector::associateDevice(string device, string user, string key) {
  return false;
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
