#include "GatewayCore.h"

#include "GatewayServiceHandler.h"
#include "CrossGatewayServiceHandler.h"

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
  pushHandlers.insert(pair<string, GatewayServiceHandler *>(mime_type, handler));
  return true;
}

bool GatewayCore::unregisterDataInterest(std::string mime_type, GatewayServiceHandler *handler) {
  LOG_INFO("Unregistering interest in " << mime_type << " by handler " << handler);
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mime_type);
  
  for(it = handlerIterators.first; it != handlerIterators.second;) {
    //need to increment the iterator *before* we erase it, because erasing it
    //invalidates the iterator (it doesn't invalidate other iterators in the list,
    //though)
    multimap<string,GatewayServiceHandler *>::iterator eraseIter = it++;
    
    if(handler == (*eraseIter).second) {
      //LOG_TRACE("Removing an element");
      pushHandlers.erase(eraseIter);
      break;
    }
  }
  return true;
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
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    (*it).second->sendPushedData(uri, mimeType, data, originUser);
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

void GatewayCore::initCrossGateway() {
  
}
  
bool GatewayCore::registerCrossGatewayConnection(std::string handlerId, CrossGatewayServiceHandler *handler) {
  LOG_DEBUG("Registering cross-gateway handler " << handlerId);
  crossGatewayHandlers[handlerId] = handler;
  return true;
}

bool GatewayCore::subscribeCrossGateway(std::string mimeType, std::string originHandlerId) {
  LOG_DEBUG("Got subscription to type " << mimeType << " for handler " << originHandlerId);
  //see if there's already a subscription to this type for this handler
  bool foundSubscription = false;
  multimap<string, SubscriptionInfo>::iterator it;
  pair<multimap<string, SubscriptionInfo>::iterator,multimap<string,SubscriptionInfo>::iterator> subscriptionIterators;
  
  subscriptionIterators = subscriptions.equal_range(mimeType);
  
  for(it = subscriptionIterators.first; it != subscriptionIterators.second; it++) {
    if(originHandlerId == (*it).second.handlerId) {
      (*it).second.references++;
      foundSubscription = true;
    }
  }
  
  if(!foundSubscription) {
    //if we get here, we don't already have an entry for this handler in the table
    SubscriptionInfo newSubscription;
    newSubscription.handlerId = originHandlerId;
    newSubscription.references = 1;
    
    subscriptions.insert(pair<string, SubscriptionInfo>(mimeType, newSubscription));
  }
  
  //now propogate the subscription to all the other gateway nodes, except the one it came from
  for(map<string, CrossGatewayServiceHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
    if(it->first != originHandlerId) {
      it->second->sendSubscribeMessage(mimeType);
    }
  }
  
  return true;
}
bool GatewayCore::unsubscribeCrossGateway(std::string mimeType, std::string originHandlerId) {
  LOG_DEBUG("Handler " << originHandlerId << " unsubscribing from type " << mimeType);
  //propogate the unsubscribe to all the other gateway nodes, except the one it came from
  for(map<string, CrossGatewayServiceHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
    if(it->first != originHandlerId) {
      it->second->sendUnsubscribeMessage(mimeType);
    }
  }
  
  //look for an existing subscription to this type
  multimap<string, SubscriptionInfo>::iterator it;
  pair<multimap<string, SubscriptionInfo>::iterator,multimap<string,SubscriptionInfo>::iterator> subscriptionIterators;
  
  subscriptionIterators = subscriptions.equal_range(mimeType);
  
  for(it = subscriptionIterators.first; it != subscriptionIterators.second; it++) {
    if(originHandlerId == (*it).second.handlerId) {
      (*it).second.references--;
      if((*it).second.references == 0) {
        subscriptions.erase(it);
      }
      return true;
    }
  }
  
  //if we get here, there wasn't a subscription to unsubscribe from
  LOG_WARN("Tried to unsubscribe without an existing subscription");
  return false;
}

bool GatewayCore::pushCrossGateway(std::string uri, std::string mimeType, const std::string &data, std::string originUser, std::string originHandlerId) {
  return false;
}

