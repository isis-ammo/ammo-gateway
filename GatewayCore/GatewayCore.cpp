#include "GatewayCore.h"

#include "GatewayConfigurationManager.h"

#include "GatewayEventHandler.h"
#include "CrossGatewayEventHandler.h"
#include "CrossGatewayConnectionManager.h"


#include "log.h"

#include <iostream>

using namespace std;

GatewayCore* GatewayCore::sharedInstance = NULL;

GatewayCore::GatewayCore() : connectionManager(NULL), parentHandler(NULL), crossGatewayAcceptor(NULL) {
  
} 

GatewayCore* GatewayCore::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayCore();
  }
  
  return sharedInstance;
}

bool GatewayCore::registerDataInterest(std::string mime_type, MessageScope messageScope, GatewayEventHandler *handler) {
  LOG_INFO("Registering interest in " << mime_type << " by handler " << handler);
  LocalSubscriptionInfo subscriptionInfo;
  subscriptionInfo.handler = handler;
  subscriptionInfo.scope = messageScope;
  pushHandlers.insert(PushHandlerMap::value_type(mime_type, subscriptionInfo));
  
  if(messageScope == SCOPE_GLOBAL) {
    //now propogate the subscription to all the other gateway nodes
    for(map<string, CrossGatewayEventHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
      it->second->sendSubscribeMessage(mime_type);
    }
  }
  return true;
}

bool GatewayCore::unregisterDataInterest(std::string mime_type, MessageScope messageScope, GatewayEventHandler *handler) {
  LOG_INFO("Unregistering interest in " << mime_type << " by handler " << handler);
  PushHandlerMap::iterator it;
  pair<PushHandlerMap::iterator,PushHandlerMap::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mime_type);
  
  bool foundSubscription = false;
  MessageScope foundScope = SCOPE_ALL;
  
  for(it = handlerIterators.first; it != handlerIterators.second;) {
    //need to increment the iterator *before* we erase it, because erasing it
    //invalidates the iterator (it doesn't invalidate other iterators in the list,
    //though)
    PushHandlerMap::iterator eraseIter = it++;
    
    if(handler == (*eraseIter).second.handler && (eraseIter->second.scope == messageScope || messageScope == SCOPE_ALL)) {
      //LOG_TRACE("Removing an element");
      foundScope = eraseIter->second.scope;
      pushHandlers.erase(eraseIter);
      foundSubscription = true;
      break;
    }
  }
  
  if(foundSubscription == true && foundScope == SCOPE_GLOBAL) {
    //now propogate the unsubscription to all the other gateway nodes
    for(map<string, CrossGatewayEventHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
      it->second->sendUnsubscribeMessage(mime_type);
    }
  }
  return foundSubscription;
}

bool GatewayCore::registerPullInterest(std::string mime_type, MessageScope scope, GatewayEventHandler *handler) {
  LOG_INFO("Registering pull interest in " << mime_type << " by handler " << handler);
  LocalPullHandlerInfo pullHandler;
  pullHandler.handler = handler;
  pullHandler.scope = scope;
  pullHandlers.insert(PullHandlerMap::value_type(mime_type, pullHandler));
  
  if(scope == SCOPE_GLOBAL) {
    LOG_TRACE("Sending pull interest to connected gateways...");
    //now propogate the subscription to all the other gateway nodes
    for(map<string, CrossGatewayEventHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
      it->second->sendRegisterPullInterest(mime_type);
    }
  }
  return true;
}

bool GatewayCore::unregisterPullInterest(std::string mime_type, MessageScope scope, GatewayEventHandler *handler) {
  LOG_INFO("Unregistering pull interest in " << mime_type << " by handler " << handler);
  PullHandlerMap::iterator it;
  pair<PullHandlerMap::iterator, PullHandlerMap::iterator> handlerIterators;
  
  handlerIterators = pullHandlers.equal_range(mime_type);
  
  bool foundSubscription = false;
  MessageScope foundScope = SCOPE_ALL;
  
  for(it = handlerIterators.first; it != handlerIterators.second;) {
    PullHandlerMap::iterator eraseIter = it++;
    
    if(handler == (*eraseIter).second.handler) {
      pullHandlers.erase(eraseIter);
      foundSubscription = true;
      foundScope = SCOPE_ALL;
      break;
    }
  }
  
  if(foundSubscription == true && foundScope == SCOPE_GLOBAL) {
    //now propogate the unsubscription to all the other gateway nodes
    for(map<string, CrossGatewayEventHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
      it->second->sendRegisterPullInterest(mime_type);
    }
  }
  return foundSubscription;
}

