#include "GatewayCore.h"

#include "GatewayServiceHandler.h"

#include <iostream>

using namespace std;

GatewayCore* GatewayCore::sharedInstance = NULL;

GatewayCore* GatewayCore::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayCore();
  }
  
  return sharedInstance;
}

bool GatewayCore::registerDataInterest(std::string mime_type, GatewayServiceHandler *handler) {
  cout << "Registering interest in " << mime_type << " by handler " << handler << endl << flush;
  pushHandlers.insert(pair<string, GatewayServiceHandler *>(mime_type, handler));
  return true;
}

bool GatewayCore::unregisterDataInterest(std::string mime_type, GatewayServiceHandler *handler) {
  cout << "Unregistering interest in " << mime_type << " by handler " << handler << endl << flush;
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mime_type);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    if(handler == (*it).second) {
      pushHandlers.erase(it);
    }
  }
  
  return true;
}

bool GatewayCore::registerPullInterest(std::string mime_type, GatewayServiceHandler *handler) {
  cout << "Registering pull interest in " << mime_type << " by handler " << handler << endl << flush;
  pullHandlers.insert(pair<string, GatewayServiceHandler *>(mime_type, handler));
  return true;
}

bool GatewayCore::unregisterPullInterest(std::string mime_type, GatewayServiceHandler *handler) {
  cout << "Unregistering pull interest in " << mime_type << " by handler " << handler << endl << flush;
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pullHandlers.equal_range(mime_type);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    if(handler == (*it).second) {
      pullHandlers.erase(it);
    }
  }
  
  return true;
}

bool GatewayCore::pushData(std::string uri, std::string mimeType, const std::string &data) {
  cout << "  Pushing data with uri: " << uri << endl;
  cout << "                    type: " << mimeType << endl << flush;
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    (*it).second->sendPushedData(uri, mimeType, data);
  }
  
  return true;
}

bool GatewayCore::pullRequest(std::string requestUid, std::string pluginId, std::string mimeType, 
                              std::string query, std::string projection, unsigned int maxResults, 
                              unsigned int startFromCount, bool liveQuery, GatewayServiceHandler *originatingPlugin) {
  cout << "  Sending pull request with type: " << mimeType << endl;
  cout << "                        pluginId: " << pluginId << endl;
  cout << "                           query: " << query << endl << flush;
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pullHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    //check for something here?
    cout << "Sending request to " << (*it).second << endl << flush;
    (*it).second->sendPullRequest(requestUid, pluginId, mimeType, query, projection, maxResults, startFromCount, liveQuery);
  }
  
  //update plugin ID to the originating service handler that called this method
  plugins[pluginId] = originatingPlugin;
  
  return true;
}

bool GatewayCore::pullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, const std::string& data) {
  cout << "  Sending pull response with type: " << mimeType << endl;
  cout << "                        pluginId: " << pluginId << endl;

  map<string,GatewayServiceHandler *>::iterator it = plugins.find(pluginId);
  if ( it != plugins.end() ) {
    //check for something here?
    (*it).second->sendPullResponse(requestUid, pluginId, mimeType, uri, data);
  }
  
  return true;
}

