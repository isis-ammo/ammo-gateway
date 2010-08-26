#include "GatewayConnector.h"
#include <string>

using namespace std;

GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate) {
  
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