bool GatewayCore::pushData(GatewayEventHandler *sender, std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, MessageScope messageScope, char priority) {
  LOG_DEBUG("  Pushing data with uri: " << uri);
  LOG_DEBUG("                    type: " << mimeType);
  LOG_DEBUG("                    scope: " << messageScope);
  set<GatewayEventHandler *>::iterator it;
  
  set<GatewayEventHandler *> handlers = getPushHandlersForType(mimeType);
  
  for(it = handlers.begin(); it != handlers.end(); ++it) {
    if((*it) != sender) { //don't send pushed data to plugin that originated it, if it's subscribed to the same topic
      (*it)->sendPushedData(uri, mimeType, encoding, data, originUser, messageScope, priority);
    }
  }
  
  if(messageScope == SCOPE_GLOBAL) {
    //now propagate the subscription to all the other gateway nodes
    {
      multimap<string, SubscriptionInfo>::iterator it;
      pair<multimap<string, SubscriptionInfo>::iterator,multimap<string,SubscriptionInfo>::iterator> subscriptionIterators;

      subscriptionIterators = subscriptions.equal_range(mimeType);

      for(it = subscriptionIterators.first; it != subscriptionIterators.second; it++) {
        LOG_TRACE("Sending cross-gateway data");
        crossGatewayHandlers[(*it).second.handlerId]->sendPushedData(uri, mimeType, encoding, data, originUser, priority);
      }
    }
  }
  return true;
}

bool GatewayCore::pullRequest(GatewayEventHandler *sender, std::string requestUid, std::string pluginId, std::string mimeType, 
                              std::string query, std::string projection, unsigned int maxResults, 
                              unsigned int startFromCount, bool liveQuery, MessageScope scope, char priority) {
  LOG_DEBUG("  Sending pull request with type: " << mimeType);
  LOG_DEBUG("                        pluginId: " << pluginId);
  LOG_DEBUG("                           query: " << query);
  PullHandlerMap::iterator it;
  pair<PullHandlerMap::iterator, PullHandlerMap::iterator> handlerIterators;
  
  handlerIterators = pullHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    if((*it).second.handler != sender) { //don't send pull request to originating plugin, if it handles the same type
      LOG_DEBUG("Sending request to " << (*it).second.handler);
      (*it).second.handler->sendPullRequest(requestUid, pluginId, mimeType, query, projection, maxResults, startFromCount, liveQuery, priority);
    }
  }
  
  //update plugin ID to the originating service handler that called this method
  plugins[pluginId] = sender;
  
  if(scope == SCOPE_GLOBAL) {
    CrossGatewayPullRequestHandlerMap::iterator it;
    pair<CrossGatewayPullRequestHandlerMap::iterator, CrossGatewayPullRequestHandlerMap::iterator> pullRequestIterators;
    
    pullRequestIterators = crossGatewayPullRequestHandlers.equal_range(mimeType);
    
    //send pull request to other gateways
    for(it = pullRequestIterators.first; it != pullRequestIterators.second; it++) {
      LOG_TRACE("Sending cross-gateway pull request");
      crossGatewayHandlers[(*it).second.handlerId]->sendPullRequest(requestUid, pluginId, mimeType, query, projection, maxResults, startFromCount, liveQuery, priority);
    }
  }
  return true;
}

bool GatewayCore::pullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::string encoding, const std::string& data, char priority) {
  LOG_DEBUG("  Sending pull response with type: " << mimeType);
  LOG_DEBUG("                        pluginId: " << pluginId);

  map<string, GatewayEventHandler *>::iterator it = plugins.find(pluginId);
  if ( it != plugins.end() ) {
    //check for something here?
    (*it).second->sendPullResponse(requestUid, pluginId, mimeType, uri, encoding, data, priority);
    return true;
  } else {
    PullRequestReturnIdMap::iterator it2 = cgPullRequestReturnIds.find(pluginId);
    if(it2 != cgPullRequestReturnIds.end()) {
      std::map<std::string, CrossGatewayEventHandler *>::iterator cgHandlerIt = crossGatewayHandlers.find(it2->second);
      if(cgHandlerIt != crossGatewayHandlers.end()) {
        (*cgHandlerIt).second->sendPullResponse(requestUid, pluginId, mimeType, uri, encoding, data, priority);
      }
      return true;
    }
  }
  return false;
}

