#include "GatewayCore.h"

#include "GatewayServiceHandler.h"

#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>

#include <iostream>

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

extern LoggerPtr logger; //statically declared in main.cpp

GatewayCore* GatewayCore::sharedInstance = NULL;

GatewayCore* GatewayCore::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayCore();
  }
  
  return sharedInstance;
}

bool GatewayCore::registerDataInterest(std::string mime_type, GatewayServiceHandler *handler) {
  NDC::push("Core");
  LOG4CXX_INFO(logger, "Registering interest in " << mime_type << " by handler " << handler);
  pushHandlers.insert(pair<string, GatewayServiceHandler *>(mime_type, handler));
  NDC::pop();
  return true;
}

bool GatewayCore::unregisterDataInterest(std::string mime_type, GatewayServiceHandler *handler) {
  NDC::push("Core");
  LOG4CXX_INFO(logger, "Unregistering interest in " << mime_type << " by handler " << handler);
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mime_type);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    if(handler == (*it).second) {
      pushHandlers.erase(it);
    }
  }
  NDC::pop();
  return true;
}

bool GatewayCore::registerPullInterest(std::string mime_type, GatewayServiceHandler *handler) {
  NDC::push("Core");
  LOG4CXX_INFO(logger, "Registering pull interest in " << mime_type << " by handler " << handler);
  pullHandlers.insert(pair<string, GatewayServiceHandler *>(mime_type, handler));
  NDC::pop();
  return true;
}

bool GatewayCore::unregisterPullInterest(std::string mime_type, GatewayServiceHandler *handler) {
  NDC::push("Core");
  LOG4CXX_INFO(logger, "Unregistering pull interest in " << mime_type << " by handler " << handler);
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pullHandlers.equal_range(mime_type);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    if(handler == (*it).second) {
      pullHandlers.erase(it);
    }
  }
  NDC::pop();
  return true;
}

bool GatewayCore::pushData(std::string uri, std::string mimeType, const std::string &data, std::string originUser) {
  NDC::push("Core");
  LOG4CXX_DEBUG(logger, "  Pushing data with uri: " << uri);
  LOG4CXX_DEBUG(logger, "                    type: " << mimeType);
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    (*it).second->sendPushedData(uri, mimeType, data, originUser);
  }
  NDC::pop();
  return true;
}

bool GatewayCore::pullRequest(std::string requestUid, std::string pluginId, std::string mimeType, 
                              std::string query, std::string projection, unsigned int maxResults, 
                              unsigned int startFromCount, bool liveQuery, GatewayServiceHandler *originatingPlugin) {
  NDC::push("Core");
  LOG4CXX_DEBUG(logger, "  Sending pull request with type: " << mimeType);
  LOG4CXX_DEBUG(logger, "                        pluginId: " << pluginId);
  LOG4CXX_DEBUG(logger, "                           query: " << query);
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pullHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    //check for something here?
    LOG4CXX_DEBUG(logger, "Sending request to " << (*it).second);
    (*it).second->sendPullRequest(requestUid, pluginId, mimeType, query, projection, maxResults, startFromCount, liveQuery);
  }
  
  //update plugin ID to the originating service handler that called this method
  plugins[pluginId] = originatingPlugin;
  NDC::pop();
  return true;
}

bool GatewayCore::pullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, const std::string& data) {
  NDC::push("Core");
  LOG4CXX_DEBUG(logger, "  Sending pull response with type: " << mimeType);
  LOG4CXX_DEBUG(logger, "                        pluginId: " << pluginId);

  map<string,GatewayServiceHandler *>::iterator it = plugins.find(pluginId);
  if ( it != plugins.end() ) {
    //check for something here?
    (*it).second->sendPullResponse(requestUid, pluginId, mimeType, uri, data);
  }
  NDC::pop();
  return true;
}

