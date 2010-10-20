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
class PullRequestReceiverListener;
class PullResponseReceiverListener;

class GatewayConnector {
public:
  GatewayConnector(GatewayConnectorDelegate *delegate);
  ~GatewayConnector();
  
  //General connection negotiation and bookkeeping
  bool associateDevice(std::string device, std::string user, std::string key);
  
  //--Data-Push support methods--

  //Sender-side
  bool pushData(std::string uri, std::string mimeType, const std::string &data);

  bool pullRequest(std::string requestUid, std::string pluginId,
		   std::string mimeType, std::string query,
		   std::string projection, unsigned int maxResults,
		   unsigned int startFromCount, bool liveQuery);

  bool pullResponse(std::string requestUid, std::string pluginId,
		    std::string mimeType, std::string uri,
		    std::vector<char>& data);


  //Receiver-side
  bool registerDataInterest(std::string mime_type, DataPushReceiverListener *listener);
  bool unregisterDataInterest(std::string mime_type);
  bool registerPullInterest(std::string mime_type, PullRequestReceiverListener *listener);
  bool unregisterPullInterest(std::string mime_type);
  bool registerPullResponseInterest(std::string mime_type, PullResponseReceiverListener *listener); /* local registration only */
  bool unregisterPullResponseInterest(std::string mime_type);
  
  void onAssociateResultReceived(const ammmo::gateway::protocol::AssociateResult &msg);
  void onPushDataReceived(const ammmo::gateway::protocol::PushData &msg);
  void onPullRequestReceived(const ammmo::gateway::protocol::PullRequest &msg);
  void onPullResponseReceived(const ammmo::gateway::protocol::PullResponse &msg);
  
private:
  GatewayConnectorDelegate *delegate;
  std::map<std::string, DataPushReceiverListener *> receiverListeners;
  std::map<std::string, PullRequestReceiverListener *> pullRequestListeners;
  std::map<std::string, PullResponseReceiverListener *> pullResponseListeners;

  ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector> *connector;
  GatewayServiceHandler *handler;
  
  bool connected;
};

class GatewayConnectorDelegate {
public:
  virtual void onConnect(GatewayConnector *sender) = 0;
  virtual void onDisconnect(GatewayConnector *sender) = 0;
  virtual void onAuthenticationResponse(GatewayConnector *sender, bool result);
};

class DataPushReceiverListener {
public:
  virtual void onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data) = 0;
};

class PullRequestReceiverListener {
public:
  virtual void onDataReceived(GatewayConnector *sender, 
			      std::string requestUid, std::string pluginId,
			      std::string mimeType, std::string query,
			      std::string projection, unsigned int maxResults,
			      unsigned int startFromCount, bool liveQuery) = 0;
};

class PullResponseReceiverListener {
public:
  virtual void onDataReceived(GatewayConnector *sender, 
			      std::string requestUid, std::string pluginId, std::string mimeType,
			      std::string uri, std::vector<char> &data) = 0;
};




#endif        //  #ifndef GATEWAY_CONNECTOR_H

