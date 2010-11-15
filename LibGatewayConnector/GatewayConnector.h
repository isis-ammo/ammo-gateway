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
  /**
  * Creates a new GatewayConnector with the given GatewayConnectorDelegate and
  * establishes a connection to the gateway core.
  *
  * @param delegate A GatewayConnectorDelegate object to be used by this
  *                 GatewayConnector instance.  May be NULL (no delegate methods
  *                 will be called).
  */
  GatewayConnector(GatewayConnectorDelegate *delegate);
  ~GatewayConnector();
  
  //General connection negotiation and bookkeeping
  /**
  * Associates a device with this GatewayConnector in the gateway core.  This
  * method should only be needed by device connector plugins (e.g. the Android
  * Gateway Plugin).
  * 
  * @todo Authentication is not yet implemented in the gateway--  this method
  *       will send an authentication request, but the gateway will always
  *       will always return 'success' (and this method will always return
  *       true).  We should actually perform authentication here (pending more
  *       information from the security people).
  * 
  * @param device The unique ID of the device connecting to the gateway
  * @param user The unique ID of the user associated with the device connecting
  *             to the gateway.
  * @param key The authentication key/password for this user and device. Content
  *            of this parameter is still to be determined.
  * 
  * @return true if authentication was successful; false if authentication
  *         failed.
  */
  bool associateDevice(std::string device, std::string user, std::string key);
  
  //--Data-Push support methods--

  //Sender-side
  /**
  * Pushes a piece of data (with a particular uri and type) to the gateway.
  * 
  * Data pushed with this method can be received by listeners registered with
  * registerDataInterest.  All listeners registered for the type specified by
  * mimeType will receive this peice of data.
  * 
  * @param uri The URI of this piece of data.  This URI should be a universally
  *            unique identifier for the object being pushed (no two distinct
  *            peices of data should have the same URI).
  * @param mimeType The MIME type of this piece of data.  This MIME type is used
  *                 to determine which other gateway plugins will receive this
  *                 pushed data.
  * @param data The data to be pushed to the gateway.  Represented as a C++
  *             string; can contain any arbitrary binary data (the gateway will
  *             accept strings with bytes invalid in C-strings such as embedded
  *             nulls).
  * 
  * @return true if the operation succeeded; false if the operation failed.
  */
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

