#ifndef ATS_HANDLER_H
#define ATS_HANDLER_H

#include <curl/curl.h>

#include "GatewayConnector.h"
#include "AtsMessageTypes.h"
#include "AtsConfigMgr.h"

class AtsHandler : 
  public ammo::gateway::DataPushReceiverListener, 
  public ammo::gateway::PullRequestReceiverListener,
  public ammo::gateway::PullResponseReceiverListener,
  public ammo::gateway::GatewayConnectorDelegate 
{
public:
  AtsHandler(); 
  //GatewayConnectorDelegate methods
  virtual void onConnect(ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect(ammo::gateway::GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onDataReceived(ammo::gateway::GatewayConnector *sender,
                              std::string uri,
                              std::string mimeType,
                              std::vector<char> &data,
                              std::string originUser);

  // PullRequestReceiverListener methods
  virtual void onDataReceived(ammo::gateway::GatewayConnector *sender,
                              std::string requestUid,
                              std::string pluginId,
                              std::string mimeType, 
                              std::string query,
                              std::string projection,
                              unsigned int maxResults,
                              unsigned int startFromCount,
                              bool liveQuery);

  // PullResponseReceiverListener
  virtual void onDataReceived (ammo::gateway::GatewayConnector *sender,
                               std::string requestUid,
                               std::string pluginId,
                               std::string mimeType, 
                               std::string uri,
                               std::vector< char > &data);
private:
  char* baseServerAddr;
  AtsConfigMgr* config;
  std::pair<std::string, std::string> credentials;

  std::string uploadMedia(CURL *curl, std::string mediaType, std::vector< char >& payload ); 
  std::string inviteChat(CURL *curl, std::string mediaType, std::vector< char >& payload ); 
  std::vector<char> listChannels(CURL *curl, std::string dataType, std::string query );

  std::vector<char> listPeople(CURL *curl, std::string dataType, std::string query ); 
  std::vector<char> listPeople(CURL *curl, std::string dataType, std::vector<char>& query ); 
  std::string channelCreate(CURL *curl, std::string dataType, std::vector< char >& payload ); 

  std::string centerMap(CURL *curl, std::string dataType, std::vector< char > &query ); 

  std::string postLocation(CURL *curl, std::string mediaType, std::vector< char >& payload ); 
  std::string postLocations(CURL *curl, std::string mediaType, std::vector< char >& payload ); 
};

#endif        //  #ifndef ATS_HANDLER_H

