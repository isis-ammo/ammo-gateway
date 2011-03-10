#include "GatewayCore.h"

#include "GatewayServiceHandler.h"

#include "log.h"

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
  LOG_INFO("Registering interest in " << mime_type << " by handler " << handler);
  
  //see if there's already a subscription to this type for this handler
  bool foundSubscription = false;
  multimap<string, LocalSubscriptionInfo>::iterator it;
  pair<multimap<string, LocalSubscriptionInfo>::iterator, multimap<string, LocalSubscriptionInfo>::iterator> subscriptionIterators;
  
  subscriptionIterators = pushHandlers.equal_range(mime_type);
  
  for(it = subscriptionIterators.first; it != subscriptionIterators.second; it++) {
    if(handler == it->second.handler) {
      it->second.references++;
      foundSubscription = true;
      break; //found a subscription; there should only be one per handler
    }
  }
  
  if(!foundSubscription) {
    LocalSubscriptionInfo newSubscription;
    newSubscription.handler = handler;
    newSubscription.references = 1;
    pushHandlers.insert(pair<string, LocalSubscriptionInfo>(mime_type, newSubscription));
  }
  return true;
}

bool GatewayCore::unregisterDataInterest(std::string mime_type, GatewayServiceHandler *handler) {
  LOG_INFO("Unregistering interest in " << mime_type << " by handler " << handler);
  multimap<string, LocalSubscriptionInfo>::iterator it;
  pair<multimap<string, LocalSubscriptionInfo>::iterator,multimap<string, LocalSubscriptionInfo>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mime_type);
  
  for(it = handlerIterators.first; it != handlerIterators.second;) {
    //need to increment the iterator *before* we erase it, because erasing it
    //invalidates the iterator (it doesn't invalidate other iterators in the list,
    //though)
    multimap<string, LocalSubscriptionInfo>::iterator eraseIter = it++;
    
    if(handler == (*eraseIter).second.handler) {
      //LOG_TRACE("Removing an element");
      it->second.references--;
      if(it->second.references == 0) {
        LOG_TRACE("Removed last reference to this subscription for this handler.");
        pushHandlers.erase(eraseIter);
      }
      return true;
    }
  }
  
  LOG_WARN("Tried to unsubscribe without an existing subscription");
  return false;
}

bool GatewayCore::registerPullInterest(std::string mime_type, GatewayServiceHandler *handler) {
  LOG_INFO("Registering pull interest in " << mime_type << " by handler " << handler);
  pullHandlers.insert(pair<string, GatewayServiceHandler *>(mime_type, handler));
  return true;
}

bool GatewayCore::unregisterPullInterest(std::string mime_type, GatewayServiceHandler *handler) {
  LOG_INFO("Unregistering pull interest in " << mime_type << " by handler " << handler);
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pullHandlers.equal_range(mime_type);
  
  for(it = handlerIterators.first; it != handlerIterators.second;) {
    multimap<string,GatewayServiceHandler *>::iterator eraseIter = it++;
    
    if(handler == (*it).second) {
      pullHandlers.erase(it);
    }
  }
  return true;
}

bool GatewayCore::pushData(std::string uri, std::string mimeType, const std::string &data, std::string originUser) {
  LOG_DEBUG("  Pushing data with uri: " << uri);
  LOG_DEBUG("                    type: " << mimeType);
  multimap<string, LocalSubscriptionInfo>::iterator it;
  pair<multimap<string, LocalSubscriptionInfo>::iterator,multimap<string, LocalSubscriptionInfo>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    (*it).second.handler->sendPushedData(uri, mimeType, data, originUser);
  }
  return true;
}

bool GatewayCore::pullRequest(std::string requestUid, std::string pluginId, std::string mimeType, 
                              std::string query, std::string projection, unsigned int maxResults, 
                              unsigned int startFromCount, bool liveQuery, GatewayServiceHandler *originatingPlugin) {
  LOG_DEBUG("  Sending pull request with type: " << mimeType);
  LOG_DEBUG("                        pluginId: " << pluginId);
  LOG_DEBUG("                           query: " << query);
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pullHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    //check for something here?
    LOG_DEBUG("Sending request to " << (*it).second);
    (*it).second->sendPullRequest(requestUid, pluginId, mimeType, query, projection, maxResults, startFromCount, liveQuery);
  }
  
  //update plugin ID to the originating service handler that called this method
  plugins[pluginId] = originatingPlugin;
  return true;
}

bool GatewayCore::pullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, const std::string& data) {
  LOG_DEBUG("  Sending pull response with type: " << mimeType);
  LOG_DEBUG("                        pluginId: " << pluginId);

  map<string,GatewayServiceHandler *>::iterator it = plugins.find(pluginId);
  if ( it != plugins.end() ) {
    //check for something here?
    (*it).second->sendPullResponse(requestUid, pluginId, mimeType, uri, data);
  }
  return true;
}

