#ifndef SAMPLE_PUSH_RECEIVER_H
#define SAMPLE_PUSH_RECEIVER_H

#include "GatewayConnector.h"

class SamplePushReceiver : public ammo::gateway::DataPushReceiverListener, public ammo::gateway::GatewayConnectorDelegate {
public:
  //GatewayConnectorDelegate methods
  virtual void onConnect(ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect(ammo::gateway::GatewayConnector *sender);
  
  //DataPushReceiverListener methods
  virtual void onDataReceived(ammo::gateway::GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser);
};

#endif        //  #ifndef SAMPLE_PUSH_RECEIVER_H

