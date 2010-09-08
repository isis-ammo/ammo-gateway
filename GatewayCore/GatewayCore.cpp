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

bool GatewayCore::pushData(std::string uri, std::string mimeType, const std::string &data) {
  multimap<string,GatewayServiceHandler *>::iterator it;
  pair<multimap<string,GatewayServiceHandler *>::iterator,multimap<string,GatewayServiceHandler *>::iterator> handlerIterators;
  
  handlerIterators = pushHandlers.equal_range(mimeType);
  
  for(it = handlerIterators.first; it != handlerIterators.second; ++it) {
    (*it).second->sendPushedData(uri, mimeType, data);
  }
  
  return true;
}

