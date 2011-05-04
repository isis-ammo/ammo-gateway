#include "SamplePushReceiver.h"

#include <iostream>
#include <string>

using namespace std;
using namespace ammo::gateway;

void SamplePushReceiver::onConnect(GatewayConnector *sender) {
  
}

void SamplePushReceiver::onDisconnect(GatewayConnector *sender) {
  
}

void SamplePushReceiver::onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData) {
  cout << "Got data." << endl;
  cout << "  URI: " << pushData.uri << endl;
  cout << "  Mime type: " << pushData.mimeType << endl;
  cout << "  Data: " << string(pushData.data.begin(), pushData.data.end()) << endl;
  cout << "  Origin Username: " << pushData.originUsername << endl;
}
