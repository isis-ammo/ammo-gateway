#ifndef SAMPLE_PUSH_RECEIVER_H
#define SAMPLE_PUSH_RECEIVER_H

#include "GatewayConnector/GatewayConnector.h"

class SamplePushReceiver : public DataPushReceiverListener, public GatewayConnectorDelegate {
public:
  //GatewayConnectorDelegate methods
  virtual void onConnect(GatewayConnector *sender);
  virtual void onDisconnect(GatewayConnector *sender);
  
  //DataPushReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data);
};

#endif        //  #ifndef SAMPLE_PUSH_RECEIVER_H

