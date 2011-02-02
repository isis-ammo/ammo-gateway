
#include "AtsHandler.h"

#include <iostream>
#include <string>

void AtsHandler::onConnect(GatewayConnector *sender) {
  
}

void AtsHandler::onDisconnect(GatewayConnector *sender) {
  
}

/**
* In response to push data (DataPushReceiverListener)
* 
* types: 
*  request for chat room construction
*
*/
void AtsHandler::onDataReceived(GatewayConnector *sender, 
                                std::string uri, 
                                std::string mimeType,
                                std::vector< char > &data,
                                std::string originUsername)
{
  std::cout << "Got push data." << std::endl;
  std::cout << "  URI: " << uri << std::endl;
  std::cout << "  Mime type: " << mimeType << std::endl;
  std::cout << "  Data: " << std::string(data.begin(), data.end()) << std::endl;
  std::cout << "  Origin User Name: " << originUsername << std::endl;
}
 

  // PullRequestReceiverListener methods
void AtsHandler::onDataReceived(GatewayConnector *sender,
                              std::string requestUid,
                              std::string pluginId,
                              std::string mimeType, 
                              std::string query,
                              std::string projection,
                              unsigned int maxResults,
                              unsigned int startFromCount,
                              bool liveQuery)
{
  std::cout << "Got pull request data." << std::endl;
  std::cout << "  ReqId: " << requestUid << std::endl;
  std::cout << "  Plugin: " << pluginId << std::endl;
  std::cout << "  Mime type: " << mimeType << std::endl;
  std::cout << "  Query: " << query << std::endl;
  std::cout << "  Projection: " << projection << std::endl;
  std::cout << "  Start Count: " << startFromCount << std::endl;
  std::cout << "  Live: " << liveQuery << std::endl;
}

  // PullResponseReceiverListener
void AtsHandler::onDataReceived (GatewayConnector *sender,
                               std::string requestUid,
                               std::string pluginId,
                               std::string mimeType, 
                               std::string uri,
                               std::vector< char > &data)
{
  std::cout << "Got pull response data." << std::endl;
  std::cout << "  ReqId: " << requestUid << std::endl;
  std::cout << "  Plugin: " << pluginId << std::endl;
  std::cout << "  Mime type: " << mimeType << std::endl;
  std::cout << "  Uri: " << uri << std::endl;
  std::cout << "  Data: " << std::string(data.begin(), data.end()) << std::endl;
}

