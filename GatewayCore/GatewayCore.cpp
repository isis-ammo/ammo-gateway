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

bool GatewayCore::registerDataInterest(std::string uri, GatewayServiceHandler *handler) {
  cout << "Registering interest in " << uri << " by handler " << handler << endl << flush;
  pushHandlers.insert(pair<string, GatewayServiceHandler *>(uri, handler));
  return true;
}

bool GatewayCore::unregisterDataInterest(std::string uri, GatewayServiceHandler *handler) {
  cout << "Unregistering interest in " << uri << " by handler " << handler << endl << flush;
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(uri);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    if(handler == (*it).second) {
      pushHandlers.erase(it);
    }
  }
  
  return true;
}

bool GatewayCore::pushData(std::string uri, std::string mimeType, const std::string &data) {
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(uri);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    (*it).second->sendPushedData(uri, mimeType, data);
  }
  
  return true;
}

