#ifndef SUBSCRIPTION_DATA_STORE_H
#define SUBSCRIPTION_DATA_STORE_H

#include <list>
#include <string>

class SubscriptionDataStore {
public:
  SubscriptionDataStore();
  
  std::list<std::string> getHandlersForType(const std::string &typeName);
  void subscribe(const std::string &typeName, const std::string &handlerName);
  void unsubscribe(const std::string &typeName, const std::string &handlerName);
  
private:
  struct HandlerInfo {
    std::string handlerName;
    unsigned int referenceCount;
  };
  
  struct SubscriptionItem {
    std::string typeName;
    std::vector<HandlerInfo> handlers;
    std::map<std::string typeName, SubscriptionItem *> subtypes;
    SubscriptionItem *parent;
  };
};

#endif