bool GatewayCore::unregisterPullResponsePluginId(std::string pluginId, GatewayEventHandler *handler) {
  map<string, GatewayEventHandler *>::iterator it = plugins.find(pluginId);
  if ( it != plugins.end() ) {
    if(it->second == handler) {
      plugins.erase(it);
    }
  }
  return true;
}

void GatewayCore::initCrossGateway() {
  GatewayConfigurationManager *config = GatewayConfigurationManager::getInstance();
  
  LOG_INFO("Initializing cross-gateway connection...");
  LOG_DEBUG("Creating acceptor for incoming connections");
  //TODO: make interface and port number specifiable on the command line
  ACE_INET_Addr serverAddress(config->getCrossGatewayServerPort(), config->getCrossGatewayServerInterface().c_str());
  
  LOG_DEBUG("(CG) Listening on port " << config->getCrossGatewayServerPort() << " on interface " << config->getCrossGatewayServerInterface().c_str());
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  crossGatewayAcceptor = new ammo::gateway::internal::NetworkAcceptor<ammo::gateway::protocol::GatewayWrapper, CrossGatewayEventHandler, ammo::gateway::internal::SYNC_MULTITHREADED, 0x8badf00d>(config->getCrossGatewayServerInterface(), config->getCrossGatewayServerPort());
  
  //We connect to a parent gateway if the parent address isn't blank; if it is
  //blank, this gateway must be the root (of our tree)
  if(config->getCrossGatewayParentAddress() != "") {
    connectionManager = new CrossGatewayConnectionManager();
    LOG_DEBUG("Starting connection manager");
    connectionManager->activate();
  } else {
    LOG_INFO("Acting as cross-gateway root node.");
  }
}

void GatewayCore::setParentHandler(CrossGatewayEventHandler *handler) {
  this->parentHandler = handler;
}
  
bool GatewayCore::registerCrossGatewayConnection(std::string handlerId, CrossGatewayEventHandler *handler, std::vector<std::pair<std::string, std::string> > connectedPlugins) {
  LOG_DEBUG("Registering cross-gateway handler " << handlerId);
  crossGatewayHandlers[handlerId] = handler;
  //send existing subscriptions
  //local subscriptions (subscribe to everything that has global scope)
  for(PushHandlerMap::iterator it = pushHandlers.begin(); it != pushHandlers.end(); it++) {
    if(it->second.scope == SCOPE_GLOBAL) {
      handler->sendSubscribeMessage(it->first);
    }
  }
  
  //and cross-gateway subscriptions (we filter out subscriptions from our handler ID, just in case, but shouldn't
  //be a problem assuming the old gateway has disconnected first)
  //also need to subscribe the number of times specified by the reference count,
  //so the ref count on the other end will be correct (should add a shortcut
  //for this so we don't send as many messages)
  for(CrossGatewaySubscriptionMap::iterator it = subscriptions.begin(); it != subscriptions.end(); it++) {
    if(it->second.handlerId != handlerId) {
      for(unsigned int i = 0; i < it->second.references; i++) {
        handler->sendSubscribeMessage(it->first);
      }
    }
  }
  
  return true;
}

