#ifndef GATEWAY_CONNECTOR_H
#define GATEWAY_CONNECTOR_H

#include <string>
#include <vector>
#include <map>
#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"

#include "GatewayServiceHandler.h"

class GatewayConnectorDelegate;
class DataPushReceiverListener;

class GatewayConnector {
public:
  GatewayConnector(GatewayConnectorDelegate *delegate);
  ~GatewayConnector();
  
  //General connection negotiation and bookkeeping
  bool associateDevice(std::string device, std::string user, std::string key);
  
  //--Data-Push support methods--
  //Sender-side
  bool pushData(std::string uri, std::string mimeType, std::vector<char> &data);
  //Receiver-side
  bool registerDataInterest(std::string uri, DataPushReceiverListener *listener);
  bool unregisterDataInterest(std::string uri);
  
private:
  GatewayConnectorDelegate *delegate;
  std::map<std::string, DataPushReceiverListener *> receiverListeners;
  ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector> *connector;
  GatewayServiceHandler handler;
};

class GatewayConnectorDelegate {
  virtual void onConnect(GatewayConnector &sender) = 0;
  virtual void onDisconnect(GatewayConnector &sender) = 0;
};

class DataPushReceiverListener {
public:
  virtual void onDataReceived(GatewayConnector &sender, std::string uri, std::vector<char> &data) = 0;
};

#endif        //  #ifndef GATEWAY_CONNECTOR_H

