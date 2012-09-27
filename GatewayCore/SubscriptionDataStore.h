#ifndef SUBSCRIPTION_DATA_STORE_H
#define SUBSCRIPTION_DATA_STORE_H

#include <set>
#include <map>
#include <string>

class SubscriptionDataStore {
public:
  SubscriptionDataStore();
  
  std::set<std::string> getHandlersForType(const std::string &typeName);
  void subscribe(const std::string &typeName, const std::string &handlerName);
  void unsubscribe(const std::string &typeName, const std::string &handlerName);
  
private:
  typedef std::multimap<std::string, std::string> SubscriptionMap;
  SubscriptionMap subscriptions;
};

#endif