bool GatewayCore::unregisterCrossGatewayConnection(std::string handlerId) {
  LOG_DEBUG("Unregistering cross-gateway handler " << handlerId);
  CrossGatewayEventHandler *handler = crossGatewayHandlers[handlerId];
  crossGatewayHandlers.erase(handlerId);
  
  if(handler == parentHandler) {
    parentHandler = NULL;
    //we're responsible for reconnecting this connection
    connectionManager->activate();
  }
  
  return false;
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
  for(map<string, CrossGatewayEventHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
    if(it->first != originHandlerId) {
      it->second->sendSubscribeMessage(mimeType);
    }
  }
  
  return true;
}
bool GatewayCore::unsubscribeCrossGateway(std::string mimeType, std::string originHandlerId) {
  LOG_DEBUG("Handler " << originHandlerId << " unsubscribing from type " << mimeType);
  //propogate the unsubscribe to all the other gateway nodes, except the one it came from
  for(map<string, CrossGatewayEventHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
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

bool GatewayCore::registerPullInterestCrossGateway(std::string mimeType, std::string originHandlerId) {
  LOG_DEBUG("Got register pull interest for type " << mimeType << " for handler " << originHandlerId);
  //see if there's already a subscription to this type for this handler
  bool foundSubscription = false;
  CrossGatewayPullRequestHandlerMap::iterator it;
  pair<CrossGatewayPullRequestHandlerMap::iterator, CrossGatewayPullRequestHandlerMap::iterator> pullRequestHandlerIterators;
  
  pullRequestHandlerIterators = crossGatewayPullRequestHandlers.equal_range(mimeType);
  
  for(it = pullRequestHandlerIterators.first; it != pullRequestHandlerIterators.second; it++) {
    if(originHandlerId == (*it).second.handlerId) {
      (*it).second.references++;
      foundSubscription = true;
    }
  }
  
  if(!foundSubscription) {
    //if we get here, we don't already have an entry for this handler in the table
    PullRequestHandlerInfo newHandler;
    newHandler.handlerId = originHandlerId;
    newHandler.references = 1;
    
    crossGatewayPullRequestHandlers.insert(CrossGatewayPullRequestHandlerMap::value_type(mimeType, newHandler));
  }
  
  //now propogate the subscription to all the other gateway nodes, except the one it came from
  for(map<string, CrossGatewayEventHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
    if(it->first != originHandlerId) {
      it->second->sendRegisterPullInterest(mimeType);
    }
  }
  
  return true;
}

bool GatewayCore::unregisterPullInterestCrossGateway(std::string mimeType, std::string originHandlerId) {
  LOG_DEBUG("Handler " << originHandlerId << " unregistering from pulls of type " << mimeType);
  //propogate the unsubscribe to all the other gateway nodes, except the one it came from
  for(map<string, CrossGatewayEventHandler *>::iterator it = crossGatewayHandlers.begin(); it != crossGatewayHandlers.end(); it++) {
    if(it->first != originHandlerId) {
      it->second->sendUnregisterPullInterest(mimeType);
    }
  }
  
  //look for an existing subscription to this type
  CrossGatewayPullRequestHandlerMap::iterator it;
  pair<CrossGatewayPullRequestHandlerMap::iterator,CrossGatewayPullRequestHandlerMap::iterator> pullHandlerIterators;
  
  pullHandlerIterators = crossGatewayPullRequestHandlers.equal_range(mimeType);
  
  for(it = pullHandlerIterators.first; it != pullHandlerIterators.second; it++) {
    if(originHandlerId == (*it).second.handlerId) {
      (*it).second.references--;
      if((*it).second.references == 0) {
        crossGatewayPullRequestHandlers.erase(it);
      }
      return true;
    }
  }
  
  //if we get here, there wasn't a subscription to unsubscribe from
  LOG_WARN("Tried to unregister without an existing pull registration");
  return false;
}

bool GatewayCore::pushCrossGateway(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, std::string originHandlerId, char priority) {
  LOG_DEBUG("  Received cross-gateway push data with uri: " << uri);
  LOG_DEBUG("                                       type: " << mimeType);
  LOG_DEBUG("                                       from: " << originHandlerId);
  
  //do a local push of this data
  {
    PushHandlerMap::iterator it;
    pair<PushHandlerMap::iterator,PushHandlerMap::iterator> handlerIterators;
    
    handlerIterators = pushHandlers.equal_range(mimeType);
    
    for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
      if((*it).second.scope == SCOPE_GLOBAL) {
        LOG_TRACE("Sending push data");
        (*it).second.handler->sendPushedData(uri, mimeType, encoding, data, originUser, SCOPE_GLOBAL, priority);
      }
    }
  }
  
  //push to all subscribed cross-gateway nodes, except the origin
  {
    multimap<string, SubscriptionInfo>::iterator it;
    pair<multimap<string, SubscriptionInfo>::iterator,multimap<string,SubscriptionInfo>::iterator> subscriptionIterators;
    
    subscriptionIterators = subscriptions.equal_range(mimeType);
    
    for(it = subscriptionIterators.first; it != subscriptionIterators.second; it++) {
      if(originHandlerId != (*it).second.handlerId) {
        LOG_TRACE("Sending cross-gateway data");
        crossGatewayHandlers[(*it).second.handlerId]->sendPushedData(uri, mimeType, encoding, data, originUser, priority);
      }
    }
  }
  
  return true;
}

bool GatewayCore::pullRequestCrossGateway(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection, unsigned int maxResults, unsigned int startFromCount, bool liveQuery, std::string originHandlerId, char priority) {
  LOG_DEBUG("  Received cross-gateway pull request with requestUid: " << requestUid);
  LOG_DEBUG("                                             pluginId: " << pluginId);
  LOG_DEBUG("                                                 from: " << originHandlerId);
  
  //do a local pull request
  PullHandlerMap::iterator it;
  pair<PullHandlerMap::iterator,PullHandlerMap::iterator> handlerIterators;
  
  handlerIterators = pullHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    //check for something here?
    LOG_DEBUG("Sending request to " << (*it).second.handler);
    (*it).second.handler->sendPullRequest(requestUid, pluginId, mimeType, query, projection, maxResults, startFromCount, liveQuery, priority);
  }
  
  //update handler return ID
  cgPullRequestReturnIds[pluginId] = originHandlerId;
  
  //push to all subscribed cross-gateway nodes, except the origin
  {
    CrossGatewayPullRequestHandlerMap::iterator it;
    pair<CrossGatewayPullRequestHandlerMap::iterator, CrossGatewayPullRequestHandlerMap::iterator> pullRequestIterators;
    
    pullRequestIterators = crossGatewayPullRequestHandlers.equal_range(mimeType);
    
    for(it = pullRequestIterators.first; it != pullRequestIterators.second; it++) {
      if(originHandlerId != (*it).second.handlerId) {
        LOG_TRACE("Sending cross-gateway data");
        crossGatewayHandlers[(*it).second.handlerId]->sendPullRequest(requestUid, pluginId, mimeType, query, projection, maxResults, startFromCount, liveQuery, priority);
      }
    }
  }
  
  return true;
}

