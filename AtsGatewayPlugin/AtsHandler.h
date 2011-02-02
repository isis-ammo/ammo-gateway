#ifndef ATS_HANDLER_H
#define ATS_HANDLER_H

#include "GatewayConnector.h"
#include "AtsMessageTypes.h"

class AtsHandler : 
  public DataPushReceiverListener, 
  public PullRequestReceiverListener,
  public PullResponseReceiverListener,
  public GatewayConnectorDelegate 
{
public:
  //GatewayConnectorDelegate methods
  virtual void onConnect(GatewayConnector *sender);
  virtual void onDisconnect(GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender,
                              std::string uri,
                              std::string mimeType,
                              std::vector<char> &data,
                              std::string originUser);

  // PullRequestReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender,
                              std::string requestUid,
                              std::string pluginId,
                              std::string mimeType, 
                              std::string query,
                              std::string projection,
                              unsigned int maxResults,
                              unsigned int startFromCount,
                              bool liveQuery);

  // PullResponseReceiverListener
  virtual void onDataReceived (GatewayConnector *sender,
                               std::string requestUid,
                               std::string pluginId,
                               std::string mimeType, 
                               std::string uri,
                               std::vector< char > &data);
};

#endif        //  #ifndef ATS_HANDLER_H

