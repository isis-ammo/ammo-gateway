#include "SubscriptionDataStore.h"

using namespace std;

SubscriptionDataStore::SubscriptionDataStore() : subscriptions() {
  
}

SubscriptionDataStore::SubscriptionMap SubscriptionDataStore::getSubscriptionMap() {
  return subscriptions;
}

SubscriptionDataStore::SubscriptionHandlerSet SubscriptionDataStore::getHandlersForType(const std::string &typeName) {
  SubscriptionHandlerSet matchingHandlers;

  for(SubscriptionMap::iterator it = subscriptions.begin(); it!= subscriptions.end(); it++) {
    if(typeName.find(it->first) == 0) { //looking for subscribers which are a prefix of mimeType
      matchingHandlers.insert(it->second);
    }
  }
  
  return matchingHandlers;
}

void SubscriptionDataStore::subscribe(const std::string &typeName, const std::string &handlerName) {
  subscriptions.insert(SubscriptionMap::value_type(typeName, handlerName));
}

void SubscriptionDataStore::unsubscribe(const std::string &typeName, const std::string &handlerName) {
  SubscriptionMap::iterator it;
  pair<SubscriptionMap::iterator,SubscriptionMap::iterator> handlerIterators;
  
  handlerIterators = subscriptions.equal_range(typeName);
  
  bool foundSubscription = false;
  
  for(it = handlerIterators.first; it != handlerIterators.second;) {
    //need to increment the iterator *before* we erase it, because erasing it
    //invalidates the iterator (it doesn't invalidate other iterators in the list,
    //though)
    SubscriptionMap::iterator eraseIter = it++;
    
    if(handlerName == (*eraseIter).second) {
      //LOG_TRACE("Removing an element");
      subscriptions.erase(eraseIter);
      foundSubscription = true;
      break;
    }
  }
}