bool GatewayCore::pullResponseCrossGateway(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::string encoding, const std::string &data, std::string originHandlerId, char priority) {
  //check for a local plugin with this ID
  map<string, GatewayEventHandler *>::iterator it = plugins.find(pluginId);
  if ( it != plugins.end() ) {
    //check for something here?
    (*it).second->sendPullResponse(requestUid, pluginId, mimeType, uri, encoding, data, priority);
    return true;
  } else {
    PullRequestReturnIdMap::iterator it2 = cgPullRequestReturnIds.find(pluginId);
    if(it2 != cgPullRequestReturnIds.end()) {
      if((it2->second) == originHandlerId) {
        LOG_WARN("Attempted to send message back to origin handler...  message won't be sent.");
      } else {
        std::map<std::string, CrossGatewayEventHandler *>::iterator cgHandlerIt = crossGatewayHandlers.find(it2->second);
        if(cgHandlerIt != crossGatewayHandlers.end()) {
          (*cgHandlerIt).second->sendPullResponse(requestUid, pluginId, mimeType, uri, encoding, data, priority);
        }
        return true;
      }
    }
  }
  
  return false;
}

bool GatewayCore::unregisterPullResponsePluginIdCrossGateway(std::string pluginId, std::string handler) {
  PullRequestReturnIdMap::iterator it = cgPullRequestReturnIds.find(pluginId);
  if ( it != cgPullRequestReturnIds.end() ) {
    if(it->second == handler) {
      cgPullRequestReturnIds.erase(it);
    }
  }
  return true;
}

std::set<GatewayEventHandler *> GatewayCore::getPushHandlersForType(std::string mimeType) {
  set<GatewayEventHandler *> matchingHandlers;

  for(PushHandlerMap::iterator it = pushHandlers.begin(); it!= pushHandlers.end(); it++) {
    if(mimeType.find(it->first) == 0) { //looking for subscribers which are a prefix of mimeType
      matchingHandlers.insert(it->second.handler);
    }
  }
  return matchingHandlers;
}

void GatewayCore::terminate() {
  if(connectionManager) {
    connectionManager->cancel();
    connectionManager->wait();
  }
}

GatewayCore::~GatewayCore() {
  LOG_DEBUG("Destroying GatewayCore.");
  if(connectionManager) {
    connectionManager->cancel();
    connectionManager->wait();
  }
}
